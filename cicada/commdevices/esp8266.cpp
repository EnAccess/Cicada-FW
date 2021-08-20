/*
 * Cicada communication library
 * Copyright (C) 2021 Okrasolar
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

#include "cicada/commdevices/esp8266.h"
#include <cstdio>
#include <cstring>

#define MIN_SPACE_AVAILABLE 22

using namespace Cicada;

const char* Esp8266Device::_okStr = "OK";
const char* Esp8266Device::_lineEndStr = "\r\n";
const char* Esp8266Device::_quoteEndStr = "\"\r\n";

const uint16_t ESP8266_MAX_RX = 2048;

Esp8266Device::Esp8266Device(
    IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize) :
    IPCommDevice(readBuffer, writeBuffer, bufferSize),
    _serial(serial)
{}

Esp8266Device::Esp8266Device(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
    Size readBufferSize, Size writeBufferSize) :
    IPCommDevice(readBuffer, writeBuffer, readBufferSize, writeBufferSize),
    _serial(serial)
{}


void Esp8266Device::resetStates()
{
    _serial.flushReceiveBuffers();
    _readBuffer.flush();
    _writeBuffer.flush();
    _sendState = 0;
    _replyState = 0;
}

void Esp8266Device::setSSID(const char* ssid)
{
    _ssid = ssid;
}

void Esp8266Device::setPassword(const char* passwd)
{
    _passwd = passwd;
}

bool Esp8266Device::connect()
{
    // TODO: Check for SSID / password

    return IPCommDevice::connect();
}

bool Esp8266Device::fillLineBuffer()
{
    // Buffer reply from modem in line buffer
    // Returns true when enough data to be parsed is available.
    if (_stateBooleans & LINE_READ) {
        while (_serial.bytesAvailable()) {
            char c = _serial.read();
            _lineBuffer[_lbFill++] = c;
            if (c == '\n' || c == '>' || _lbFill == LINE_MAX_LENGTH) {
                _lineBuffer[_lbFill] = '\0';
                _lbFill = 0;
                return true;
            }
            if (_replyState == waitCiprecvdata && c == ':' && _lbFill > 14 &&
                strncmp(_lineBuffer, "+CIPRECVDATA,", 13) == 0) {
                _replyState = parseCiprecvdata;
                _lineBuffer[_lbFill] = '\0';
                _lbFill = 0;
                return true;
            }
        }
    }
    return false;
}

void Esp8266Device::logStates(int8_t sendState, int8_t replyState)
{
#ifdef CICADA_DEBUG
    if (_connectState < IPCommDevice::connected) {
        if (_waitForReply)
            printf("_sendState=%d, _replyState=%d, "
                   "_waitForReply=\"%s\", data: %s",
                sendState, replyState, _waitForReply, _lineBuffer);
        else
            printf("_sendState=%d, _replyState=%d, "
                   "_waitForReply=NULL, data: %s",
                sendState, replyState, _lineBuffer);
    }
#endif
}

bool Esp8266Device::handleDisconnect(int8_t nextState)
{
    if (_stateBooleans & DISCONNECT_PENDING) {
        _stateBooleans &= ~DISCONNECT_PENDING;
        _sendState = nextState;

        return true;
    }

    return false;
}

bool Esp8266Device::handleConnect(int8_t nextState)
{
    if (_stateBooleans & CONNECT_PENDING) {
        _stateBooleans &= ~CONNECT_PENDING;
        _sendState = nextState;

        return true;
    }

    return false;
}

void Esp8266Device::sendCommand(const char* cmd)
{
    _serial.write((const uint8_t*)cmd);
    _serial.write((const uint8_t*)_lineEndStr);
}

bool Esp8266Device::prepareSending()
{
    if (_serial.spaceAvailable() < MIN_SPACE_AVAILABLE)
        return false;

    _bytesToWrite = _writeBuffer.bytesAvailable();
    if (_bytesToWrite > _serial.spaceAvailable() - MIN_SPACE_AVAILABLE) {
        _bytesToWrite = _serial.spaceAvailable() - MIN_SPACE_AVAILABLE;
    }

    // cmd
    _serial.write((const uint8_t*)"AT+CIPSEND=");

    // length
    char sizeStr[6];
    snprintf(sizeStr, sizeof(sizeStr), "%u", (int)_bytesToWrite);
    _serial.write((const uint8_t*)sizeStr);

    _waitForReply = ">";

    return true;
}

void Esp8266Device::sendData()
{
    while (_bytesToWrite--) {
        _serial.write(_writeBuffer.pull());
    }
}

bool Esp8266Device::sendCiprcvdata()
{
    if (_serial.readBufferSize() - _serial.bytesAvailable() > 30
        && _readBuffer.spaceAvailable() > 0) {
        // Make sure there is enough space in the serial buffer for the reply
        Size bytesToReceive = _serial.readBufferSize() - _serial.bytesAvailable() - 30;
        if (bytesToReceive > _bytesToReceive)
            bytesToReceive = _bytesToReceive;
        // Make sure there is enough space in the local device buffer
        if (bytesToReceive > _readBuffer.spaceAvailable())
            bytesToReceive = _readBuffer.spaceAvailable();
        if (bytesToReceive > ESP8266_MAX_RX)
            bytesToReceive = ESP8266_MAX_RX;

        char sizeStr[6];
        sprintf(sizeStr, "%u", (unsigned int)bytesToReceive);
        _serial.write((const uint8_t*)"AT+CIPRECVDATA=");
        _serial.write((const uint8_t*)sizeStr);
        _serial.write((const uint8_t*)_lineEndStr);
        return true;
    } else {
        return false;
    }
}

bool Esp8266Device::parseCiprcvdata()
{
    int bytesToRead;
    sscanf(_lineBuffer + 13, "%d", &bytesToRead);
    _bytesToReceive -= bytesToRead;
    _bytesToRead += bytesToRead;
    _stateBooleans &= ~LINE_READ;
    return true;
}

void Esp8266Device::flushReadBuffer()
{
    while (_bytesToRead && _serial.bytesAvailable()) {
        _serial.read();
        _bytesToRead--;
    }
    _bytesToReceive = 0;

    if (_bytesToRead == 0) {
        _stateBooleans |= LINE_READ;
    }
}

bool Esp8266Device::receive()
{
    if (_serial.bytesAvailable() >= _bytesToRead) {
        while (_bytesToRead) {
            _readBuffer.push(_serial.read());
            _bytesToRead--;
        }
        _stateBooleans |= LINE_READ;

        return true;
    } else {
        return false;
    }
}

void Esp8266Device::run()
{
    // If the serial device is net yet open, try to open it
    if (!_serial.isOpen()) {
        if (!_serial.open()) {
            _sendState = serialError;
        }
        return;
    }

    // If a modem reset is pending, handle it
    if (_stateBooleans & RESET_PENDING) {
        _serial.flushReceiveBuffers();
        _stateBooleans = LINE_READ;
        _bytesToRead = 0;
        _bytesToReceive = 0;
        _bytesToWrite = 0;
        if (_sendState >= connecting && _sendState <= receiving)
            _sendState = connecting;
        else
            _sendState = notConnected;
        _serial.write((const uint8_t*)"AT+RST");
        _serial.write((const uint8_t*)_lineEndStr);
        _replyState = okReply;
        _waitForReply = "ready";

        setDelay(10);

        return;
    }

    // Buffer data from the modem
    bool parseLine = fillLineBuffer();

    // Check if there is data from the modem
    if (parseLine) {

        // Log the current modem states
        logStates(_sendState, _replyState);

        // If sent a command, process standard reply
        if (_waitForReply) {
            if (strncmp(_lineBuffer, _waitForReply, strlen(_waitForReply)) == 0) {
                _waitForReply = NULL;
            } else if (strncmp(_lineBuffer, "ERROR", 5) == 0) {
                _stateBooleans |= RESET_PENDING;
                _connectState = generalError;
                _waitForReply = NULL;
                return;
            }
        }

        // Process incoming data which need special treatment
        if (_replyState == parseCiprecvdata) {
            if (parseCiprcvdata()) {
                _replyState = okReply;
                _sendState = receiving;
            }
        }

        // In connected state, check for new data or IP connection close
        if (_sendState >= connected) {
            if (strncmp(_lineBuffer, "+IPD,", 5) == 0) {
                int bytesToReceive;
                sscanf(_lineBuffer + 5, "%d", &bytesToReceive);
                _bytesToReceive = bytesToReceive;
                _stateBooleans |= DATA_PENDING;
            } else if (strncmp(_lineBuffer, "CLOSED", 6) == 0) {
                _waitForReply = NULL;
                _stateBooleans &= ~IP_CONNECTED;
            }
        }
    }

    // When disconnecting was requested, flush read buffer first
    else if ((_stateBooleans & DISCONNECT_PENDING) && _sendState == receiving) {
        flushReadBuffer();
    }

    // Don't go on when waiting for a reply
    if (_waitForReply || _replyState != okReply)
        return;

    // Don't go on if space in write buffer is low
    if (_serial.spaceAvailable() < 20)
        return;

    // Connection state machine
    switch (_sendState) {
    case notConnected:
        setDelay(10);
        _connectState = IPCommDevice::notConnected;
        handleConnect(connecting);
        break;

    case connecting:
        setDelay(10);
        _connectState = IPCommDevice::intermediate;
        _stateBooleans |= LINE_READ;
        _waitForReply = _okStr;
        _sendState = sendCwjap;
        sendCommand("ATE1");
        break;

    case sendCwjap:
        if (handleDisconnect(finalizeDisconnect))
            break;

        _serial.write((const uint8_t*)"AT+CWJAP=\"");
        _serial.write((const uint8_t*)_ssid, strlen(_ssid));
        _serial.write((const uint8_t*)"\",\"");
        _serial.write((const uint8_t*)_passwd, strlen(_passwd));
        _serial.write((const uint8_t*)_quoteEndStr);

        _waitForReply = _okStr;
        _sendState = sendCiprecvmode;
        break;

    case sendCiprecvmode:
        _waitForReply = _okStr;
        _sendState = sendCipmode;
        sendCommand("AT+CIPRECVMODE=1");
        break;

    case sendCipmode:
        _waitForReply = _okStr;
        _sendState = sendCipstart;
        sendCommand("AT+CIPMODE=0");
        break;

    case sendCipstart: {
        char portStr[6];
        snprintf(portStr, sizeof(portStr), "%u", _port);

        _serial.write((const uint8_t*)"AT+CIPSTART");
        if (_type == UDP) {
            _serial.write((const uint8_t*)"=\"UDP\",\"");
        } else {
            _serial.write((const uint8_t*)"=\"TCP\",\"");
        }
        // TODO: Escape characters
        _serial.write((const uint8_t*)_host);
        _serial.write((const uint8_t*)"\",");
        _serial.write((const uint8_t*)portStr);
        _serial.write((const uint8_t*)_lineEndStr);

        _waitForReply = _okStr;
        _sendState = finalizeConnect;
        break;
    }

    case finalizeConnect:
        setDelay(0);
        _connectState = IPCommDevice::connected;
        _sendState = connected;
        _stateBooleans |= IP_CONNECTED;
        break;

    case connected:
        if (_writeBuffer.bytesAvailable()) {
            if (prepareSending()) {
                _serial.write((const uint8_t*)_lineEndStr);
                _connectState = IPCommDevice::transmitting;
                _sendState = sendDataState;
            }
        } else if (_stateBooleans & DATA_PENDING) {
            _stateBooleans &= ~DATA_PENDING;
            _connectState = IPCommDevice::receiving;
            _sendState = sendCiprecvdata;
        } else {
            _connectState = IPCommDevice::connected;
            if (_stateBooleans & IP_CONNECTED) {
                handleDisconnect(sendCipclose);
            } else {
                handleDisconnect(sendCwqap);
            }
        }
        break;

    // States after connecting

    case sendDataState:
        sendData();
        _waitForReply = "SEND OK";
        _sendState = connected;
        break;

    case sendCiprecvdata:
        if (handleDisconnect(sendCipclose))
            break;

        if (_bytesToReceive > 0) {
            if (sendCiprcvdata()) {
                _sendState = waitReceive;
                _replyState = waitCiprecvdata;
            }
        } else {
            _sendState = connected;
        }

        break;

    case waitReceive:
        break;

    case receiving:
        if (_bytesToRead > 0) {
            if (receive()) {
                _replyState = okReply;
                _waitForReply = _okStr;
            }
        } else if (_bytesToReceive > 0) {
            _sendState = sendCiprecvdata;
        } else {
            _sendState = connected;
        }
        break;

    case sendCipclose:
        _connectState = IPCommDevice::intermediate;
        if (_stateBooleans & IP_CONNECTED) {
            _waitForReply = _okStr;
            _sendState = sendCwqap;
            sendCommand("AT+CIPCLOSE");
        } else {
            _sendState = sendCwqap;
        }
        break;

    case sendCwqap:
        _connectState = IPCommDevice::intermediate;
        _waitForReply = "WIFI DISCONNECT";
        _sendState = finalizeDisconnect;
        sendCommand("AT+CWQAP");
        break;

    case finalizeDisconnect:
        _stateBooleans &= ~IP_CONNECTED;
        _connectState = IPCommDevice::notConnected;
        _sendState = notConnected;
        break;

    default:
        break;
    }
}
