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

#ifndef LINE_CIRCULAR_BUFFER_H
#define LINE_CIRCULAR_BUFFER_H

#include "cicada/circularbuffer.h"
#include <cstdint>

namespace Cicada {

/*!
 * \class LineCircularBuffer
 *
 * Extends the circular buffer for handling lines.
 */

class LineCircularBuffer : public CircularBuffer<char>
{
  public:
    LineCircularBuffer(char* buffer, Size bufferSize) :
        CircularBuffer(buffer, bufferSize), _bufferedLines(0)
    {}

    Size push(const char* data, Size size) override
    {
        if (size > CircularBuffer<char>::spaceAvailable())
            size = CircularBuffer<char>::spaceAvailable();

        Size writeCount = 0;

        while (writeCount < size) {
            push(data[writeCount++]);
        }

        return writeCount;
    }

    void push(char data) override
    {
        CircularBuffer<char>::push(data);

        if (data == '\n') {
            _bufferedLines++;
        }
    }

    virtual Size pull(char* data, Size size) override
    {
        if (size > CircularBuffer<char>::bytesAvailable())
            size = CircularBuffer<char>::bytesAvailable();

        Size readCount = 0;

        while (readCount < size) {
            data[readCount++] = pull();
        }

        return readCount;
    }

    char pull() override
    {
        char data = CircularBuffer<char>::pull();

        if (data == '\n') {
            _bufferedLines--;
        }

        return data;
    }

    /*!
     * \return Number of lines currently in the buffer
     */
    inline uint16_t numBufferedLines() const
    {
        return _bufferedLines;
    }

    /*!
     * Reads a full line from the buffer.
     * \param data Pointer where pulled data will be stored
     * \param size Available space in data
     * \return Actual number of characters pulled from the buffer
     */
    Size readLine(char* data, Size size)
    {
        Size readCount = 0;
        char c = '\0';

        while (!CircularBuffer<char>::isEmpty() && c != '\n') {
            c = pull();
            if (readCount < size) {
                data[readCount++] = c;
            }
        }

        return readCount;
    }

    virtual void flush()
    {
        _bufferedLines = 0;
        CircularBuffer::flush();
    }

  private:
    uint16_t _bufferedLines;
};

}

#endif
