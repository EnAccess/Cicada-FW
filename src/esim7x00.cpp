/*
 * E-Lib
 * Copyright (C) 2019 EnAccess
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "esim7x00.h"

#define LINE_MAX_LENGTH     60
#define MIN_SPACE_AVAILABLE 22

#define OK_STR_LENGTH        2
#define LINE_END_STR_LENGTH  2
#define QUOTE_END_STR_LENGTH 3

#define CONNECT_PENDING    (1 << 0)
#define RESET_PENDING      (1 << 1)
#define DATA_PENDING       (1 << 2)
#define DISCONNECT_PENDING (1 << 3)
#define IP_CONNECTED       (1 << 4)
#define LINE_READ          (1 << 5)

const char* ESim7x00CommDevice::_okStr = "OK";
const char* ESim7x00CommDevice::_lineEndStr = "\r\n";
const char* ESim7x00CommDevice::_quoteEndStr = "\"\r\n";

ESim7x00CommDevice::ESim7x00CommDevice(EIBufferedSerial& serial) :
    _serial(serial),
    _sendState(notConnected),
    _replyState(normalReply),
    _apn(NULL),
    _host(NULL),
    _port(0),
    _waitForReply(NULL),
    _stateBooleans(0),
    _bytesToWrite(0),
    _bytesToReceive(0),
    _bytesToRead(0)
{ }

void ESim7x00CommDevice::setHostPort(const char* host, uint16_t port)
{
    _host = host;
    _port = port;
}

void ESim7x00CommDevice::setApn(const char* apn)
{
    _apn = apn;
}

bool ESim7x00CommDevice::connect()
{
    if (_apn == NULL || _host == NULL || _port == 0)
        return false;

    _stateBooleans |= CONNECT_PENDING;

    return true;
}

void ESim7x00CommDevice::disconnect()
{
    _stateBooleans |= DISCONNECT_PENDING;
}

bool ESim7x00CommDevice::isConnected()
{
    return _sendState >= connected &&
        _sendState < ipUnconnected && (_stateBooleans & IP_CONNECTED);
}

bool ESim7x00CommDevice::isIdle()
{
    return _sendState == notConnected;
}

uint16_t ESim7x00CommDevice::bytesAvailable() const
{
    return _readBuffer.availableData();
}

uint16_t ESim7x00CommDevice::spaceAvailable() const
{
    if (_sendState < connected || _sendState > receiving)
        return 0;

    return _writeBuffer.availableSpace();
}

uint16_t ESim7x00CommDevice::read(uint8_t* data, uint16_t maxSize)
{
    return _readBuffer.pull(data, maxSize);
}

uint16_t ESim7x00CommDevice::write(const uint8_t* data, uint16_t size)
{
    if (_sendState < connected || _sendState > receiving)
        return 0;

    return _writeBuffer.push(data, size);
}

bool ESim7x00CommDevice::handleDisconnect(SendState nextState)
{
    if (_stateBooleans & DISCONNECT_PENDING)
    {
        _stateBooleans &= ~DISCONNECT_PENDING;
        _replyState = noReply;
        _sendState = nextState;

        return true;
    }

    return false;
}

bool ESim7x00CommDevice::handleConnect(SendState nextState)
{
    if (_stateBooleans & CONNECT_PENDING)
    {
        _stateBooleans &= ~CONNECT_PENDING;
        _replyState = noReply;
        _sendState = nextState;

        return true;
    }

    return false;
}

#define SEND_COMMAND(cmd, expectedReply, nextState)             \
    {                                                           \
        const char sendStr[] = cmd;                             \
        _serial.write(sendStr, sizeof(sendStr) - 1);            \
        _serial.write(_lineEndStr, LINE_END_STR_LENGTH);        \
        _waitForReply = expectedReply;                          \
        _sendState = nextState;                                 \
        break;                                                  \
    }

void ESim7x00CommDevice::run()
{
    // If the serial device is net yet open, try to open it
    if (!_serial.isOpen())
    {
        if (!_serial.open())
        {
            _sendState = serialError;
        }
        return;
    }

    // If a modem reset is pending, handle it
    if (_stateBooleans & RESET_PENDING)
    {
        _serial.flushReceiveBuffers();
        _stateBooleans = LINE_READ;
        _bytesToRead = 0;
        _bytesToReceive = 0;
        _bytesToWrite = 0;
        if (_sendState >= connecting && _sendState <= receiving)
            _sendState = connecting;
        else
            _sendState = notConnected;
        const char str[] = "AT+CRESET";
        _serial.write(str, sizeof(str) - 1);
        _serial.write(_lineEndStr, LINE_END_STR_LENGTH);
        _replyState = normalReply;
        _waitForReply = "RDY";

        setDelay(4000);

        return;
    }

    // Check if there is a reply from the modem
    if ((_stateBooleans & LINE_READ) && _serial.canReadLine())
    {
        char data[LINE_MAX_LENGTH + 1];
        uint8_t size = _serial.readLine(data, LINE_MAX_LENGTH);
        data[size] = '\0';

        /*
        if (_sendState < connected || _sendState > receiving)
        {
            if (_waitForReply)
                printf("_sendState=%d, _replyState=%d, "
                       "_waitForReply=\"%s\", data: %s",
                       _sendState, _replyState, _waitForReply, data);
            else
                printf("_sendState=%d, _replyState=%d, "
                       "_waitForReply=NULL, data: %s",
                       _sendState, _replyState, data);
        }
        */

        // If we sent a command, we process the reply here
        if (_waitForReply)
        {
            if (strncmp(data, _waitForReply, strlen(_waitForReply)) == 0)
            {
                _replyState = normalReply;
                _waitForReply = NULL;
            }
            else if (strncmp(data, "ERROR", 5) == 0)
            {
                _stateBooleans |= RESET_PENDING;

                _replyState = noReply;
                _waitForReply = NULL;
                return;
            }
        }

        // Depending on the reply to expect, handle different cases
        switch(_replyState)
        {
        case netopen:
            if (strncmp(data, "+NETOPEN: 1", 11) == 0)
            {
                setDelay(2000);
                _sendState = sendNetopen;
                _waitForReply = NULL;
                _replyState = noReply;
                return;
            }
            break;

        case cdnsgip:
            if (strncmp(data, "+CDNSGIP: 1", 11) == 0)
            {
                _replyState = normalReply;

                char* tmpStr;
                uint8_t i = 0, q = 0;

                // Validate DNS reply string
                while (data[i])
                {
                    if (data[i++] == '\"')
                        q++;
                }
                if (q != 4)
                {
                    // Error in input string
                    _sendState = dnsError;
                    return;
                }
                i = 0, q = 0;

                // Parse IP address
                while(q < 3)
                    if (data[i++] == '\"')
                        q++;
                tmpStr = data + i;
                while(data[i])
                {
                    if (data[i] == '\"')
                        data[i] = '\0';
                    i++;
                }
                strcpy(_ip, tmpStr);
            }
            break;

        case ciprxget4:
            if (strncmp(data, "+CIPRXGET: 4,0,", 15) == 0)
            {
                int bytesToReceive;
                sscanf(data + 15, "%d", &bytesToReceive);
                _bytesToReceive += bytesToReceive;
            }
            break;

        case ciprxget2:
            if (strncmp(data, "+CIPRXGET: 2,0,", 15) == 0)
            {
                int bytesToReceive;
                sscanf(data + 15, "%d", &bytesToReceive);
                _bytesToReceive -= bytesToReceive;
                _bytesToRead += bytesToReceive;
                _stateBooleans &= ~LINE_READ;
                _replyState = noReply;
                _sendState = receiving;
            }
            break;

        default:
            break;
        }

        // In connected state, check for new data or IP connection close
        if (_sendState >= connected)
        {
            if (strncmp(data, "+CIPRXGET: 1,0", 14) == 0)
            {
                _stateBooleans |= DATA_PENDING;
            }
            else if (strncmp(data, "+IPCLOSE: 0,", 12) == 0)
            {
                _stateBooleans &= ~IP_CONNECTED;
            }
        }
    }

    // When disconnecting was requested, flush read buffer first
    else if ((_stateBooleans & DISCONNECT_PENDING) && _sendState == receiving)
    {
        while (_bytesToRead && _serial.bytesAvailable())
        {
            _serial.read();
            _bytesToRead--;
        }
        _bytesToReceive = 0;

        if (_bytesToRead == 0)
        {
            _stateBooleans |= LINE_READ;
        }
    }

    // Don't go on when waiting for a reply
    if (_waitForReply)
        return;

    // Don't go on if space in write buffer is low
    if (_serial.spaceAvailable() < 20)
        return;

    // Connection state machine
    switch(_sendState)
    {
    case notConnected:
        setDelay(10);
        handleConnect(connecting);
        break;

    case connecting:
        setDelay(10);
        _stateBooleans |= LINE_READ;
        SEND_COMMAND("ATE1", _okStr, sendCgdcont);

    case sendCgdcont:
    {
        const char str[] = "AT+CGDCONT=1,\"IP\",\"";
        _serial.write(str, sizeof(str) - 1);
        _serial.write(_apn, strlen(_apn));
        _serial.write(_quoteEndStr, QUOTE_END_STR_LENGTH);

        _waitForReply = _okStr;
        _sendState = sendAtd;
        break;
    }

    case sendAtd:
        setDelay(500);
        SEND_COMMAND("ATD*99#", "CONNECT", sendPpp);

    case sendPpp:
        setDelay(1000);
    {
        const char str[] = "+++";
        _serial.write(str, sizeof(str) - 1);

        _waitForReply = _okStr;
        _sendState = sendCsocksetpn;
        break;
    }

    case sendCsocksetpn:
        setDelay(10);
        SEND_COMMAND("AT+CSOCKSETPN=1", _okStr, sendCipmode);

    case sendCipmode:
        SEND_COMMAND("AT+CIPMODE=0", _okStr, sendNetopen);

    case sendNetopen:
        setDelay(10);
        _replyState = netopen;
        SEND_COMMAND("AT+NETOPEN", "+NETOPEN: 0", sendCiprxget);

    case sendCiprxget:
        SEND_COMMAND("AT+CIPRXGET=1", _okStr, sendDnsQuery);

    case sendDnsQuery:
    {
        if (_serial.spaceAvailable() < strlen(_host) + 20)
            break;
        _replyState = cdnsgip;
        const char str[] = "AT+CDNSGIP=\"";
        _serial.write(str, sizeof(str) - 1);
        _serial.write(_host, strlen(_host));
        _serial.write(_quoteEndStr, QUOTE_END_STR_LENGTH);

        _waitForReply = _okStr;
        _sendState = sendCipopen;
        break;
    }

    case sendCipopen:
    {
        const char str[] = "AT+CIPOPEN=0,\"TCP\",\"";
        const char midStr[] = "\",";
        char portStr[6];

        sprintf(portStr, "%d", _port);

        _serial.write(str, sizeof(str) - 1);
        _serial.write(_ip, strlen(_ip));
        _serial.write(midStr, sizeof(midStr) - 1);
        _serial.write(portStr, strlen(portStr));
        _serial.write(_lineEndStr, LINE_END_STR_LENGTH);

        _waitForReply = "+CIPOPEN: 0,0";
        _sendState = finalizeConnect;
        break;
    }

    case finalizeConnect:
        setDelay(0);
        _replyState = normalReply;
        _sendState = connected;
        _stateBooleans |= IP_CONNECTED;
        break;

    case connected:
        if (_writeBuffer.availableData())
        {
            if (_serial.spaceAvailable() >= MIN_SPACE_AVAILABLE)
            {
                _bytesToWrite = _writeBuffer.availableData();
                if (_bytesToWrite >
                    _serial.spaceAvailable() - MIN_SPACE_AVAILABLE)
                {
                    _bytesToWrite =
                        _serial.spaceAvailable() - MIN_SPACE_AVAILABLE;
                }

                _stateBooleans &= ~LINE_READ;

                const char str[] = "AT+CIPSEND=0,";
                char sizeStr[6];

                sprintf(sizeStr, "%d", _bytesToWrite);

                _serial.write(str, sizeof(str) - 1);
                _serial.write(sizeStr, strlen(sizeStr));
                _serial.write(_lineEndStr, LINE_END_STR_LENGTH);

                _sendState = sendCipsend;
            }
        }
        else if (_stateBooleans & DATA_PENDING)
        {
            _stateBooleans &= ~DATA_PENDING;
            _sendState = sendCiprxget4;
        }
        else
        {
            handleDisconnect(sendNetclose);
        }
        break;

    // States after connecting

    case sendCipsend:
    {
        char data;
        if (_serial.read(&data, 1) == 1)
        {
            if (data == '>')
            {
                while (_bytesToWrite--)
                {
                    _serial.write(_writeBuffer.pull());
                }
                _stateBooleans |= LINE_READ;
                _waitForReply = _okStr;
                _sendState = connected;
            }
        }
        break;
    }

    case sendCiprxget4:
        _replyState = ciprxget4;
        SEND_COMMAND("AT+CIPRXGET=4,0", _okStr, sendCiprxget2);

    case sendCiprxget2:
        if (handleDisconnect(sendNetclose))
            break;

        if (_bytesToReceive > 0)
        {
            if (_serial.spaceAvailable() > 8 &&
                _readBuffer.availableSpace() > 0)
            {
                int bytesToReceive = _serial.spaceAvailable() - 8;
                if (bytesToReceive > _bytesToReceive)
                    bytesToReceive = _bytesToReceive;
                if (bytesToReceive > _readBuffer.availableSpace())
                    bytesToReceive = _readBuffer.availableSpace();

                const char str[] = "AT+CIPRXGET=2,0,";
                char sizeStr[6];
                sprintf(sizeStr, "%d", bytesToReceive);
                _serial.write(str, sizeof(str) - 1);
                _serial.write(sizeStr, strlen(sizeStr));
                _serial.write(_lineEndStr, LINE_END_STR_LENGTH);
                _replyState = ciprxget2;
                _sendState = waitReceive;
            }
        }
        else if (_stateBooleans & IP_CONNECTED)
        {
            _replyState = normalReply;
            _sendState = connected;
        }
        else
        {
            _replyState = noReply;
            _sendState = ipUnconnected;
        }

        break;

    case waitReceive:
        break;

    case receiving:
        if (_bytesToRead > 0)
        {
            if (_serial.bytesAvailable() >= _bytesToRead)
            {
                while(_bytesToRead)
                {
                    _readBuffer.push(_serial.read());
                    _bytesToRead--;
                }
                _stateBooleans |= LINE_READ;
                _replyState = normalReply;
                _waitForReply = _okStr;
            }
        }
        else if (_bytesToReceive > 0)
        {
            _sendState = sendCiprxget2;
        }
        else
        {
            _sendState = sendCiprxget4;
        }
        break;

    case ipUnconnected:
        if (handleDisconnect(sendNetclose))
            break;

        handleConnect(sendCipopen);
        break;

    case sendNetclose:
        SEND_COMMAND("AT+NETCLOSE", "+NETCLOSE: 0", sendAth);

    case sendAth:
        SEND_COMMAND("ATH", _okStr, finalizeDisconnect);

    case finalizeDisconnect:
        _sendState = notConnected;
        break;

    default:
        break;
    }
}
