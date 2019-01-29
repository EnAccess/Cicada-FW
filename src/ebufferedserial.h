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
        m_writeBarrier(false),
        m_bytesToWrite(0)
    { }

    /*!
     * Number of bytes available in the reading buffer.
     * \return Number of bytes available for reading
     */
    virtual uint16_t bytesAvailable()
    {
        return m_readBuffer.availableData();
    }

    virtual uint16_t spaceAvailable()
    {
        return m_writeBuffer.size() - m_writeBuffer.availableData();
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
        return m_readBuffer.pull(data, size);
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
        return m_writeBuffer.push(data, size);
    }

    /*!
     * \return true if a whole line is in the buffer, false otherwise
     */
    virtual bool canReadLine()
    {
        return m_readBuffer.numBufferedLines() > 0;
    }

    /*!
     * Reads a line.
     */
    virtual uint16_t readLine(char* data, uint16_t size)
    {
        return m_readBuffer.readLine(data, size);
    }

    virtual void setWriteBarrier()
    {
        m_bytesToWrite = m_writeBuffer.availableData();
        m_writeBarrier = true;
    }

    virtual void clearWriteBarrier()
    {
        m_bytesToWrite = 0;
        m_writeBarrier = false;
    }

    virtual void run()
    {
        if (m_writeBuffer.availableData() &&
            (!m_writeBarrier || m_bytesToWrite))
        {
            char data = m_writeBuffer.read();
            if (rawWrite((const uint8_t*)&data, 1) == 1)
            {
                m_writeBuffer.pull();
                m_bytesToWrite--;
            }
        }

        if (rawBytesAvailable() && !m_readBuffer.isFull())
        {
            char data;
            if (rawRead((uint8_t*)&data, 1) == 1)
            {
                m_readBuffer.push(data);
            }
        }
    }

private:
    ELineCircularBuffer<BUFFER_SIZE> m_readBuffer;
    ELineCircularBuffer<BUFFER_SIZE> m_writeBuffer;
    bool m_writeBarrier;
    uint16_t m_bytesToWrite;
};

typedef EBufferedSerial<E_DEFAULT_SERIAL_BUFFERSIZE> EDefaultBufferedSerial;

#endif
