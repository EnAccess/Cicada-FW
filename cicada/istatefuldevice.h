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

#ifndef EISTATEFULDEVICE_H
#define EISTATEFULDEVICE_H

#include "cicada/icommdevice.h"
#include <cstdint>

namespace Cicada {
/*!
 * \class IStatefulDevice
 *
 * Base interface for stateful devices, which have a
 * connected/disconnected state.
 */

class IStatefulDevice : public ICommDevice
{
  public:
    virtual ~IStatefulDevice() {}

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
     * Hard resets internal states and flush buffers.
     */
    virtual void resetStates() = 0;
};

}

#endif
