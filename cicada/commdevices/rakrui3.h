/*
 * Cicada communication library
 * Copyright (C) 2022 Okrasolar
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

#ifndef RAKRUI3_H
#define RAKRUI3_H

#include "cicada/bufferedserial.h"
#include "cicada/commdevices/iipcommdevice.h"
#include <stdint.h>

#define LINE_MAX_LENGTH 60

namespace Cicada {

/*!
 * Driver for LoRaWAN modules based on RAKwireless Unified Interface V3 (RUI3).
 * Tested with RAK3172 module.
 * NOTE: This driver is preliminary and not well tested!
 */
class RakDevice : public IStatefulDevice, public Task
{
  public:
    RakDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize);
    RakDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
        Size readBufferSize, Size writeBufferSize);
    virtual ~RakDevice() {}

    /*!
     * Resets the drivers states. The internal states will be initialized
     * with default values as they are right after construction. This method
     * should be called if the modem hardware is reset, and thus the driver state
     * is not consistent with the modem state anymore.
     */
    virtual void resetStates();

    /*!
     * Set's the device identifier
     * \param eui The device EUI. Note: The eui is *not* copied, the string
     * needs to be valid during the lifetime of RakDevice object!
     */
    virtual void setDevEUI(const char* eui);

    /*!
     * Set's the application identifier
     * \param eui The application EUI. Note: The eui is *not* copied, the string
     * needs to be valid during the lifetime of RakDevice object!
     */
    virtual void setAppEUI(const char* eui);

    /*!
     * Set's the application key
     * \param eui The application key. Note: The key is *not* copied, the string
     * needs to be valid during the lifetime of RakDevice object!
     */
    virtual void setAppKey(const char* key);

    /*!
     * Sets the port to send data to. Can be after joining the network.
     * \param port The port to send data to.
     */
    void setPort(uint8_t port);

    /*!
     * Setup the device and join the LoRaWAN network
     */
    virtual bool connect();

    /*!
     * Un-join the LoRaWAN network
     */
    virtual void disconnect();

    /*!
     * LoRaWAN network could be sucessfully joined
     */
    virtual bool isConnected();

    virtual bool isIdle();
    virtual Size bytesAvailable() const;
    virtual Size spaceAvailable() const;
    virtual Size read(uint8_t* data, Size maxSize);
    virtual Size write(const uint8_t* data, Size size);
    virtual bool writeBufferProcessed() const;

    /*
    virtual Size read(uint8_t* data, Size maxSize);
    virtual Size write(const uint8_t* data, Size size);
    virtual bool writeBufferProcessed() const;
    */

    /*!
     * Actually performs communication with the wifi module.
     * Each time, run() is called, it roughly perorms these steps:
     * -# If a reset is pending, reset device and internal states
     * -# Process any incoming data from the device, including:
     *   - Error messages
     *   - Incoming data
     *   - Connection close
     */
    virtual void run();

    enum ReplyState { okReply = 0, dataRate, sendConfirm };

    enum SendState {
        notConnected,
        serialError,
        sendDevEUI,
        sendAppEUI,
        sendAppKey,
        sendClass,
        sendDR,
        join,
        finalizeJoin,
        joined,
        sendPacket,
        waitForSend,
        finalizeDisconnect
    };

  protected:
    bool fillLineBuffer();
    void sendCommand(const char* cmd);

    IBufferedSerial& _serial;
    char _lineBuffer[LINE_MAX_LENGTH + 1];
    uint8_t _lbFill = 0;

    CircularBuffer<uint8_t> _readBuffer;
    CircularBuffer<uint8_t> _writeBuffer;
    Size _bytesToResend;

    const char* _devEui = nullptr;
    const char* _appEui = nullptr;
    const char* _appKey = nullptr;
    char _portStr[4];

    uint8_t _stateBooleans;
    int8_t _sendState;
    int8_t _replyState;
    const char* _waitForReply;

    static const char* _okStr;
    static const char* _lineEndStr;
    static const char* _quoteEndStr;

    uint8_t _currentPacketSize = 0;
    static const uint8_t _packetSizes[14];
};
}

#endif
