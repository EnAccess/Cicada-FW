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

#ifndef EISERIAL_H
#define EISERIAL_H

namespace Cicada {

/*!
 * \class ISerial
 *
 * Interface for raw (unbuffered) access to the serial hardware. rawRead() and
 * rawWrite() shall be implemented to read/write from the according hardware
 * register. The IBufferedSerial / BufferedSerial classes access those
 * method to perform read/write on a higher level.
 */
class ISerial
{
  public:
    virtual ~ISerial() {}

    /*!
     * Opens the serial device.
     * \return true if port was opened sucessfully, false otherwise
     */
    virtual bool open() = 0;

    /*!
     * Returns the opening state of the device.
     * \return true if the device is open, false otherwise.
     */
    virtual bool isOpen() = 0;

    /*!
     * Sets the serial device parameters.
     * \param baudRate One of the valid serial baud rates
     * \param dataBits Bit depth, usually 5, 6, 7, or 8
     * \return true if configuration could be applied sucessfully, false otherwise
     */
    virtual bool setSerialConfig(uint32_t baudRate, uint8_t dataBits) = 0;

    /*!
     * Closes the device
     */
    virtual void close() = 0;

    /*!
     * Name of the serial port used.
     * \return Port name or NULL if not available
     */
    virtual const char* portName() const = 0;

  protected:
    /*!
     * Reads one byte of data from the device. If there is no
     * data to read, the returned result is undefined.
     * \param data Place to store data read
     * \return true if read was successful, false otherwise
     */
    virtual bool rawRead(uint8_t& data) = 0;

    /*!
     * Writes one byte of data to the device.
     * \param data byte to be written
     * \return true if write was successful, false otherwise
     */
    virtual bool rawWrite(uint8_t data) = 0;

    /*!
     * Starts transmission. This would usually set the according
     * interrupt bits and/or install callbacks and interrupt handlers.
     */
    virtual void startTransmit() = 0;
};

}

#endif
