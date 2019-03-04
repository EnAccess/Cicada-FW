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

#ifndef ENOPLATFORM_H
#define ENOPLATFORM_H

#include "ebufferedserial.h"

class NoplatformSerial : public EDefaultBufferedSerial
{
public:
    bool open() { return false; }

    bool isOpen() { return false; }

    bool setSerialConfig(uint32_t baudRate, uint8_t dataBits) { return false; }

    void close() { }

    const char* portName() const { return NULL; }

protected:
    uint16_t rawBytesAvailable() const { return 0; }

    uint16_t rawRead(uint8_t* data, uint16_t maxSize) { return 0; }

    uint16_t rawWrite(const uint8_t* data, uint16_t size) { return 0; }
};

#endif
