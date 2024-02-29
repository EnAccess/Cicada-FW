/*
 * Cicada communication library
 * Copyright (C) 2023 Okrasolar
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

#include "cicada/commdevices/cc1352p7.h"
#include "cicada/commdevices/atcommdevice.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace Cicada;

const uint16_t CC1352P7_MAX_RX = 1220;   // Match network buffer of the modem

CC1352P7CommDevice::CC1352P7CommDevice(
    IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize) :
    ATCommDevice(serial, readBuffer, writeBuffer, bufferSize)
{
    resetStates();
}

CC1352P7CommDevice::CC1352P7CommDevice(IBufferedSerial& serial, uint8_t* readBuffer,
    uint8_t* writeBuffer, Size readBufferSize, Size writeBufferSize) :
    ATCommDevice(serial, readBuffer, writeBuffer, readBufferSize, writeBufferSize)
{
    resetStates();
}

void CC1352P7CommDevice::resetStates()
{
    _serial.flushReceiveBuffers();
    _readBuffer.flush();
    _writeBuffer.flush();
    _lbFill = 0;
    _sendState = 0;
    _replyState = 0;
    _connectState = IPCommDevice::notConnected;
    _bytesToWrite = 0;
    _bytesToReceive = 0;
    _bytesToRead = 0;
    _waitForReply = NULL;
    _stateBooleans = LINE_READ;
}

bool CC1352P7CommDevice::fillLineBuffer()
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
        }
    }
    return false;
}

bool CC1352P7CommDevice::sendCiprcvdata()
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
        if (bytesToReceive > CC1352P7_MAX_RX)
            bytesToReceive = CC1352P7_MAX_RX;

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

bool CC1352P7CommDevice::parseCiprecvdata()
{
    if (strncmp(_lineBuffer, "+CIPRECVDATA", 12) == 0) {
        int bytesToRead;
        sscanf(_lineBuffer + 13, "%d", &bytesToRead);
        _bytesToReceive -= bytesToRead;
        _bytesToRead += bytesToRead;
        _stateBooleans &= ~LINE_READ;
        return true;
    }

    return false;
}

void CC1352P7CommDevice::run()
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
        _bytesToRead = 0;
        _bytesToReceive = 0;
        _bytesToWrite = 0;
        _sendState = sendCipclose;
        _replyState = okReply;
        _waitForReply = NULL;
        _stateBooleans &= ~RESET_PENDING;
        if (_connectState >= intermediate) {
            connect();
        }
    }

    // Buffer data from the modem
    bool parseLine = fillLineBuffer();

    // Check if there is data from the modem
    if (parseLine) {

        // Log the current modem states
        logStates(_sendState, _replyState);

        // Handle error states
        if (strncmp(_lineBuffer, "ERROR", 5) == 0
            || (_sendState >= connected && strncmp(_lineBuffer, "SEND FAIL", 9) == 0)) {
            _stateBooleans |= RESET_PENDING;
            _connectState = generalError;
            _waitForReply = NULL;
            return;
        }

        // If sent a command, process standard reply
        if (_waitForReply) {
            if (strncmp(_lineBuffer, _waitForReply, strlen(_waitForReply)) == 0) {
                _waitForReply = NULL;
            }
        }

        // Process incoming data which need special treatment
        switch (_replyState) {
        case parseStateCiprecvdata:
            if (parseCiprecvdata()) {
                _replyState = okReply;
                _sendState = receiving;
            }
            break;

        default:
            break;
        }

        // In connected state, check for new data or IP connection close
        if (_sendState >= connected) {
            if (strncmp(_lineBuffer, "+IPD,", 4) == 0) {
                int bytes;
                sscanf(_lineBuffer + 5, "%d", &bytes);
                _bytesToReceive = bytes;
                _stateBooleans |= DATA_PENDING;
            } else if (strncmp(_lineBuffer, "CLOSED", 6) == 0) {
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
        _connectState = IPCommDevice::notConnected;
        handleConnect(connecting);
        break;

    case connecting:
        _connectState = IPCommDevice::intermediate;
        _stateBooleans |= LINE_READ;
        _waitForReply = _okStr;
        _sendState = sendCipstart;
        sendCommand("ATE0");
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
        _connectState = IPCommDevice::connected;
        _sendState = connected;
        _stateBooleans |= IP_CONNECTED;
        break;

    case connected:
        if (_writeBuffer.bytesAvailable()) {
            if (prepareSending(false)) {
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
                if (handleDisconnect(sendCipclose)) {
                }
            } else {
                _stateBooleans &= ~DISCONNECT_PENDING;
                _sendState = finalizeDisconnect;
            }
        }
        break;

        // States after connecting

    case sendDataState:
        sendData();
        _waitForReply = _okStr;
        _sendState = connected;
        break;

    case sendCiprecvdata:
        if (handleDisconnect(sendCipclose)) {
            break;
        }

        if (_bytesToReceive > 0) {
            if (sendCiprcvdata()) {
                _sendState = waitReceive;
                _replyState = parseStateCiprecvdata;
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
        _waitForReply = _okStr;
        _sendState = finalizeDisconnect;
        sendCommand("AT+CIPCLOSE");
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
