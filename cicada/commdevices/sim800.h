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

namespace Cicada {

/*!
 * Driver for the Simcom SIM800 series of 2G cellular modems.
 */

class Sim800CommDevice : public SimCommDevice
{
  public:
    /*!
     * \param serial Serial driver for the port the modem is connected to.
     */
    Sim800CommDevice(
        IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize);

    Sim800CommDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
        Size readBufferSize, Size writeBufferSize);

    /*!
     * Actually performs communication with the modem.
     * Each time, run() is called, it roughly perorms these steps:
     * -# If a reset is pending, reset modem and internal states
     * -# Process any incoming data from the device, including:
     *   - Error messages
     *   - Incoming data
     *   - Connection close
     * -# If the modem is ready to process new commands, run() will then
     * proceed to the connection state machine:
     * \startuml
     * top to bottom direction
     * [*] --> notConnected
     * notConnected --> notConnected
     * notConnected --> connecting : connection request via API
     * connecting --> sendCiprxget
     * connecting : ""ATE0""
     * sendCiprxget --> sendCipmux
     * sendCiprxget : ""AT+CIPRXGET=1""
     * sendCipmux --> sendCstt
     * sendCipmux : ""AT+CIPMUX=1""
     * sendCstt --> sendCiicr
     * sendCstt : ""AT+CSTT=<apn>""
     * sendCiicr --> sendCifsr
     * sendCiicr : AT+CIICR
     * sendCifsr --> sendCipshut : connection closed via API
     * sendCifsr --> sendDnsQuery
     * sendCifsr : AT+CIFSR
     * sendDnsQuery --> sendDnsQuery
     * sendDnsQuery --> sendCipstart : can send DNS query
     * sendDnsQuery : ""AT+CDNSGIP="<hostname>" ""
     * sendCipstart --> finalizeConnect
     * sendCipstart : ""AT+CIPSTART=0,"UDP",<ip>""
     * sendCipstart : ""AT+CIPSTART=0,"TCP",<ip>""
     * finalizeConnect --> connected
     * connected --> sendData : bytes in write buffer
     * connected --> sendCiprxget4 : incoming data pending
     * connected --> sendCipclose : connection closed via API
     * connected --> connected
     * connected : bytes in write buffer: ""AT+CIPSEND=0,<numOfBytes>""
     * sendData --> connected
     * sendData : send data
     * sendCiprxget4 --> sendCiprxget2
     * sendCiprxget4 : ""AT+CIPRXGET=4,0""
     * sendCiprxget2 --> sendCipclose : connection closed via API
     * sendCiprxget2 --> waitReceive : bytesToReceive > 0
     * sendCiprxget2 --> connected
     * sendCiprxget2 --> ipUnconnected : connection close by peer
     * sendCiprxget2 : ""AT+CIPRXGET=2,0,<numBytesReceive>""
     * waitReceive --> waitReceive : no data
     * waitReceive --> receiving : data available
     * receiving --> receiving : bytesToRead > 0
     * receiving --> sendCiprxget2 : bytesToReceive > 0
     * receiving --> sendCiprxget4 : no bytes to receive/read
     * ipUnconnected --> finalizeDisconnect : connection closed via API
     * ipUnconnected --> sendCipstart : connection close by peer
     * sendCipclose --> sendCipshut
     * sendCipclose : ""AT+CIPCLOSE=0""
     * sendCipshut --> finalizeDisconnect
     * sendCipshut : ""AT+CIPSHUT""
     * finalizeDisconnect --> notConnected
     * \enduml
     */
    virtual void run();

  private:
    enum ReplyState { okReply = 0, csq, requestID, cifsr, cdnsgip, cipstart, ciprxget4, ciprxget2 };

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

    const char* iccidCommand = "AT+CCID";
};
}

#endif
