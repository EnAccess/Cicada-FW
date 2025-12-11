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

#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include "cicada/types.h"
#include <cstdint>

namespace Cicada {

/*!
 * \class CircularBuffer
 *
 * Implementation of a circular buffer.
 */

template <typename T> class CircularBuffer
{
  public:
    /*!
     * Constructs a CirculerBuffer with a user provided underlying raw buffer.
     * \param buffer Pointer to the raw buffer to store data. This buffer must
     * remain valid during the whole lifetime of the CircularBuffer accessing it.
     * \bufferSize Number of entries available in the buffer.
     */
    CircularBuffer(T* buffer, Size bufferSize) :
        _writeHead(0), _readHead(0), _availableData(0), _bufferSize(bufferSize), _buffer(buffer)
    {}

    virtual ~CircularBuffer() {}

    /*!
     * Push data into the buffer. Data is copied.
     * \param pointer to the data to be copied into the buffer
     * \param size number of elements in data
     */
    // TODO: Check if virtual is appropriate
    virtual Size push(const T* data, Size size)
    {
        if (size > spaceAvailable())
            size = spaceAvailable();

        Size writeCount = 0;

        while (writeCount < size) {
            _buffer[_writeHead] = data[writeCount++];
            incrementOrResetHead(_writeHead);
        }
        _availableData += writeCount;

        return writeCount;
    }

    /*!
     * Pushes one elementinto the buffer. This function
     * does not check for available space in the buffer.
     * If there is no available space, an existing element
     * will be overwritten.
     * \param data Element to push into the buffer
     */
    virtual void push(T data)
    {
        _buffer[_writeHead] = data;
        incrementOrResetHead(_writeHead);
        if (_availableData < _bufferSize)
            _availableData++;
    }

    /*!
     * Pull data from the buffer
     * \param data Pointer where pulled data will be stored
     * \param size Maximum size to pull
     * \return Actual number of elements pulled from the buffer
     */
    virtual Size pull(T* data, Size size)
    {
        if (size > bytesAvailable())
            size = bytesAvailable();

        Size readCount = 0;

        while (readCount < size) {
            data[readCount++] = _buffer[_readHead];
            incrementOrResetHead(_readHead);
        }
        _availableData -= readCount;

        return readCount;
    }

    /*!
     * Pull a single element from the buffer. This function does not
     * check if the buffer is empty, in which case old data will
     * be returned.
     * \return The element pulled from the buffer
     */
    virtual T pull()
    {
        T data = _buffer[_readHead];
        incrementOrResetHead(_readHead);
        if (_availableData > 0)
            _availableData--;

        return data;
    }

    /*!
     * Read a single element without removing it from the buffer.
     * This function does not check if the buffer is empty,
     * in which case old data will be returned.
     * \return The element read from the buffer
     */
    virtual T read()
    {
        return _buffer[_readHead];
    }

    /*!
     * Empties the buffer by resetting all counters to zero.
     */
    virtual void flush()
    {
        _writeHead = 0;
        _readHead = 0;
        _availableData = 0;
    }

    /*!
     * \return true if the buffer is empty, false if there is data in it
     */
    virtual bool isEmpty() const
    {
        return _availableData == 0;
    }

    /*!
     * \return true if the buffer is full, false if there is still space
     */
    virtual bool isFull() const
    {
        return _availableData == _bufferSize;
    }

    /*!
     * \return Number of available elements in the buffer
     */
    virtual Size bytesAvailable() const
    {
        return _availableData;
    }

    /*!
     * \return Number of available space in the buffer
     */
    virtual Size spaceAvailable() const
    {
        return _bufferSize - _availableData;
    }

    /*!
     * \return size of the buffer, which was specified at compile time
     */
    virtual Size size() const
    {
        return _bufferSize;
    }

    /*!
     * Rewinds the read head.
     * Useful to re-read old data which has been pulled before.
     * \param num number of entries to rewind the read head
     */
    virtual void rewindReadHead(Size num)
    {
        if (num <= _readHead) {
            _readHead -= num;
        } else {
            _readHead = _bufferSize + _readHead - num;
        }
        _availableData += num;
    }

  private:
    Size _writeHead;
    Size _readHead;
    Size _availableData;
    const Size _bufferSize;
    T* _buffer;

    void incrementOrResetHead(Size& head)
    {
        head++;
        if (head >= _bufferSize)
            head = 0;
    }
};

}

#endif
