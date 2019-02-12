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

#include <cstdint>
#include "ebufferedserial.h"

EBufferedSerial::EBufferedSerial() :
    _writeBarrier(false),
    _bytesToWrite(0)
{ }

uint16_t EBufferedSerial::bytesAvailable() const
{
    return _readBuffer.availableData();
}

uint16_t EBufferedSerial::spaceAvailable() const
{
    return _writeBuffer.size() - _writeBuffer.availableData();
}

uint16_t EBufferedSerial::read(char* data, uint16_t size)
{
    return _readBuffer.pull(data, size);
}

char EBufferedSerial::read()
{
    return _readBuffer.pull();
}

uint16_t EBufferedSerial::write(const char* data, uint16_t size)
{
    return _writeBuffer.push(data, size);
}

void EBufferedSerial::write(char data)
{
    _writeBuffer.push(data);
}

bool EBufferedSerial::canReadLine() const
{
    return _readBuffer.numBufferedLines() > 0;
}

uint16_t EBufferedSerial::readLine(char* data, uint16_t size)
{
    return _readBuffer.readLine(data, size);
}

void EBufferedSerial::setWriteBarrier()
{
    _bytesToWrite = _writeBuffer.availableData();
    _writeBarrier = true;
}

void EBufferedSerial::clearWriteBarrier()
{
    _bytesToWrite = 0;
    _writeBarrier = false;
}

void EBufferedSerial::flushReceiveBuffers()
{
    _readBuffer.flush();
}

uint16_t EBufferedSerial::bufferSize()
{
    return _writeBuffer.size();
}

void EBufferedSerialTask::run()
{
    if (_writeBuffer.availableData() &&
        (!_writeBarrier || _bytesToWrite))
    {
        if (rawWrite(_writeBuffer.read()))
        {
            _writeBuffer.pull();
            _bytesToWrite--;
        }
    }

    if (rawBytesAvailable() && !_readBuffer.isFull())
    {
        uint8_t data;
        if (rawRead(data))
        {
            _readBuffer.push(data);
        }
    }
}
