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

#include "cicada/commdevices/atcommdevice.h"

#define IDSTRING_MAX_LENGTH 24

namespace Cicada {

class SimCommDevice : public ATCommDevice
{
  public:
    SimCommDevice(
        IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize);
    SimCommDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
        Size readBufferSize, Size writeBufferSize);
    virtual ~SimCommDevice() {}

    /*!
     * Resets the drivers states. The internal states will be initialized
     * with default values as they are right after construction. This method
     * should be called if the modem hardware is reset, and thus the driver state
     * is not consistent with the modem state anymore.
     */
    virtual void resetStates();

    /*!
     * Set's the cellular network APN.
     * \param apn The network APN
     */
    virtual void setApn(const char* apn);

    virtual bool connect();

    /*!
     * Locks the serial device for the modem driver, so that it can be used by
     * the serialWrite() / serialRead() methods.
     *
     * \returns True if the lock was acquired sucessfully.
     */
    virtual bool serialLock();

    /*!
     * Unlock the serial device for the modem driver.
     */
    virtual void serialUnlock();

    /*!
     * The purpose of this function is to send custom AT commands to the modem.
     * To do so, first lock the serial device for the modem driver by calling
     * serialLock(). When serialLock() returns true, no data will be sent or
     * recieved by the driver, so it does not interfere with the custom
     * user data. The function is non-blocking. Data are copied to the write
     * buffer and actually sent later.
     *
     * \param data zero-Terminated string of data to send to the modem
     * \return Actual number of bytes written
     */
    virtual Size serialWrite(char* data);

    /*!
     * After acquiring the lock with serialLock(), this function may be used
     * to read data directly from the modem. It's purpose is to read a reply
     * after sending a command with serialWrite(). This function is non-blocking,
     * it returns immediately even if there are no data available for reading.
     *
     * \param data Buffer to write data into.
     * \maxSize maximum space available in the buffer.
     * \return Number of bytes actually copied to data.
     */
    virtual Size serialRead(char* data, Size maxSize);

    /*!
     * Request the RSSI (signal strength) from the modem. It can then be
     * retreieved with getRSSI();
     */
    void requestRSSI();

    /*!
     * Actually get the value for RSSI (signal strength), which has
     * been requested by requestRSSI(). If the signal strength has been
     * requested but not yet been retrieved, the returned value will
     * be UINT8_MAX.
     *
     * \return RSSI value as returned by the modem, or UINT8_MAX if
     * a request is still pending.
     */
    uint8_t getRSSI();

    enum RequestIDType { noRequest, manufacturer, model, imei, imsi, iccid };

    /*!
     * Request one of the identifications from the modem. It can then be
     * retreieved with getIDString();
     * Note: This method flushes the recieve buffer.
     */
    void requestIDString(RequestIDType type);

    /*!
     * Actually returns the identification strings requested before
     * with one of the request*() methods, or NULL if the string is
     * not yet available. The string is 0-terminated. When returned,
     * this points to the internal string buffer and stays unchanged until
     * another request*() method is called. The buffer is valid during the
     * lifetime of this class.
     * */
    char* getIDString();

  protected:
    bool fillLineBuffer();
    bool parseDnsReply();
    bool parseCiprxget4();
    bool parseCiprxget2();
    bool parseCsq();
    bool parseIDReply();
    void checkConnectionState(const char* closeVariant);
    bool sendDnsQuery();
    void sendCipstart(const char* openVariant);
    bool sendCiprxget2();
    bool sendIDRequest(const char* modemSpecificICCIDCommand);

    const char* _apn;

    char _ip[16];

    char _idStringBuffer[IDSTRING_MAX_LENGTH];

    uint8_t _rssi;

    uint16_t _modemMaxReceiveSize;
};
}

#endif
