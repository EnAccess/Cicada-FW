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

template <uint16_t BUFFER_SIZE>
class LineCircularBuffer : public CircularBuffer<char, BUFFER_SIZE>
{
  public:
    LineCircularBuffer() :
        _bufferedLines(0)
    { }

    uint16_t push(const char* data, uint16_t size) override
    {
        if (size > CircularBuffer<char, BUFFER_SIZE>::spaceAvailable())
            size = CircularBuffer<char, BUFFER_SIZE>::spaceAvailable();

        uint16_t writeCount = 0;

        while (writeCount < size) {
            push(data[writeCount++]);
        }

        return writeCount;
    }

    void push(char data) override
    {
        CircularBuffer<char, BUFFER_SIZE>::push(data);

        if (data == '\n') {
            _bufferedLines++;
        }
    }

    virtual uint16_t pull(char* data, uint16_t size) override
    {
        if (size > CircularBuffer<char, BUFFER_SIZE>::bytesAvailable())
            size = CircularBuffer<char, BUFFER_SIZE>::bytesAvailable();

        uint16_t readCount = 0;

        while (readCount < size) {
            data[readCount++] = pull();
        }

        return readCount;
    }

    char pull() override
    {
        char data = CircularBuffer<char, BUFFER_SIZE>::pull();

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
    uint16_t readLine(char* data, uint16_t size)
    {
        uint16_t readCount = 0;
        char c = '\0';

        while (!CircularBuffer<char, BUFFER_SIZE>::isEmpty() && c != '\n') {
            c = pull();
            if (readCount < size) {
                data[readCount++] = c;
            }
        }

        return readCount;
    }

  private:
    uint16_t _bufferedLines;
};

}

#endif
