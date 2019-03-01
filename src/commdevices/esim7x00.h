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

#include <stdint.h>
#include "ebufferedserial.h"
#include "eiipcommdevice.h"

class ESim7x00CommDevice : public EIIPCommDevice
{
public:
    ESim7x00CommDevice(EIBufferedSerial& serial);

    virtual void setHostPort(const char* host, uint16_t port);

    virtual void setApn(const char* apn);

    virtual bool connect();

    virtual void disconnect();

    virtual bool isConnected();

    virtual bool isIdle();

    virtual uint16_t bytesAvailable() const;

    virtual uint16_t spaceAvailable() const;

    virtual uint16_t read(uint8_t* data, uint16_t maxSize);

    virtual uint16_t write(const uint8_t* data, uint16_t size);
 
    virtual void run();

private:
    enum ReplyState
    {
        noReply,
        normalReply,
        expectConnect,
        netopen,
        cdnsgip,
        ciprxget4,
        ciprxget2
    };
    enum SendState
    {
        notConnected,
        serialError,
        dnsError,
        netopenError,
        connecting,
        sendCgdcont,
        sendAtd,
        sendPpp,
        sendCsocksetpn,
        sendCipmode,
        sendNetopen,
        sendCiprxget,
        sendDnsQuery,
        sendCipopen,
        finalizeConnect,
        connected,
        sendCipsend,
        sendCiprxget4,
        sendCiprxget2,
        waitReceive,
        receiving,
        ipUnconnected,
        sendNetclose,
        sendAth,
        finalizeDisconnect
    };

    EIBufferedSerial& _serial;
    CircularBuffer<uint8_t, E_NETWORK_BUFFERSIZE> _readBuffer;
    CircularBuffer<uint8_t, E_NETWORK_BUFFERSIZE> _writeBuffer;
    SendState _sendState;
    ReplyState _replyState;
    const char* _apn;
    const char* _host;
    char _ip[16];
    uint16_t _port;
    const char* _waitForReply;
    uint8_t _stateBooleans;
    uint16_t _bytesToWrite;
    uint16_t _bytesToReceive;
    uint16_t _bytesToRead;

    static const char* _okStr;
    static const char* _lineEndStr;
    static const char* _quoteEndStr;

    bool handleDisconnect(SendState nextState);
    bool handleConnect(SendState nextState);
};
