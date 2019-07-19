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

#include "cicada/circularbuffer.h"
#include "cicada/defines.h"
#include "cicada/ibufferedserial.h"
#include "cicada/linecircularbuffer.h"
#include "cicada/task.h"

namespace Cicada {

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

    virtual Size bytesAvailable() const override;

    virtual Size spaceAvailable() const override;

    virtual Size read(uint8_t* data, Size size) override;

    virtual uint8_t read() override;

    virtual Size write(const uint8_t* data, Size size) override;

    virtual Size write(const uint8_t* data) override;

    virtual void write(uint8_t data) override;

    virtual bool canReadLine() const override;

    /*!
     * Reads a line, or more precisely, all characters before the
     * next '\n'. The '\n' is not included in the result, but
     * instead replaced with '\0'.
     *
     * This is meant a s convenience function to easily retrieve full
     * lines in character data, for example a reply after an AT command.
     * It should not be used for binary data.
     */
    virtual Size readLine(uint8_t* data, Size size) override;

    virtual void flushReceiveBuffers() override;

    virtual Size bufferSize() override;

    /*!
     * Actually perform read/write to the underlying
     * raw serial device.
     */
    void transferToAndFromBuffer();

  protected:
    LineCircularBuffer<E_SERIAL_BUFFERSIZE> _readBuffer;
    LineCircularBuffer<E_SERIAL_BUFFERSIZE> _writeBuffer;

  private:
    void copyToBuffer(uint8_t data);
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
