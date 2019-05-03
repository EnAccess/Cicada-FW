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

#ifndef E_SIM800_H
#define E_SIM800_H

#include "cicada/bufferedserial.h"
#include "cicada/commdevices/simcommdevice.h"
#include <stdint.h>

namespace EnAccess {

/*!
 * Driver for the Simcom SIM800 series of 2G cellular modems.
 */

class Sim800CommDevice : public SimCommDevice
{
  public:
    /*!
     * \param serial Serial driver for the port the modem is connected to.
     */
    Sim800CommDevice(IBufferedSerial& serial);

    /*!
     * Actually performs communication with the modem.
     */
    virtual void run();

  private:
    enum ReplyState {
        okReply = 0,
        cifsr,
        cdnsgip,
        cipstart,
        ciprxget4,
        ciprxget2
    };

    enum SendState {
        notConnected,
        serialError,
        connecting,
        sendCiprxget,
        sendCipmux,
        sendCipsprt,
        sendCstt,
        sendCiicr,
        sendCifsr,
        sendDnsQuery,
        sendCipstart,
        finalizeConnect,
        connected,
        sendData,
        sendCiprxget4,
        sendCiprxget2,
        waitReceive,
        receiving,
        ipUnconnected,
        sendCipclose,
        sendCipshut,
        finalizeDisconnect
    };
};
}

#endif
