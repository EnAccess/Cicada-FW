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

#include "bufferedserial.h"
#include "irq.h"
#include <cstdint>

using namespace EnAccess;

BufferedSerial::BufferedSerial() {}

uint16_t BufferedSerial::bytesAvailable() const
{
    eDisableInterrupts();
    uint16_t availableData = _readBuffer.availableData();
    eEnableInterrupts();

    return availableData;
}

uint16_t BufferedSerial::spaceAvailable() const
{
    eDisableInterrupts();
    uint16_t spaceAvailable = _writeBuffer.availableSpace();
    eEnableInterrupts();

    return spaceAvailable;
}

uint16_t BufferedSerial::read(char* data, uint16_t size)
{
    uint16_t avail = bytesAvailable();
    if (size > avail)
        size = avail;

    uint16_t readCount = 0;

    while (readCount < size) {
        data[readCount++] = read();
    }

    return readCount;
}

char BufferedSerial::read()
{
    eDisableInterrupts();
    char c = _readBuffer.pull();
    eEnableInterrupts();

    return c;
}

uint16_t BufferedSerial::write(const char* data, uint16_t size)
{
    uint16_t space = spaceAvailable();
    if (size > space)
        size = space;

    uint16_t writeCount = 0;

    while (writeCount < size) {
        write(data[writeCount++]);
    }

    return writeCount;
}

void BufferedSerial::write(char data)
{
    eDisableInterrupts();
    _writeBuffer.push(data);
    eEnableInterrupts();
}

bool BufferedSerial::canReadLine() const
{
    eDisableInterrupts();
    uint16_t lines = _readBuffer.numBufferedLines();
    eEnableInterrupts();

    return lines > 0;
}

uint16_t BufferedSerial::readLine(char* data, uint16_t size)
{
    uint16_t readCount = 0;
    char c = '\0';

    while (bytesAvailable() && c != '\n') {
        c = read();
        if (readCount < size) {
            data[readCount++] = c;
        }
    }

    return readCount;
}

void BufferedSerial::flushReceiveBuffers()
{
    eDisableInterrupts();
    _readBuffer.flush();
    eEnableInterrupts();
}

uint16_t BufferedSerial::bufferSize()
{
    return _writeBuffer.size();
}

void BufferedSerial::performReadWrite()
{
    if (_writeBuffer.availableData()) {
        if (rawWrite(_writeBuffer.read())) {
            _writeBuffer.pull();
        }
    }

    if (rawBytesAvailable() && !_readBuffer.isFull()) {
        uint8_t data;
        if (rawRead(data)) {
            _readBuffer.push(data);
        }
    }
}
