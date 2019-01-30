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

/*!
 * \class ECircularBuffer
 *
 * Implementation of a circular buffer.
 */

template <typename T, uint16_t BUFFER_SIZE>
class ECircularBuffer
{
public:
    ECircularBuffer() :
        m_writeHead(0),
        m_readHead(0),
        m_availableData(0),
        m_buffer()
    { }

    virtual ~ECircularBuffer()
    { }

    /*!
     * Push data into the buffer. Data is copied.
     * \param pointer to the data to be copied into the buffer
     * \param size number of elements in data
     */
    //TODO: Check if virtual is appropriate
    virtual uint16_t push(const T* data, uint16_t size)
    {
        if (size > BUFFER_SIZE)
            size = BUFFER_SIZE;

        uint16_t writeCount = 0;

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
    virtual void push(T data)
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
    virtual uint16_t pull(T* data, uint16_t size)
    {
        uint16_t readCount = 0;

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
    virtual T pull()
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
    virtual T read()
    {
        return m_buffer[m_readHead];
    }

    /*!
     * Empties the buffer by resetting all counters to zero.
     */
    virtual void flush()
    {
        m_writeHead = 0;
        m_readHead = 0;
        m_availableData = 0;
    }

    /*!
     * \return true if the buffer is empty, false if there is data in it
     */
    virtual bool isEmpty()
    {
        return m_availableData == 0;
    }

    /*!
     * \return true if the buffer is full, false if there is still space
     */
    virtual bool isFull()
    {
        return m_availableData == BUFFER_SIZE;
    }

    /*!
     * \return Number of available elements in the buffer
     */
    virtual uint16_t availableData()
    {
        return m_availableData;
    }

    /*!
     * \return size of the buffer, which was specified at compile time
     */
    virtual uint16_t size()
    {
        return BUFFER_SIZE;
    }

private:
    uint16_t m_writeHead;
    uint16_t m_readHead;
    uint16_t m_availableData;
    T m_buffer[BUFFER_SIZE];

    void incrementOrResetHead(uint16_t& head)
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
    }
};

/*!
 * \class ELineCircularBuffer
 *
 * Extends the circular buffer for handling lines.
 */

template <uint16_t BUFFER_SIZE>
class ELineCircularBuffer : public ECircularBuffer<char, BUFFER_SIZE>
{
public:
    ELineCircularBuffer() :
        m_bufferedLines(0)
        { }

    using ECircularBuffer<char, BUFFER_SIZE>::push;

    void push(char data)
    {
        ECircularBuffer<char, BUFFER_SIZE>::push(data);

        if (data == '\n')
        {
            m_bufferedLines++;
        }
    }

    using ECircularBuffer<char, BUFFER_SIZE>::pull;

    char pull()
    {
        char data = ECircularBuffer<char, BUFFER_SIZE>::pull();

        if (data == '\n')
        {
            m_bufferedLines--;
        }

        return data;
    }

    /*!
     * \return Number of lines currently in the buffer
     */
    inline uint16_t numBufferedLines()
    {
        return m_bufferedLines;
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

        while (!ECircularBuffer<char, BUFFER_SIZE>::isEmpty() && c != '\n') {
            c = pull();
            if (readCount < size)
            {
                data[readCount++] = c;
            }
        }

        return readCount;
    }

private:
    uint16_t m_bufferedLines;
};

#endif
