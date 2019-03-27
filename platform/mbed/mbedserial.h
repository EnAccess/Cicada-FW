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

#ifndef MBEDSERIAL_H
#define MBEDSERIAL_H

#include "mbed.h"
#include "bufferedserial.h"

namespace EnAccess {

class MbedSerial : public BufferedSerial
{
  public:
    MbedSerial(PinName tx = SERIAL_TX, PinName rx = SERIAL_RX);

    virtual bool open() override;
    virtual bool isOpen() override;
    virtual bool setSerialConfig(uint32_t baudRate, uint8_t dataBits) override;
    virtual void close() override;
    virtual const char* portName() const override;
    virtual uint16_t write(const char* data, uint16_t size) override;
    virtual void write(char data) override;
    virtual bool rawRead(uint8_t& data) override;
    virtual bool rawWrite(uint8_t data) override;

    void handleInterrupt();
    void handleInterrupt2(int i);

  private:
    // Private constructors to avoid copying
    MbedSerial(const MbedSerial&);
    MbedSerial& operator=(const MbedSerial&);

    mbed::RawSerial _rawSerial;
};
}

#endif
