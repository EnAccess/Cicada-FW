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

#ifndef EIBUFFEREDSERIAL_H
#define EIBUFFEREDSERIAL_H

#include "cicada/icommdevice.h"
#include "cicada/iserial.h"

namespace Cicada {

/*!
 * \class IBufferedSerial
 *
 * Interface for buffered serial access. The BufferedSerial class
 * implements this interface.
 */

class IBufferedSerial : public ICommDevice, public ISerial
{
  public:
    virtual ~IBufferedSerial() { }

    virtual uint16_t read(uint8_t* data, uint16_t maxSize) override = 0;
    virtual uint16_t write(const uint8_t* data, uint16_t size) override = 0;

    /*!
     * Reads a single char from the buffer
     * \return The character read
     */
    virtual uint8_t read() = 0;

    /*!
     * Writes a zero-terminated string to the write buffer,
     * but *without* the terminating zero byte.
     * \param data Buffer with string copied to the write buffer.
     * If there is not enough space in the write buffer, the string
     * will be concatenated and the actual number of bytes written
     * is returned.
     * \return Actual number of characters written
     */
    virtual uint16_t write(const uint8_t* data) = 0;

    /*!
     * Writes a singla char to the buffer.
     * \param data Character to write
     */
    virtual void write(uint8_t data) = 0;

    /*!
     * \return true if a whole line is in the buffer, false otherwise
     */
    virtual bool canReadLine() const = 0;

    /*!
     * Reads a line.
     */
    virtual uint16_t readLine(uint8_t* data, uint16_t size) = 0;

    /*!
     * Clears the read buffer, discarding all available data.
     */
    virtual void flushReceiveBuffers() = 0;

    /*!
     * \return Buffer size of read/write buffer
     */
    virtual uint16_t bufferSize() = 0;
};

}

#endif
