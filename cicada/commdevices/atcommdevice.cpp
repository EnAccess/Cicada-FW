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

#include "cicada/commdevices/atcommdevice.h"
#include <cinttypes>
#include <cstddef>
#include <cstdio>
#include <cstring>

#ifdef CICADA_DEBUG
#include "printf.h"
#endif

#define MIN_SPACE_AVAILABLE 22

using namespace Cicada;

const char* ATCommDevice::_okStr = "OK";
const char* ATCommDevice::_lineEndStr = "\r\n";
const char* ATCommDevice::_quoteEndStr = "\"\r\n";

ATCommDevice::ATCommDevice(
    IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize) :
    IPCommDevice(readBuffer, writeBuffer, bufferSize), _serial(serial)
{}

ATCommDevice::ATCommDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
    Size readBufferSize, Size writeBufferSize) :
    IPCommDevice(readBuffer, writeBuffer, readBufferSize, writeBufferSize), _serial(serial)
{}

void ATCommDevice::logStates(int8_t sendState, int8_t replyState)
{
#ifdef CICADA_DEBUG
    if (_connectState < connected) {
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

void ATCommDevice::flushReadBuffer()
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

bool ATCommDevice::handleDisconnect(int8_t nextState)
{
    if (_stateBooleans & DISCONNECT_PENDING) {
        _stateBooleans &= ~DISCONNECT_PENDING;
        _sendState = nextState;

        return true;
    }

    return false;
}

bool ATCommDevice::handleConnect(int8_t nextState)
{
    if (_stateBooleans & CONNECT_PENDING) {
        _stateBooleans &= ~CONNECT_PENDING;
        _sendState = nextState;

        return true;
    }

    return false;
}

bool ATCommDevice::prepareSending()
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

void ATCommDevice::sendData()
{
    while (_bytesToWrite--) {
        _serial.write(_writeBuffer.pull());
    }
}

bool ATCommDevice::receive()
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

void ATCommDevice::sendCommand(const char* cmd)
{
    _serial.write((const uint8_t*)cmd);
    _serial.write((const uint8_t*)_lineEndStr);
}
