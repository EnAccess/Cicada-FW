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

#include "task.h"

namespace EnAccess {

class ICommDevice : public Task
{
public:
    virtual ~ICommDevice() { }

    /*!
     * Connects the device to the other side of the communication channel
     */
    virtual bool connect() = 0;

    /*!
     * Disconnects the device
     */
    virtual void disconnect() = 0;

    /*!
     * \return true if the device is fully connected, false otherwise.
     * Fully connected means it's able to perform read/write operations
     * to it's connected host.
     */
    virtual bool isConnected() = 0;

    /*!
     * \return true if the device is in idle state, false otherwise.
     * Idle means all services are disconnected and it's able to
     * establish a new connection.
     */
    virtual bool isIdle() = 0;

    /*!
     * Number of bytes available for reading.
     * \return Number of bytes available for reading
     */
    virtual uint16_t bytesAvailable() const = 0;

    /*!
     * Number of bytes which can be written immediately,
     * with out blocking. This is usually the size of space
     * available in ther underlying buffer.
     * \return Number of bytes available for writing
     */
    virtual uint16_t spaceAvailable() const = 0;

    /*!
     * Reads data from the device.
     * \param data Buffer to store data. Must be large enough to store
     * maxSize bytes.
     * \param maxSize Maximum number of bytes to store into data. The actual
     * number of bytes stored can be smaller than maxSize.
     * \return Number of bytes actually copied to data
     */
    virtual uint16_t read(uint8_t* data, uint16_t maxSize) = 0;

    /*!
     * Writes data to the device.
     * \param data Buffer with data written to the device
     * \param size Number of bytes to write
     * \return Actual number of bytes written
     */
    virtual uint16_t write(const uint8_t* data, uint16_t size) = 0;
};

}
    
#endif
