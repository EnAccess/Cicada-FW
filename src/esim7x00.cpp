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

#define LINE_MAX_LENGTH 60

#define OK_STR_LENGTH        2
#define LINE_END_STR_LENGTH  2
#define QUOTE_END_STR_LENGTH 3
const char* ESim7x00CommDevice::_okStr = "OK";
const char* ESim7x00CommDevice::_lineEndStr = "\r\n";
const char* ESim7x00CommDevice::_quoteEndStr = "\"\r\n";

ESim7x00CommDevice::ESim7x00CommDevice(EDefaultBufferedSerial& serial) :
    _serial(serial),
    _connectState(notConnected),
    _replyState(normalReply),
    _apn(NULL),
    _host(NULL),
    _port(0),
    _waitForReply(NULL),
    _lineRead(true),
    _ipConnected(false),
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
    if (_connectState != notConnected || _apn == NULL ||
        _host == NULL || _port == 0)
    {
        return false;
    }

    _connectState = connecting;

    return true;
}

bool ESim7x00CommDevice::disconnect()
{
    if (_connectState <= sendPpp || _connectState > receiving)
        return false;

    if (_connectState == finalizeConnect)
        _waitForReply = NULL;

    if (_connectState > sendNetopen)
        _connectState = sendNetclose;
    else if (_connectState > sendPpp)
        _connectState = sendAth;
    else
        _connectState = notConnected;

    return true;
}

bool ESim7x00CommDevice::isConnected()
{
    return _connectState >= connected &&
        _connectState < sendNetclose && _ipConnected;
}

bool ESim7x00CommDevice::isIdle()
{
    return _connectState == notConnected;
}

uint16_t ESim7x00CommDevice::bytesAvailable() const
{
    if (_connectState != receiving)
        return 0;

    if (_bytesToRead > _serial.bytesAvailable())
        return _serial.bytesAvailable();
    else
        return _bytesToRead;
}

uint16_t ESim7x00CommDevice::spaceAvailable() const
{
    if (_connectState < connected || _connectState > receiving)
        return 0;

    return _serial.bytesAvailable() - 4;
}

uint16_t ESim7x00CommDevice::read(uint8_t* data, uint16_t maxSize)
{
    if (_connectState != receiving)
        return 0;

    if (maxSize > _bytesToRead)
        maxSize = _bytesToRead;

    uint16_t bytesRead = _serial.read((char*)data, maxSize);
    _bytesToRead -= bytesRead;

    if (_bytesToRead == 0)
    {
        _lineRead = true;

        if (_bytesToReceive == 0)
        {
            _connectState = sendCiprxget4;
        }
        else
        {
            _connectState = sendCiprxget2;
        }
    }

    return bytesRead;
}

uint16_t ESim7x00CommDevice::write(const uint8_t* data, uint16_t size)
{
    const char minSpace = 22;

    if (_connectState != connected)
        return 0;

    if (_serial.spaceAvailable() < minSpace)
        return 0;

    if (size > _serial.spaceAvailable() - minSpace + 1)
        size = _serial.spaceAvailable() - minSpace + 1;

    _connectState = sendCipsend;
    _lineRead = false;

    const char str[] = "AT+CIPSEND=0,";
    char sizeStr[6];

    sprintf(sizeStr, "%d", size);

    _serial.write(str, sizeof(str) - 1);
    _serial.write(sizeStr, strlen(sizeStr));
    _serial.write(_lineEndStr, LINE_END_STR_LENGTH);
    _serial.setWriteBarrier();

    return _serial.write((char*)data, size);
}

#define SEND_COMMAND(cmd, expectedReply, nextState)             \
    {                                                           \
        const char sendStr[] = cmd;                             \
        _serial.write(sendStr, sizeof(sendStr) - 1);           \
        _serial.write(_lineEndStr, LINE_END_STR_LENGTH);      \
        _waitForReply = expectedReply;                         \
        _connectState = nextState;                             \
        break;                                                  \
    }

