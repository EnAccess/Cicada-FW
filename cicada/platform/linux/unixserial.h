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

#include "cicada/bufferedserial.h"
#include <stdint.h>
#include <termios.h>

namespace Cicada {

/*!
 * \class UnixSerial
 *
 * Serial driver for Unix tty serial devices, runs on Linux
 * and OS X.
 *
 * This class is meant for testing porpose only. It can be used
 * to connect to serial devices from a normal PC without the need
 * for an actual microcontroller hardware.
 *
 * It's implementation is very inefficient, reading/writing
 * one byte only at each system call. Not meant for production.
 */

class UnixSerial : public BufferedSerialTask
{
  public:
    /*!
     * Construct a new UnixSerial object with the given serial port,
     * for example /dev/ttyUSB0. The String is not copied and must
     * be valid for the object's lifetime.
     * \param port Name of the serial port
     */
    UnixSerial(
        char* readBuffer, char* writeBuffer, Size bufferSize, const char* port = "/dev/ttyUSB0");
    UnixSerial(char* readBuffer, char* writeBuffer, Size readBufferSize, Size writeBufferSize,
        const char* port = "/dev/ttyUSB0");

    virtual bool open();

    inline virtual bool isOpen()
    {
        return _isOpen;
    }

    virtual bool setSerialConfig(uint32_t baudRate, uint8_t dataBits);

    virtual void close();

    inline const char* portName() const
    {
        return _port;
    }

    virtual bool writeBufferProcessed() const;

  protected:
    virtual bool rawRead(uint8_t& data);

    virtual bool rawWrite(uint8_t data);

    virtual void startTransmit() {}

  private:
    bool _isOpen;
    const char* _port;
    int _fd;
    speed_t _speed;
    tcflag_t _dataBits;
};
}

#endif
