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

#include "circularbuffer.h"
#include "defines.h"
#include "ibufferedserial.h"
#include "linecircularbuffer.h"
#include "task.h"

namespace EnAccess {

/*!
 * Base class for serial devices. The buffering is handled by this
 * class, as well as reading/writing to/from the buffers. When adding
 * a new serial device, inherit from this class. You need to implement
 * the pure virtual functions from ISerial.
 */

class BufferedSerial : public IBufferedSerial
{
  public:
    BufferedSerial();

    virtual uint16_t bytesAvailable() const override;

    virtual uint16_t spaceAvailable() const override;

    virtual uint16_t read(char* data, uint16_t size) override;

    virtual char read() override;

    virtual uint16_t write(const char* data, uint16_t size) override;

    virtual void write(char data) override;

    virtual bool canReadLine() const override;

    virtual uint16_t readLine(char* data, uint16_t size) override;

    virtual void flushReceiveBuffers() override;

    virtual uint16_t bufferSize() override;

    /*!
     * Actually perform read/write to the underlying
     * raw serial device.
     */
    virtual void transferToAndFromBuffer() volatile;

  protected:
    volatile LineCircularBuffer<E_SERIAL_BUFFERSIZE> _readBuffer;
    volatile LineCircularBuffer<E_SERIAL_BUFFERSIZE> _writeBuffer;
};

/*!
 * \class BufferedSerialTask
 *
 * Turns BufferedSerial into a Task. Normally, performReadWrite() would
 * be called from an interrupt handler as soon as new data is available
 * from the serial hardware. On platforms where an interrupt is not available
 * and the serial hardware needs to be polled instead (like Unix termios),
 * this class can be used to do the polling in a Task and and it to
 * the Scheduler.
 */
class BufferedSerialTask : public BufferedSerial, public Task
{
  public:
    /*!
     * Calls BufferedSerial::performReadWrite().
     */
    inline void run()
    {
        transferToAndFromBuffer();
    }
};
}

#endif
