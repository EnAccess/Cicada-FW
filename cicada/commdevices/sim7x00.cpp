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

#include "cicada/commdevices/sim7x00.h"
#include "cicada/commdevices/ipcommdevice.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

using namespace EnAccess;

Sim7x00CommDevice::Sim7x00CommDevice(IBufferedSerial& serial) :
    SimCommDevice(serial)
{}

void Sim7x00CommDevice::run()
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
        _serial.write(_lineEndStr);
        _replyState = okReply;
        _waitForReply = "RDY";

        setDelay(4000);

        return;
    }

    // Buffer reply from the modem
    bool parseLine = fillLineBuffer();

    // Check if there is a reply from the modem
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

        // Process replies which need special treatment
        switch (_replyState) {
        case netopen:
            if (_waitForReply == NULL) {
                _replyState = okReply;
            }
            else if (strncmp(_lineBuffer, "+NETOPEN: 1", 11) == 0) {
                setDelay(2000);
                _sendState = sendNetopen;
                _waitForReply = NULL;
                _replyState = okReply;
                return;
            }
            break;

        case cdnsgip:
            if (parseDnsReply()) {
                _replyState = okReply;
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

        default:
            break;
        }

        // In connected state, check for new data or IP connection close
        if (_sendState >= connected) {
            checkConnectionState("+IPCLOSE: 0,");
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
        _sendState = sendCgdcont;
        sendCommand("ATE1");
        break;

    case sendCgdcont: {
        const char str[] = "AT+CGDCONT=1,\"IP\",\"";
        _serial.write(str, sizeof(str) - 1);
        _serial.write(_apn, strlen(_apn));
        _serial.write(_quoteEndStr);

        _waitForReply = _okStr;
        _sendState = sendAtd;
        break;
    }

    case sendAtd:
        setDelay(500);
        _waitForReply = "CONNECT";
        _sendState = sendPpp;
        sendCommand("ATD*99#");
        break;

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
        _waitForReply = _okStr;
        _sendState = sendCipmode;
        sendCommand("AT+CSOCKSETPN=1");
        break;

    case sendCipmode:
        _waitForReply = _okStr;
        _sendState = sendNetopen;
        sendCommand("AT+CIPMODE=0");
        break;

    case sendNetopen:
        setDelay(10);
        _waitForReply = "+NETOPEN: 0";
        _sendState = sendCiprxget;
        _replyState = netopen;
        sendCommand("AT+NETOPEN");
        break;

    case sendCiprxget:
        _waitForReply = _okStr;
        _sendState = sendDnsQuery;
        sendCommand("AT+CIPRXGET=1");
        break;

    case sendDnsQuery:
        if (SimCommDevice::sendDnsQuery()) {
            _replyState = cdnsgip;
            _waitForReply = _okStr;
            _sendState = sendCipopen;
        }
        break;

    case sendCipopen: {
        SimCommDevice::sendCipstart("OPEN");

        _waitForReply = "+CIPOPEN: 0,0";
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
        if (_writeBuffer.availableData()) {
            if (prepareSending()) {
                _connectState = IPCommDevice::transmitting;
                _sendState = sendData;
            }
        } else if (_stateBooleans & DATA_PENDING) {
            _stateBooleans &= ~DATA_PENDING;
            _connectState = IPCommDevice::transmitting;
            _sendState = sendCiprxget4;
        } else {
            handleDisconnect(sendNetclose);
        }
        break;

        // States after connecting

    case sendData:
        SimCommDevice::sendData();
        _waitForReply = _okStr;
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
        if (handleDisconnect(sendNetclose))
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
        if (handleDisconnect(sendNetclose))
            break;

        handleConnect(sendCipopen);
        break;

    case sendNetclose:
        _connectState = IPCommDevice::intermediate;
        _waitForReply = "+NETCLOSE: 0";
        _sendState = sendAth;
        sendCommand("AT+NETCLOSE");
        break;

    case sendAth:
        _waitForReply = _okStr;
        _sendState = finalizeDisconnect;
        sendCommand("ATH");
        break;

    case finalizeDisconnect:
        _connectState = IPCommDevice::notConnected;
        _sendState = notConnected;
        break;

    default:
        break;
    }
}
