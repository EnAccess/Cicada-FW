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

#include "eiserial.h"
#include "etask.h"
#include "ecircularbuffer.h"

template <uint16_t BUFFER_SIZE>
class EBufferedSerial : public EISerial, public ETask
{
public:
    EBufferedSerial() :
        _writeBarrier(false),
        _bytesToWrite(0)
    { }

    /*!
     * Number of bytes available in the reading buffer.
     * \return Number of bytes available for reading
     */
    virtual uint16_t bytesAvailable() const
    {
        return _readBuffer.availableData();
    }

    /*!
     * Number of bytes available for writing in this buffer.
     * \return Number of bytes available for writing
     */
    virtual uint16_t spaceAvailable() const
    {
        return _writeBuffer.size() - _writeBuffer.availableData();
    }

    /*!
     * Reads data from the read buffer.
     * \param data Buffer to store data. Must be large enough to store
     * maxSize bytes.
     * \param maxSize Maximum number of bytes to store into data.
     * \return Number of bytes actually copied to data
     */
    virtual uint16_t read(char* data, uint16_t size)
    {
        return _readBuffer.pull(data, size);
    }

    /*!
     * Reads a single char from the buffer
     * \return The character read
     */
    virtual char read()
    {
        return _readBuffer.pull();
    }

    /*!
     * Writes data to the write buffer.
     * \param data Buffer with data copied to the write buffer.
     * If there is not enough space in the write buffer, the actual
     * number of bytes copied can be smaller than size.
     * \param size Number of bytes to write
     * \return Actual number of bytes written
     */
    virtual uint16_t write(const char* data, uint16_t size)
    {
        return _writeBuffer.push(data, size);
    }

    /*!
     * Writes a singla char to the buffer.
     * \param data Character to write
     */
    virtual void write(char data)
    {
        _writeBuffer.push(data);
    }

    /*!
     * \return true if a whole line is in the buffer, false otherwise
     */
    virtual bool canReadLine() const
    {
        return _readBuffer.numBufferedLines() > 0;
    }

    /*!
     * Reads a line.
     */
    virtual uint16_t readLine(char* data, uint16_t size)
    {
        return _readBuffer.readLine(data, size);
    }

    /*!
     * Sets the write barrier. A write barrier means, the BufferdSerial's
     * run function will not send further than up to the number of bytes
     * when the barrier was set. This is useful if there is data
     * to be buffered, but those should not yet be sent to the device.
     */
    virtual void setWriteBarrier()
    {
        _bytesToWrite = _writeBuffer.availableData();
        _writeBarrier = true;
    }

    /*!
     * Clears the write barrier. Data after the write barrier is now
     * allowed to be sent to the device.
     */
    virtual void clearWriteBarrier()
    {
        _bytesToWrite = 0;
        _writeBarrier = false;
    }

    /*!
     * Clears the read buffer, discarding all available data.
     */
    virtual void flushReceiveBuffers()
    {
        _readBuffer.flush();
    }

    /*!
     * \return Buffer size of read/write buffer
     */
    virtual uint16_t bufferSize()
    {
        return _writeBuffer.size();
    }

    virtual void run()
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
