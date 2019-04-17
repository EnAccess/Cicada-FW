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

#ifndef SIMCOMMDEVICE_H
#define SIMCOMMDEVICE_H

#include "ipcommdevice.h"

#define LINE_MAX_LENGTH 60
#define CONNECT_PENDING (1 << 0)
#define RESET_PENDING (1 << 1)
#define DATA_PENDING (1 << 2)
#define DISCONNECT_PENDING (1 << 3)
#define IP_CONNECTED (1 << 4)
#define LINE_READ (1 << 5)

namespace EnAccess {

class SimCommDevice : public IPCommDevice
{
  public:
    SimCommDevice(IBufferedSerial& serial);
    virtual ~SimCommDevice() { }

    /*!
     * Set's the cellular network APN.
     * \param apn The network APN
     */
    virtual void setApn(const char* apn);

    virtual bool connect();

  protected:
    bool fillLineBuffer();
    void logStates(int8_t sendState, int8_t replyState);
    void processStandardReply();
    bool parseDnsReply();
    bool parseCiprxget4();
    bool parseCiprxget2();
    void checkConnectionState(const char* closeVariant);
    void flushReadBuffer();
    bool handleDisconnect(int8_t nextState);
    bool handleConnect(int8_t nextState);
    bool sendDnsQuery();
    void sendCipstart(const char* openVariant);
    bool prepareSending();
    void sendData();
    bool sendCiprxget2();
    bool receive();
    void sendCommand(const char* cmd);

    IBufferedSerial& _serial;
    const char* _apn;

    char _lineBuffer[LINE_MAX_LENGTH + 1];
    uint8_t _lbFill;

    char _ip[16];

    int8_t _sendState;
    int8_t _replyState;
    uint16_t _bytesToWrite;
    uint16_t _bytesToReceive;
    uint16_t _bytesToRead;

    static const char* _okStr;
    static const char* _lineEndStr;
    static const char* _quoteEndStr;
};

}

#endif
