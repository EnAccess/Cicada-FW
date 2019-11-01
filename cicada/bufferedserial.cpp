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

#include "cicada/bufferedserial.h"
#include "cicada/irq.h"
#include <cstdint>

using namespace Cicada;

BufferedSerial::BufferedSerial(
    char* readBuffer, char* writeBuffer, Size readBufferSize, Size writeBufferSize) :
    _readBuffer(readBuffer, readBufferSize),
    _writeBuffer(writeBuffer, writeBufferSize)
{}

BufferedSerial::BufferedSerial(char* readBuffer, char* writeBuffer, Size bufferSize) :
    _readBuffer(readBuffer, bufferSize),
    _writeBuffer(writeBuffer, bufferSize)
{}

Size BufferedSerial::bytesAvailable() const
{
    eDisableInterrupts();
    Size availableData = _readBuffer.bytesAvailable();
    eEnableInterrupts();

    return availableData;
}

Size BufferedSerial::spaceAvailable() const
{
    eDisableInterrupts();
    Size spaceAvailable = _writeBuffer.spaceAvailable();
    eEnableInterrupts();

    return spaceAvailable;
}

Size BufferedSerial::read(uint8_t* data, Size size)
{
    Size avail = bytesAvailable();
    if (size > avail)
        size = avail;

    Size readCount = 0;

    while (readCount < size) {
        data[readCount++] = read();
    }

    return readCount;
}

uint8_t BufferedSerial::read()
{
    eDisableInterrupts();
    uint8_t c = _readBuffer.pull();
    eEnableInterrupts();

    return c;
}

Size BufferedSerial::write(const uint8_t* data, Size size)
{
    Size space = spaceAvailable();
    if (size > space)
        size = space;

    Size writeCount = 0;

    while (writeCount < size) {
        copyToBuffer(data[writeCount++]);
    }

    startTransmit();

    return writeCount;
}

Size BufferedSerial::write(const uint8_t* data)
{
    Size space = spaceAvailable();

    Size writeCount = 0;

    while (data[writeCount] != '\0' && writeCount < space) {
        copyToBuffer(data[writeCount++]);
    }

    startTransmit();

    return writeCount;
}

void BufferedSerial::write(uint8_t data)
{
    copyToBuffer(data);
    startTransmit();
}

void BufferedSerial::copyToBuffer(uint8_t data)
{
    eDisableInterrupts();
    _writeBuffer.push(data);
    eEnableInterrupts();
}

bool BufferedSerial::canReadLine() const
{
    eDisableInterrupts();
    Size lines = _readBuffer.numBufferedLines();
    eEnableInterrupts();

    return lines > 0;
}

Size BufferedSerial::readLine(uint8_t* data, Size size)
{
    Size readCount = 0;
    uint8_t c = '\0';

    if (size == 0)
        return 0;

    while (bytesAvailable() && c != '\n') {
        c = read();
        if (readCount < size - 1) {
            data[readCount++] = c;
        }
    }
    data[readCount] = '\0';

    return readCount;
}

void BufferedSerial::flushReceiveBuffers()
{
    eDisableInterrupts();
    _readBuffer.flush();
    eEnableInterrupts();
}

Size BufferedSerial::bufferSize()
{
    return _writeBuffer.size();
}

void BufferedSerial::transferToAndFromBuffer()
{
    if (_writeBuffer.bytesAvailable()) {
        if (rawWrite(_writeBuffer.read())) {
            _writeBuffer.pull();
        }
    }

    if (!_readBuffer.isFull()) {
        uint8_t data;
        if (rawRead(data)) {
            _readBuffer.push(data);
        }
    }
}
