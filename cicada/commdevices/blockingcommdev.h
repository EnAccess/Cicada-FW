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

#ifndef EBLOCKINGCOMMDEV_H
#define EBLOCKINGCOMMDEV_H

#include "cicada/commdevices/icommdevice.h"
#include "cicada/defines.h"
#include <cstddef>

namespace Cicada {

/*!
 *\class BlockingCommDevice
 *
 * A blocking wrapper around the non-blocking ICommDevice.
 * This class is especially useful for the Eclips Paho MQTTClient.
 * It can be directly passed to the MQTTClient as it's
 * Network class.
 */
class BlockingCommDevice
{
  public:
    /*!
     * \param dev CommDevice to be used in blocking mode
     * \param tickFunction function which delivers system tick time
     * \param yieldFunction function to be called in reguler intrvals
     * \param yieldUserData data which can be passed to the yield function
     * while waiting for buffers. This is usually the operating system's
     * yield() or a user defined function to process other tasks.
     */
    BlockingCommDevice(ICommDevice& dev, E_TICK_TYPE (*tickFunction)(void),
        void (*yieldFunction)(void*), void* yieldUserData = NULL);

    /*!
     * Blocking read.
     */
    int read(unsigned char* buffer, int len, int timeout);

    /*!
     * Blocking write.
     */
    int write(unsigned char* buffer, int len, int timeout);

  private:
    ICommDevice& _commDev;
    E_TICK_TYPE (*_tickFunction)(void);
    void (*_yieldFunction)(void*);
    void* _yieldUserData;
};
}

#endif
