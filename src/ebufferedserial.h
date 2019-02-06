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

#ifndef EBUFFEREDSERIAL_H
#define EBUFFEREDSERIAL_H

#define E_DEFAULT_SERIAL_BUFFERSIZE 1504

#include "eibufferedserial.h"
#include "etask.h"
#include "ecircularbuffer.h"

template <uint16_t BUFFER_SIZE>
class EBufferedSerial : public EIBufferedSerial, public ETask
{
public:
    EBufferedSerial() :
        _writeBarrier(false),
        _bytesToWrite(0)
    { }

    uint16_t bytesAvailable() const
    {
        return _readBuffer.availableData();
    }

    uint16_t spaceAvailable() const
    {
        return _writeBuffer.size() - _writeBuffer.availableData();
    }

    uint16_t read(char* data, uint16_t size)
    {
        return _readBuffer.pull(data, size);
    }

    char read()
    {
        return _readBuffer.pull();
    }

    uint16_t write(const char* data, uint16_t size)
    {
        return _writeBuffer.push(data, size);
    }

    void write(char data)
    {
        _writeBuffer.push(data);
    }

    bool canReadLine() const
    {
        return _readBuffer.numBufferedLines() > 0;
    }

    uint16_t readLine(char* data, uint16_t size)
    {
        return _readBuffer.readLine(data, size);
    }

    void setWriteBarrier()
    {
        _bytesToWrite = _writeBuffer.availableData();
        _writeBarrier = true;
    }

    void clearWriteBarrier()
    {
        _bytesToWrite = 0;
        _writeBarrier = false;
    }

    void flushReceiveBuffers()
    {
        _readBuffer.flush();
    }

    uint16_t bufferSize()
    {
        return _writeBuffer.size();
    }

    void run()
    {
        if (_writeBuffer.availableData() &&
            (!_writeBarrier || _bytesToWrite))
        {
            char data = _writeBuffer.read();
            if (rawWrite((const uint8_t*)&data, 1) == 1)
            {
                _writeBuffer.pull();
                _bytesToWrite--;
            }
        }

        if (rawBytesAvailable() && !_readBuffer.isFull())
        {
            char data;
            if (rawRead((uint8_t*)&data, 1) == 1)
            {
                _readBuffer.push(data);
            }
        }
    }

private:
    ELineCircularBuffer<BUFFER_SIZE> _readBuffer;
    ELineCircularBuffer<BUFFER_SIZE> _writeBuffer;
    bool _writeBarrier;
    uint16_t _bytesToWrite;
};

typedef EBufferedSerial<E_DEFAULT_SERIAL_BUFFERSIZE> EDefaultBufferedSerial;

#endif
