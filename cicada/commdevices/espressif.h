/*
 * Cicada communication library
 * Copyright (C) 2021 Okrasolar
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

#ifndef ESPRESSIFDEVICE_H
#define ESPRESSIFDEVICE_H

#include "cicada/bufferedserial.h"
#include "cicada/commdevices/atcommdevice.h"
#include <stdint.h>

#define MACSTRING_MAX_LENGTH 18

namespace Cicada {

/*!
 * Driver for Wifi modules based on Espressif chip with Firware NonOS_AT v1.7 or v2.1
 * is required to work with the driver.
 */
class EspressifDevice : public ATCommDevice
{
  public:
    EspressifDevice(
        IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize);
    EspressifDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
        Size readBufferSize, Size writeBufferSize);
    virtual ~EspressifDevice() {}

    /*!
     * Resets the drivers states. The internal states will be initialized
     * with default values as they are right after construction. This method
     * should be called if the modem hardware is reset, and thus the driver state
     * is not consistent with the modem state anymore.
     */
    virtual void resetStates();

    /*!
     * Set's the wifi network SSID.
     * \param ssid The network SSID
     */
    virtual void setSSID(const char* ssid);

    /*!
     * Set's the wifi passwd.
     * \param ssid The wifi password
     */
    virtual void setPassword(const char* passwd);

    virtual bool connect();

    /*!
     * Request one of the MAC address from the module. It can then be
     * retreieved with getMACString();
     * Note: This method flushes the recieve buffer.
     */
    void requestMac();

    /*!
     * Actually returns the MAC address string requested before
     * with the requestMAC() method, or NULL if the string is
     * not yet available. The string is 0-terminated. When returned,
     * this points to the internal string buffer and stays unchanged until
     * another request*() method is called. The buffer is valid during the
     * lifetime of this class.
     * */
    char* getMacString();

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
     * connecting --> sendCwjap
     * connecting : ""ATE0""
     * sendCwjap --> sendCiprecvmode
     * sendCwjap --> finalizeDisconnect : connection close via API
     * sendCwjap : ""AT+CWJAP="<ssid>","<password>"""
     * sendCiprecvmode --> sendCipmode
     * sendCiprecvmode : ""AT+CIPRECVMODE=1""
     * sendCipmode --> sendCipstart
     * sendCipmode : ""AT+CIPMODE=0""
     * sendCipstart --> finalizeConnect
     * sendCipstart : ""AT+CIPSTART="UDP",<host>,<port>""
     * sendCipstart : ""AT+CIPSTART="TCP",<host>,<port>""
     * finalizeConnect --> connected
     * connected --> sendDataState : bytes in write buffer
     * connected --> sendCiprecvdata : incoming data pending & TCP
     * connected --> receiving : incoming data pending & UDP
     * connected --> sendCipclose : connection closed via API
     * connected --> sendCwqap : connection closed by peer
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
     * sendCipclose --> sendCwqap
     * sendCipclose : ""AT+CIPCLOSE""
     * sendCwqap --> finalizeDisconnect
     * sendCwqap : ""AT+CWQAP""
     * finalizeDisconnect --> notConnected
     * \enduml
     */
    virtual void run();

    enum ReplyState { okReply = 0, waitCiprecvdata, parseStateCiprecvdata, reqMac, rssi };

    enum SendState {
        notConnected,
        serialError,
        connecting,
        sendCwmode,
        sendCwjap,
        sendCiprecvmode,
        sendCipmux,
        sendCipmode,
        sendCipstart,
        finalizeConnect,
        connected,
        sendDataState,
        sendCiprecvdata,
        waitReceive,
        receiving,
        sendCipclose,
        sendCwqap,
        finalizeDisconnect
    };

  protected:
    bool fillLineBuffer();
    bool sendCiprcvdata();
    bool parseCiprecvdata();

    const char* _ssid;
    const char* _passwd;

    char _macStringBuffer[MACSTRING_MAX_LENGTH];
};
}

#endif
