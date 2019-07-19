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

#ifndef EICOMMDEVICE_H
#define EICOMMDEVICE_H

#include "cicada/types.h"
#include <cstdint>

namespace Cicada {

/*!
 * \class ICommDevice
 *
 * Base Interface for communication devices, like UARTs, 4G/GSM cellular modems
 * or WiFi modules. Drivers for these devices must implement this
 * interface. All methods in this class have to be **non-blocking**.
 * The actual processing and communication with the hardware is done
 * in the driver's run() method, which will be called in regular intervals
 * by the Scheduler.
 */

class ICommDevice
{
  public:
    virtual ~ICommDevice() { }

    /*!
     * Number of bytes available for reading.
     * \return Number of bytes available for reading
     */
    virtual Size bytesAvailable() const = 0;

    /*!
     * Number of bytes which can be written immediately,
     * with out blocking. This is usually the size of space
     * available in ther underlying buffer.
     * \return Number of bytes available for writing
     */
    virtual Size spaceAvailable() const = 0;

    /*!
     * Reads data from the device. This method is non-blocking and only
     * copies data already in the receive buffer. If no data is
     * currently available, the method does not copy anything and will
     * return 0. Filling up the read buffer is done in an interrupt or
     * the device driver's run() function.
     * \param data Buffer to store data. Must be large enough to store
     * maxSize bytes.
     * \param maxSize Maximum number of bytes to store into data. The actual
     * number of bytes stored can be smaller than maxSize.
     * \return Number of bytes actually copied to data
     */
    virtual Size read(uint8_t* data, Size maxSize) = 0;

    /*!
     * Writes data to the device. This method is non-blocking and
     * only copies data to the internal send buffer. The data will be
     * transmitted later from the modem driver's run() function. If there
     * is not enough space in the send buffer, this function will copy only
     * as many data as it has space available, and returns the number
     * of bytes acutally copied.
     * \param data Buffer with data written to the device
     * \param size Number of bytes to write
     * \return Actual number of bytes copied into the transmit buffer.
     */
    virtual Size write(const uint8_t* data, Size size) = 0;
};

}

#endif
