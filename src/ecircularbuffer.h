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

#ifndef ECIRCULARBUFFER_H
#define ECIRCULARBUFFER_H

template <typename T, uint8_t BUFFER_SIZE>
class ECircularBuffer
{
public:
    ECircularBuffer() :
        m_writeHead(0),
        m_readHead(0),
        m_availableData(0),
        m_buffer()
    { }

    /*!
     * Push data into the buffer. Data is copied.
     * \param pointer to the data to be copied into the buffer
     * \param size number of elements in data
     */
    uint8_t push(const T* data, uint8_t size)
    {
        if (size > BUFFER_SIZE)
            size = BUFFER_SIZE;

        uint8_t writeCount = 0;

        while (!isFull() && writeCount < size) {
            push(data[writeCount++]);
        }

        return writeCount;
    }

    /*!
     * Pushes one elementinto the buffer. This function
     * does not check for available space in the buffer.
     * If there is no available space, an existing element
     * will be overwritten.
     * \param data Element to push into the buffer
     */
    void push(T data)
    {
        m_buffer[m_writeHead] = data;
        incrementOrResetHead(m_writeHead);
        increaseAvailableDataCount();
    }

    /*!
     * Pull data from the buffer
     * \param data Pointer where pulled data will be stored
     * \param size Maximum size to pull
     * \return Actual number of elements pulled from the buffer
     */
    uint8_t pull(T* data, uint8_t size)
    {
        uint8_t readCount = 0;

        while (!isEmpty() && readCount < size) {
            data[readCount++] = pull();
        }

        return readCount;
    }

    /*!
     * Pull a single element from the buffer. This function does not
     * check if the buffer is empty, in which case old data will
     * be returned.
     * \return The element pulled from the buffer
     */
    T pull()
    {
        T data = m_buffer[m_readHead];
        incrementOrResetHead(m_readHead);
        decreaseAvailableDataCount();

        return data;
    }

    /*!
     * Read a single element without removing it from the buffer.
     * This function does not check if the buffer is empty,
     * in which case old data will be returned.
     * \return The element read from the buffer
     */
    T read()
    {
        return m_buffer[m_readHead];
    }

    /*!
     * Empties the buffer by resetting all counters to zero.
     */
    void flush()
    {
        m_writeHead = 0;
        m_readHead = 0;
        m_availableData = 0;
    }

    /*!
     * \return true if the buffer is empty, false if there is data in it
     */
    bool isEmpty()
    {
        return m_availableData == 0;
    }

    /*!
     * \return true if the buffer is full, false if there is still space
     */
    bool isFull()
    {
        return m_availableData == BUFFER_SIZE;
    }

    /*!
     * \return Number of available elements in the buffer
     */
    uint8_t availableData()
    {
        return m_availableData;
    }

    /*!
     * \return size of the buffer, which was specified at compile time
     */
    uint8_t size()
    {
        return BUFFER_SIZE;
    }

private:
    uint8_t m_writeHead;
    uint8_t m_readHead;
    uint8_t m_availableData;
    T m_buffer[BUFFER_SIZE];

    void incrementOrResetHead(uint8_t& head)
    {
        head++;
        if (head >= BUFFER_SIZE)
            head = 0;
    }

    void increaseAvailableDataCount()
    {
        // If we hit max data available,
        // make sure we keep bumping up the m_readHead to ensure FIFO
        m_availableData++;
        if (m_availableData > BUFFER_SIZE) {
            m_availableData = BUFFER_SIZE;
            m_readHead++;
        }
    }

    void decreaseAvailableDataCount()
    {
        m_availableData--;
        if (m_availableData <= 0) {
            m_availableData = 0;
        }
    }
};

template <uint8_t BUFFER_SIZE>
class ELineCircularBuffer : public ECircularBuffer<uint8_t, BUFFER_SIZE>
{
public:
    ELineCircularBuffer() :
        m_bufferedLines(0)
        { }

    using ECircularBuffer<uint8_t, BUFFER_SIZE>::push;

    void push(uint8_t data)
    {
        ECircularBuffer<uint8_t, BUFFER_SIZE>::push(data);

        if (data == '\n')
        {
            m_bufferedLines++;
        }
    }

    using ECircularBuffer<uint8_t, BUFFER_SIZE>::pull;

    uint8_t pull()
    {
        uint8_t data = ECircularBuffer<uint8_t, BUFFER_SIZE>::pull();

        if (data == '\n')
            m_bufferedLines--;

        return data;
        return 0;
    }

    inline uint8_t numBufferedLines()
    {
        return m_bufferedLines;
    }

    uint8_t readLine(uint8_t* data, uint8_t size)
    {
        uint8_t readCount = 0;
        uint8_t c = '\0';

        while (!ECircularBuffer<uint8_t, BUFFER_SIZE>::isEmpty() && c != '\n') {
            c = pull();
            if (readCount < size)
            {
                data[readCount++] = c;
            }
        }

        return readCount;
    }

private:
    uint8_t m_bufferedLines;
};

#endif