void ESim7x00CommDevice::run()
{
    // If the serial device is net yet open, try to open it
    if (!_serial.isOpen())
    {
        if (!_serial.open())
        {
            _connectState = serialError;
        }
        return;
    }

    // Check if there is a reply from the modem
    if (_lineRead && _serial.canReadLine())
    {
        char data[LINE_MAX_LENGTH + 1];
        uint8_t size = _serial.readLine(data, LINE_MAX_LENGTH);
        data[size] = '\0';

        if (_connectState < connected || _connectState > receiving)
        {
            if (_waitForReply)
                printf("_connectState=%d, _replyState=%d, "
                       "_waitForReply=\"%s\", data: %s",
                       _connectState, _replyState, _waitForReply, data);
            else
                printf("_connectState=%d, _replyState=%d, "
                       "_waitForReply=NULL, data: %s",
                       _connectState, _replyState, data);
        }

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
                disconnect();

                _replyState = noReply;
                _waitForReply = NULL;
                return;
            }
        }

        switch(_replyState)
        {
        case netopen:
            if (strncmp(data, "+NETOPEN: 1", 11) == 0)
            {
                // Send netopen again in case it didn't work
                setDelay(2000);
                _connectState = sendNetopen;
                _waitForReply = NULL;
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
                    _connectState = dnsError;
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

        case waitForData:
            if (strncmp(data, "+CIPRXGET: 1,0", 14) == 0)
            {
                _connectState = sendCiprxget4;
            }
            break;

        case ciprxget4:
            if (strncmp(data, "+CIPRXGET: 4,0,", 15) == 0)
            {
                int bytesToReceive;
                sscanf(data + 15, "%d", &bytesToReceive);
                _bytesToReceive += bytesToReceive;
                if (_bytesToReceive > 0)
                {
                    _replyState = normalReply;
                    _connectState = sendCiprxget2;
                }
                else if (_ipConnected)
                {
                    _replyState = waitForData;
                    _connectState = connected;
                }
                else
                {
                    _replyState = noReply;
                    _connectState = sendCipopen;
                }
            }
            break;

        case ciprxget2:
            if (strncmp(data, "+CIPRXGET: 2,0,", 15) == 0)
            {
                int bytesToReceive;
                sscanf(data + 15, "%d", &bytesToReceive);
                _bytesToReceive -= bytesToReceive;
                _bytesToRead += bytesToReceive;
                _lineRead = false;
                _replyState = normalReply;
                _connectState = receiving;
            }
            break;

        default:
            break;
        }

        // In connected state, check for connection close
        if (_connectState >= connected)
        {
            if (strncmp(data, "+IPCLOSE: 0,", 12) == 0)
            {
                _ipConnected = false;
            }
        }
    }

    // When disconnecting was requested, flush read buffer first
    else if (_connectState == sendNetclose)
    {
        while (_bytesToRead && _serial.bytesAvailable())
        {
            _serial.read();
            _bytesToRead--;
        }
        _bytesToReceive = 0;

        if (_bytesToRead == 0)
            _lineRead = true;
    }

    // Don't go on if we are in wait state
    if (_waitForReply)
        return;

    // Don't go on if space in write buffer is low
    if (_serial.spaceAvailable() < 20)
        return;

    // Connection state machine
    switch(_connectState)
    {

    case connecting:
        setDelay(10);
        SEND_COMMAND("ATE1", _okStr, sendCgdcont);

    case sendCgdcont:
    {
        const char str[] = "AT+CGDCONT=1,\"IP\",\"";
        _serial.write(str, sizeof(str) - 1);
        _serial.write(_apn, strlen(_apn));
        _serial.write(_quoteEndStr, QUOTE_END_STR_LENGTH);

        _waitForReply = _okStr;
        _connectState = sendAtd;
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
        _connectState = sendCsocksetpn;
        break;
    }

    case sendCsocksetpn:
        setDelay(10);
        SEND_COMMAND("AT+CSOCKSETPN=1", _okStr, sendCipmode);

    case sendCipmode:
        SEND_COMMAND("AT+CIPMODE=0", _okStr, sendNetopen);

    case sendNetopen:
        _replyState = netopen;
        SEND_COMMAND("AT+NETOPEN", "+NETOPEN: 0", sendCiprxget);

    case sendCiprxget:
        setDelay(10);
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
        _connectState = sendCipopen;
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
        _connectState = finalizeConnect;
        break;
    }

    case finalizeConnect:
        setDelay(0);
        _replyState = waitForData;
        _connectState = connected;
        _ipConnected = true;
        break;

    // States after connecting

    case sendCipsend:
    {
        char data;
        if (_serial.read(&data, 1) == 1)
        {
            if (data == '>')
            {
                _serial.clearWriteBarrier();
                _lineRead = true;
                _connectState = connected;
            }
        }
        break;
    }

    case sendCiprxget4:
        _replyState = ciprxget4;
        SEND_COMMAND("AT+CIPRXGET=4,0", _okStr, sendCiprxget2);

    case sendCiprxget2:
        if (_bytesToReceive > 0 && _serial.spaceAvailable() > 4)
        {
            int bytesToReceive = _serial.spaceAvailable() - 4;
            if (bytesToReceive > _bytesToReceive)
                bytesToReceive = _bytesToReceive;
            const char str[] = "AT+CIPRXGET=2,0,";
            char sizeStr[6];
            sprintf(sizeStr, "%d", bytesToReceive);
            _serial.write(str, sizeof(str) - 1);
            _serial.write(sizeStr, strlen(sizeStr));
            _serial.write(_lineEndStr, LINE_END_STR_LENGTH);
            _replyState = ciprxget2;
            _waitForReply = _okStr;
            _connectState = waitReceive;
        }
        break;

    case sendNetclose:
        SEND_COMMAND("AT+NETCLOSE", "+NETCLOSE: 0", sendAth);

    case sendAth:
        SEND_COMMAND("ATH", _okStr, finalizeDisconnect);

    case finalizeDisconnect:
        _connectState = notConnected;
        break;

    default:
        break;
    }
}
