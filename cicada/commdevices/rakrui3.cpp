/*
 * Cicada communication library
 * Copyright (C) 2022 Okrasolar
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

#include "cicada/commdevices/rakrui3.h"
#include <cinttypes>
#include <cstdio>
#include <cstring>

using namespace Cicada;

#define CONNECT_PENDING (1 << 0)
#define RESET_PENDING (1 << 1)
#define DATA_PENDING (1 << 2)
#define DISCONNECT_PENDING (1 << 3)
#define NETWORK_JOINED (1 << 4)
#define LINE_READ (1 << 5)

const char* RakDevice::_okStr = "OK";
const char* RakDevice::_lineEndStr = "\r\n";
// const char* RakDevice::_quoteEndStr = "\"\r\n";

// LoRaWAN maximum packet payload for different data rates. Taken from table 3 at
// https://lora-developers.semtech.com/documentation/tech-papers-and-guides/the-book/packet-size-considerations/
const uint8_t RakDevice::_packetSizes[14]
    = { 11, 51, 51, 115, 222, 222, 222, 222, 33, 109, 222, 222, 222, 222 };

RakDevice::RakDevice(
    IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize) :
    _serial(serial), _readBuffer(readBuffer, bufferSize), _writeBuffer(writeBuffer, bufferSize)
{
    resetStates();
}

RakDevice::RakDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
    Size readBufferSize, Size writeBufferSize) :
    _serial(serial),
    _readBuffer(readBuffer, readBufferSize),
    _writeBuffer(writeBuffer, writeBufferSize)
{
    resetStates();
}

void RakDevice::resetStates()
{
    setPort(1);
    _sendState = 0;
    _replyState = 0;
    _waitForReply = nullptr;
    _currentPacketSize = _packetSizes[0];
}

void RakDevice::setDevEUI(const char* eui)
{
    _devEui = eui;
}

void RakDevice::setAppEUI(const char* eui)
{
    _appEui = eui;
}

void RakDevice::setAppKey(const char* key)
{
    _appKey = key;
}

void RakDevice::setPort(uint8_t port)
{
    snprintf(_portStr, sizeof(_portStr), "%u", port);
}

bool RakDevice::connect()
{
    if (_appKey == nullptr || strlen(_appKey) < 32) {
        return false;
    }

    _stateBooleans |= CONNECT_PENDING;
    _stateBooleans |= LINE_READ;

    return true;
}

void RakDevice::disconnect()
{
    if (isIdle())
        return;

    _stateBooleans |= DISCONNECT_PENDING;
}

bool RakDevice::isConnected()
{
    return _sendState >= joined;
}

bool RakDevice::isIdle()
{
    return _sendState == notConnected;
}

Size RakDevice::bytesAvailable() const
{
    return _readBuffer.bytesAvailable();
}

Size RakDevice::spaceAvailable() const
{
    return _writeBuffer.spaceAvailable();
}

Size RakDevice::read(uint8_t* data, Size maxSize)
{
    return _readBuffer.pull(data, maxSize);
}

Size RakDevice::write(const uint8_t* data, Size size)
{
    if (_sendState < joined)
        return 0;

    return _writeBuffer.push(data, size);
}

bool RakDevice::writeBufferProcessed() const
{
    return _writeBuffer.bytesAvailable() == 0 && _sendState != waitForSend;
}

bool RakDevice::fillLineBuffer()
{
    // Buffer reply from modem in line buffer
    // Returns true when enough data to be parsed is available.
    if (_stateBooleans & LINE_READ) {
        while (_serial.bytesAvailable()) {
            char c = _serial.read();
            _lineBuffer[_lbFill++] = c;
            if (c == '\n' || c == '\r' || _lbFill == LINE_MAX_LENGTH) {
                _lineBuffer[_lbFill] = '\0';
                _lbFill = 0;
                return true;
            }
        }
    }
    return false;
}

void RakDevice::sendCommand(const char* cmd)
{
    _serial.write((const uint8_t*)cmd);
    _serial.write((const uint8_t*)_lineEndStr);
}

void RakDevice::run()
{
    // If the serial device is not yet open, try to open it
    if (!_serial.isOpen()) {
        if (!_serial.open()) {
            _sendState = serialError;
        }
        return;
    }

    // Buffer data from the modem
    bool parseLine = fillLineBuffer();

    // Check if there is data from the modem
    if (parseLine) {

        // Log the current modem states
#ifdef CICADA_DEBUG
        if (_waitForReply)
            printf("_sendState=%d, _replyState=%d, "
                   "_waitForReply=\"%s\", data: %s\n",
                _sendState, _replyState, _waitForReply, _lineBuffer);
        else
            printf("_sendState=%d, _replyState=%d, "
                   "_waitForReply=NULL, data: %s\n",
                _sendState, _replyState, _lineBuffer);
#endif

        // If sent a command, process standard reply
        if (_waitForReply) {
            if (strncmp(_lineBuffer, _waitForReply, strlen(_waitForReply)) == 0) {
                _waitForReply = NULL;
            }
        }

        // Process incoming data which need special treatment
        switch (_replyState) {
        case dataRate:
            if (strncmp(_lineBuffer, "AT+DR=", 6) == 0) {
                char* src = _lineBuffer + 6;
                if (*src == '?') {
                    break;
                }
                uint8_t dr;
                sscanf(src, "%" SCNu8, &dr);
                if (dr < sizeof(_packetSizes)) {
                    _currentPacketSize = _packetSizes[dr];
                } else {
                    _currentPacketSize = _packetSizes[0];
                }
                _replyState = okReply;
            }
            break;

        case sendConfirm:
            if (strncmp(_lineBuffer, "+EVT:SEND_CONFIRMED_FAILED", 26) == 0
                || strncmp(_lineBuffer, "AT_BUSY_ERROR", 13) == 0) {
                _writeBuffer.rewindReadHead(_bytesToResend);
                _replyState = okReply;
                _waitForReply = NULL;
            }
            break;

        default:
            break;
        }

        // In joined state, check for new data
        if (_sendState >= joined) {
            char* src = _lineBuffer + 6;
            int nColons = 0;
            if (strncmp(_lineBuffer, "+EVT:RX", 7) == 0) {
                while (*src) {
                    if (*src++ == ':') {
                        if (++nColons == 5) {
                            int b;
                            while (sscanf(src, "%02x", &b) == 1) {
                                _readBuffer.push((uint8_t)b);
                                src += 2;
                            }
                        }
                    }
                }
            }
        }
    }

    // Don't go on when waiting for a reply
    if (_waitForReply)   // || _replyState != okReply)
        return;

    // Don't go on if space in write buffer is low
    if (_serial.spaceAvailable() < 20)
        return;

    // Connection state machine
    switch (_sendState) {

    case notConnected:
        setDelay(10);
        if (_stateBooleans & CONNECT_PENDING) {
            _stateBooleans &= ~CONNECT_PENDING;
            _waitForReply = NULL;
            _sendState = sendDevEUI;
        }
        break;

    case sendDevEUI:
        if (_devEui) {
            _serial.write((const uint8_t*)"AT+DEVEUI=");
            _serial.write((const uint8_t*)_devEui);
            _serial.write((const uint8_t*)_lineEndStr);
            _waitForReply = _okStr;
        }
        _sendState = sendAppEUI;
        break;

    case sendAppEUI:
        if (_appEui) {
            _serial.write((const uint8_t*)"AT+APPEUI=");
            _serial.write((const uint8_t*)_appEui);
            _serial.write((const uint8_t*)_lineEndStr);
            _waitForReply = _okStr;
        }
        _sendState = sendAppKey;
        break;

    case sendAppKey:
        _serial.write((const uint8_t*)"AT+APPKEY=");
        _serial.write((const uint8_t*)_appKey);
        _serial.write((const uint8_t*)_lineEndStr);
        _waitForReply = _okStr;
        _sendState = sendClass;
        break;

    case sendClass:
        sendCommand("AT+CLASS=C");
        _waitForReply = _okStr;
        _sendState = sendDR;
        break;

    case sendDR:
        sendCommand("AT+DR=0");
        _waitForReply = _okStr;
        _sendState = join;
        break;

    case join:
        _waitForReply = "+EVT:JOINED";
        _sendState = finalizeJoin;
        sendCommand("AT+JOIN=1:0:8:4");
        break;

    case finalizeJoin:
        setDelay(0);
        _sendState = joined;
        _stateBooleans |= NETWORK_JOINED;
        break;

    case joined:
        if (_writeBuffer.bytesAvailable()) {
            _waitForReply = _okStr;
            _sendState = sendPacket;
            _replyState = dataRate;
            sendCommand("AT+DR=?");
        }
        break;

    case sendPacket: {
        _waitForReply = "+EVT:SEND_CONFIRMED_OK";
        //_waitForReply = _okStr;
        _sendState = waitForSend;
        _replyState = sendConfirm;
        _serial.write((const uint8_t*)"AT+SEND=");
        _serial.write((const uint8_t*)_portStr);
        _serial.write((const uint8_t*)":");
        int i;
        for (i = 0; i < _currentPacketSize && _writeBuffer.bytesAvailable(); i++) {
            char hexStr[3];
            char c = _writeBuffer.pull();
            sprintf(hexStr, "%02X", c);
            _serial.write((const uint8_t*)hexStr);
        }
        _bytesToResend = i;
        _serial.write((const uint8_t*)_lineEndStr);
        break;
    }

    case waitForSend:
        _sendState = joined;
        break;

    default:
        break;
    }
}
