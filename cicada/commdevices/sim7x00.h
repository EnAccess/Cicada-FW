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

#ifndef E_SIM7x00_H
#define E_SIM7x00_H

#include "cicada/bufferedserial.h"
#include "cicada/commdevices/simcommdevice.h"
#include <stdint.h>

namespace Cicada {

/*!
 * Driver for the Simcom SIM7x00 series of 4G cellular modems.
 */

class Sim7x00CommDevice : public SimCommDevice
{
  public:
    /*!
     * \param serial Serial driver for the port the modem is connected to.
     */
    Sim7x00CommDevice(
        IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize);

    Sim7x00CommDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
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
     * connecting --> sendCgsockcont
     * connecting : ""ATE0""
     * sendCgsockcont --> sendCsocksetpn
     * sendCgsockcont : ""AT+CGSOCKCONT=1,"IP",<apn>""
     * sendCsocksetpn --> sendCipmode
     * sendCsocksetpn : ""AT+CSOCKSETPN=1""
     * sendCipmode --> sendNetopen
     * sendCipmode : ""AT+CIPMODE=0""
     * sendNetopen --> sendCiprxget
     * sendNetopen : ""AT+NETOPEN""
     * sendCiprxget --> sendDnsQuery
     * sendCiprxget : ""AT+CIPRXGET=1""
     * sendDnsQuery --> sendDnsQuery
     * sendDnsQuery --> sendCipopen : can send DNS query
     * sendDnsQuery : ""AT+CDNSGIP="<hostname>" ""
     * sendCipopen --> finalizeConnect
     * sendCipopen : ""AT+CIPOPEN=0,"UDP",,,""
     * sendCipopen : ""AT+CIPOPEN=0,"TCP",<ip>,""
     * finalizeConnect --> connected
     * connected --> sendData : bytes in write buffer
     * connected --> sendCiprxget4 : incoming data pending
     * connected --> sendNetclose : connection closed via API
     * connected --> connected
     * connected : bytes in write buffer: ""AT+CIPSEND=0,<numOfBytes>""
     * sendData --> connected
     * sendData : send data
     * sendCiprxget4 --> sendCiprxget2
     * sendCiprxget4 : ""AT+CIPRXGET=4,0""
     * sendCiprxget2 --> sendNetclose : connection closed via API
     * sendCiprxget2 --> waitReceive : bytesToReceive > 0
     * sendCiprxget2 --> connected
     * sendCiprxget2 --> ipUnconnected : connection close by peer
     * sendCiprxget2 : ""AT+CIPRXGET=2,0,<numBytesReceive>""
     * waitReceive --> waitReceive : no data
     * waitReceive --> receiving : data available
     * receiving --> receiving : bytesToRead > 0
     * receiving --> sendCiprxget2 : bytesToReceive > 0
     * receiving --> sendCiprxget4 : no bytes to receive/read
     * ipUnconnected --> sendNetclose : connection closed via API
     * ipUnconnected --> sendCipopen : connection close by peer
     * sendNetclose --> finalizeDisconnect
     * sendNetclose : ""AT+NETCLOSE=0""
     * finalizeDisconnect --> notConnected
     * \enduml
     */
    virtual void run();

  private:
    enum ReplyState {
        okReply = 0,
        csq,
        requestID,
        expectConnect,
        netopen,
        cdnsgip,
        cipopen,
        ciprxget4,
        ciprxget2
    };

    enum SendState {
        notConnected,
        serialError,
        dnsError,
        connecting,
        sendCgsockcont,
        sendCsocksetpn,
        sendCipmode,
        sendNetopen,
        sendCiprxget,
        sendDnsQuery,
        sendCipopen,
        finalizeConnect,
        connected,
        sendData,
        sendCiprxget4,
        sendCiprxget2,
        waitReceive,
        receiving,
        ipUnconnected,
        sendNetclose,
        finalizeDisconnect
    };

    const char* iccidCommand = "AT+CICCID";
};
}

#endif
