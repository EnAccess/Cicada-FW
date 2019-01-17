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

#ifndef ETERMIOS_H
#define ETERMIOS_H

#include <stdint.h>
#include "eiserial.h"

class ETermios : EISerial
{
public:
    /*!
     * Construct a new ETermios object with the given serial port,
     * for example /dev/ttyUSB0. The String is not copied and must
     * be valid for the object's lifetime.
     * \param port Name of the serial port
     */
    ETermios(const char* port);

    virtual bool open();

    virtual bool setSerialConfig(uint32_t baudRate, uint8_t dataBits);

    virtual void close();

    virtual uint16_t bytesAvailable() const;

    virtual uint16_t read(uint8_t* data, uint16_t maxSize);

    virtual uint16_t write(const uint8_t* data, uint16_t size);

    /*!
     * Name of the serial port used.
     * \return Pointer to the string given to the constructor
     */
    inline const char* portName() const { return m_port; }

private:
    const char* m_port;
    int m_fd;
};

#endif
