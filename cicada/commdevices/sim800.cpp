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

#include "cicada/commdevices/sim800.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace Cicada;

Sim800CommDevice::Sim800CommDevice(
    IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize) :
    SimCommDevice(serial, readBuffer, writeBuffer, bufferSize)
{}

Sim800CommDevice::Sim800CommDevice(IBufferedSerial& serial, uint8_t* readBuffer,
    uint8_t* writeBuffer, Size readBufferSize, Size writeBufferSize) :
    SimCommDevice(serial, readBuffer, writeBuffer, readBufferSize, writeBufferSize)
{}

void Sim800CommDevice::run()
{
    // If the serial device is net yet open, try to open it
    if (!_serial.isOpen()) {
        if (!_serial.open()) {
            _sendState = serialError;
        }
        return;
    }

    // If the serial device is locked, don't go on
    if (_stateBooleans & SERIAL_LOCKED)
        return;

    // If a modem reset is pending, handle it
    if (_stateBooleans & RESET_PENDING) {
        _serial.flushReceiveBuffers();
        _bytesToRead = 0;
        _bytesToReceive = 0;
        _bytesToWrite = 0;
        _sendState = sendCipshut;
        _replyState = okReply;
        _waitForReply = NULL;
        _stateBooleans &= ~RESET_PENDING;
        if (_connectState >= intermediate) {
            setDelay(2000);
            connect();
        }
    }

    // Buffer reply from the modem
    bool parseLine = fillLineBuffer();

    // Parse reply from the modem
    if (parseLine) {
        // Log the current modem states
        logStates(_sendState, _replyState);

        // Handle deactivated or error states
        if (strncmp(_lineBuffer, "+PDP: DEACT", 11) == 0
            || strncmp(_lineBuffer, "+CME ERROR", 10) == 0
            || strncmp(_lineBuffer, "ERROR", 5) == 0) {
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

        // Process replies which need special treatment
        switch (_replyState) {
        case cifsr: {
            // Validate IP address by checking for three dots
            uint8_t i = 0, p = 0;
            while (_lineBuffer[i]) {
                if (_lineBuffer[i++] == '.')
                    p++;
            }
            if (p == 3) {
                _replyState = okReply;
            }
        } break;

        case cdnsgip:
            if (parseDnsReply()) {
                _replyState = okReply;
            }
            break;

        case cipstart:
            if (handleDisconnect(sendCipshut)) {
                _replyState = okReply;
            } else if (_waitForReply == NULL) {
                _replyState = okReply;
            } else if (strncmp(_lineBuffer, "0, CONNECT FAIL", 15) == 0) {
                _stateBooleans |= RESET_PENDING;
                _connectState = generalError;
            }
            break;

        case ciprxget4:
            if (parseCiprxget4()) {
                _replyState = okReply;
            }
            break;

        case ciprxget2:
            if (parseCiprxget2()) {
                _replyState = okReply;
                _sendState = receiving;
            }
            break;

        case csq:
            if (parseCsq()) {
                _replyState = okReply;
            }

        default:
            break;
        }

        // In connected state, check for new data or IP connection close
        if (_sendState >= connected) {
            checkConnectionState("0, CLOSED");
        }
    }

    // When disconnecting was requested, flush read buffer
    else if ((_stateBooleans & DISCONNECT_PENDING) && _sendState == receiving) {
        flushReadBuffer();
    }

    // Don't go on when waiting for a reply
    if (_waitForReply || _replyState != okReply)
        return;

    // Don't go on if space in write buffer is low
    if (_serial.spaceAvailable() < 20)
        return;

    // When signal strength was requested, send the command to the modem
    if (_rssi == UINT8_MAX && _stateBooleans & LINE_READ) {
        _replyState = csq;
        _waitForReply = _okStr;
        sendCommand("AT+CSQ");
        return;
    }

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
        _sendState = sendCiprxget;
        sendCommand("ATE0");
        break;

    case sendCiprxget:
        _waitForReply = _okStr;
        _sendState = sendCipmux;
        sendCommand("AT+CIPRXGET=1");
        break;

    case sendCipmux:
        _waitForReply = _okStr;
        _sendState = sendCstt;
        sendCommand("AT+CIPMUX=1");
        break;

    case sendCstt: {
        const char str[] = "AT+CSTT=\"";
        _serial.write((const uint8_t*)str, sizeof(str) - 1);
        _serial.write((const uint8_t*)_apn);
        _serial.write((const uint8_t*)_quoteEndStr);

        _waitForReply = _okStr;
        _sendState = sendCiicr;
        break;
    }

    case sendCiicr:
        _waitForReply = _okStr;
        _sendState = sendCifsr;
        sendCommand("AT+CIICR");
        break;

    case sendCifsr: {
        if (handleDisconnect(sendCipshut))
            break;

        const char str[] = "AT+CIFSR";
        _serial.write((const uint8_t*)str, sizeof(str) - 1);
        _serial.write((const uint8_t*)_lineEndStr);

        _replyState = cifsr;
        _sendState = sendDnsQuery;
        break;
    }

    case sendDnsQuery:
        if (SimCommDevice::sendDnsQuery()) {
            _replyState = cdnsgip;
            _waitForReply = _okStr;
            _sendState = sendCipstart;
        }
        break;

    case sendCipstart: {
        char portStr[6];
        snprintf(portStr, sizeof(portStr), "%u", _port);

        _serial.write((const uint8_t*)"AT+CIPSTART");
        if (_type == UDP) {
            _serial.write((const uint8_t*)"=0,\"UDP\",\"");
        } else {
            _serial.write((const uint8_t*)"=0,\"TCP\",\"");
        }
        _serial.write((const uint8_t*)_ip);
        _serial.write((const uint8_t*)"\",");
        _serial.write((const uint8_t*)portStr);
        _serial.write((const uint8_t*)_lineEndStr);

        _replyState = cipstart;
        _waitForReply = "0, CONNECT OK";
        _sendState = finalizeConnect;
        break;
    }

    case finalizeConnect:
        setDelay(0);
        _connectState = IPCommDevice::connected;
        _replyState = okReply;
        _sendState = connected;
        _stateBooleans |= IP_CONNECTED;
        break;

    case connected:
        if (_writeBuffer.bytesAvailable()) {
            if (prepareSending()) {
                _serial.write((const uint8_t*)_lineEndStr);
                _connectState = IPCommDevice::transmitting;
                _sendState = sendData;
            }
        } else if (_stateBooleans & DATA_PENDING) {
            _stateBooleans &= ~DATA_PENDING;
            _connectState = IPCommDevice::transmitting;
            _sendState = sendCiprxget4;
        } else {
            handleDisconnect(sendCipclose);
        }
        break;

        // States after connecting

    case sendData:
        SimCommDevice::sendData();
        _waitForReply = "0, SEND OK";
        _connectState = IPCommDevice::connected;
        _sendState = connected;
        break;

    case sendCiprxget4:
        _waitForReply = _okStr;
        _sendState = sendCiprxget2;
        _replyState = ciprxget4;
        sendCommand("AT+CIPRXGET=4,0");
        break;

    case sendCiprxget2:
        if (handleDisconnect(sendCipclose))
            break;

        if (_bytesToReceive > 0) {
            if (SimCommDevice::sendCiprxget2()) {
                _sendState = waitReceive;
                _replyState = ciprxget2;
            }
        } else if (_stateBooleans & IP_CONNECTED) {
            _connectState = IPCommDevice::connected;
            _sendState = connected;
        } else {
            _sendState = ipUnconnected;
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
            _sendState = sendCiprxget2;
        } else {
            _sendState = sendCiprxget4;
        }
        break;

    case ipUnconnected:
        _connectState = IPCommDevice::intermediate;
        if (handleDisconnect(finalizeDisconnect))
            break;

        handleConnect(sendCipstart);
        break;

    case sendCipclose:
        _connectState = IPCommDevice::intermediate;
        if (_stateBooleans & IP_CONNECTED) {
            _waitForReply = "0, CLOSE OK";
            _sendState = sendCipshut;
            sendCommand("AT+CIPCLOSE=0");
        } else {
            _sendState = sendCipshut;
        }
        break;

    case sendCipshut:
        _connectState = IPCommDevice::intermediate;
        _waitForReply = "SHUT OK";
        _sendState = finalizeDisconnect;
        sendCommand("AT+CIPSHUT");
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
