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

#ifndef E_CC1352P7_H
#define E_CC1352P7_H

#include "cicada/bufferedserial.h"
#include "cicada/commdevices/atcommdevice.h"
#include <stdint.h>

namespace Cicada {

/*!
 * Driver for the CC1352P7-1 WiSUN.
 */

class CC1352P7CommDevice : public ATCommDevice
{
  public:
    /*!
     * \param serial Serial driver for the port the modem is connected to.
     */
    CC1352P7CommDevice(
        IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize);

    CC1352P7CommDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
        Size readBufferSize, Size writeBufferSize);

    virtual void resetStates();

    /*!
     * Actually performs communication with the wifi module.
     * Each time, run() is called, it roughly perorms these steps:
     * -# If a reset is pending, reset device and internal states
     * -# Process any incoming data from the device, including:
     *   - Error messages
     *   - Incoming data
     *   - Connection close
     * -# If the device is ready to process new commands, run() will then
     * proceed to the connection state machine:
     * \startuml
     * top to bottom direction
     * [*] --> notConnected
     * notConnected --> notConnected
     * notConnected --> connecting : connection request via API
     * connecting --> sendCipstart
     * sendCipstart --> finalizeConnect
     * sendCipstart : ""AT+CIPSTART="UDP",<host>,<port>""
     * sendCipstart : ""AT+CIPSTART="TCP",<host>,<port>""
     * finalizeConnect --> connected
     * connected --> sendDataState : bytes in write buffer
     * connected --> sendCiprecvdata : incoming data pending & TCP
     * connected --> receiving : incoming data pending & UDP
     * connected --> sendCipclose : connection closed via API
     * connected --> connected
     * connected : bytes in write buffer: ""AT+CIPSEND=<numOfBytes>""
     * sendDataState --> connected
     * sendDataState : send data
     * sendCiprecvdata --> sendCipclose : connection closed via API
     * sendCiprecvdata --> waitReceive : bytesToReceive > 0
     * sendCiprecvdata --> connected : no bytes to receive
     * sendCiprecvdata : bytesToReceive > 0: ""AT+CIPRECVDATA=<numBytesReceive>""
     * waitReceive --> waitReceive : no data
     * waitReceive --> receiving : data available
     * receiving --> receiving : bytesToRead > 0
     * receiving --> sendCiprecvdata : bytesToReceive > 0
     * receiving --> connected : no bytes to receive/read
     * sendCipclose --> finalizeDisconnect
     * sendCipclose : ""AT+CIPCLOSE""
     * finalizeDisconnect --> notConnected
     * \enduml
     */
    virtual void run();

    enum ReplyState { okReply = 0, parseStateCiprecvdata, csq };

    enum SendState {
        notConnected,
        serialError,
        connecting,
        sendCipstart,
        finalizeConnect,
        connected,
        sendDataState,
        sendCiprecvdata,
        waitReceive,
        receiving,
        sendCipclose,
        finalizeDisconnect
    };

  protected:
    bool fillLineBuffer();
    bool sendCiprcvdata();
    bool parseCiprecvdata();
    bool parseCsq();
};
}

#endif
