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

#include "ibufferedserial.h"
#include "task.h"
#include "circularbuffer.h"
#include "linecircularbuffer.h"
#include "defines.h"

class BufferedSerial : public IBufferedSerial
{
public:
    BufferedSerial();

    virtual uint16_t bytesAvailable() const;

    virtual uint16_t spaceAvailable() const;

    virtual uint16_t read(char* data, uint16_t size);

    virtual char read();

    virtual uint16_t write(const char* data, uint16_t size);

    virtual void write(char data);

    virtual bool canReadLine() const;

    virtual uint16_t readLine(char* data, uint16_t size);

    virtual void flushReceiveBuffers();

    virtual uint16_t bufferSize();

    /*!
     * Actually perform read/write to the underlying
     * raw serial device.
     */
    virtual void performReadWrite();

protected:
    LineCircularBuffer<E_SERIAL_BUFFERSIZE> _readBuffer;
    LineCircularBuffer<E_SERIAL_BUFFERSIZE> _writeBuffer;
};

class BufferedSerialTask : public BufferedSerial, public Task
{
    inline void run() { performReadWrite(); }
};

#endif
