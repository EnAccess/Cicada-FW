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

#ifndef ATCOMMDEVICE_H
#define ATCOMMDEVICE_H

#include "cicada/bufferedserial.h"
#include "cicada/commdevices/ipcommdevice.h"
#include <stdint.h>

#define LINE_MAX_LENGTH 60

namespace Cicada {

/*!
 * Driver for Wifi modules based on Espressif Esp8266 chip. Firware NonOS_AT v1.7 or later
 * is required to work with the driver.
 */
class ATCommDevice : public IPCommDevice
{
  public:
    ATCommDevice(
        IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize);
    ATCommDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
        Size readBufferSize, Size writeBufferSize);
    virtual ~ATCommDevice() {}

    enum ReplyState {
        okReply = 0,
        waitCiprecvdata,
        parseStateCiprecvdata
    };

    enum SendState {
        notConnected,
        serialError,
        connecting,
        sendCwmode,
        sendCwjap,
        sendCiprecvmode,
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
    void logStates(int8_t sendState, int8_t replyState);
    bool handleDisconnect(int8_t nextState);
    bool handleConnect(int8_t nextState);
    void sendCommand(const char* cmd);
    bool prepareSending();
    void sendData();
    void flushReadBuffer();
    bool receive();

    IBufferedSerial& _serial;

    char _lineBuffer[LINE_MAX_LENGTH + 1];
    uint8_t _lbFill;

    int8_t _sendState;
    int8_t _replyState;
    Size _bytesToWrite;
    Size _bytesToReceive;
    Size _bytesToRead;

    static const char* _okStr;
    static const char* _lineEndStr;
    static const char* _quoteEndStr;
};
}

#endif
