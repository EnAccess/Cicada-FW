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

#include "cicada/commdevices/simcommdevice.h"
#include "printf.h"
#include <cinttypes>
#include <cstddef>
#include <cstdlib>
#include <cstring>

using namespace Cicada;

SimCommDevice::SimCommDevice(
    IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer, Size bufferSize) :
    ATCommDevice(serial, readBuffer, writeBuffer, bufferSize), _apn(NULL)
{
    resetStates();
}

SimCommDevice::SimCommDevice(IBufferedSerial& serial, uint8_t* readBuffer, uint8_t* writeBuffer,
    Size readBufferSize, Size writeBufferSize) :
    ATCommDevice(serial, readBuffer, writeBuffer, readBufferSize, writeBufferSize), _apn(NULL)
{
    resetStates();
}

void SimCommDevice::resetStates()
{
    _serial.flushReceiveBuffers();
    _readBuffer.flush();
    _writeBuffer.flush();
    _lbFill = 0;
    _sendState = 0;
    _replyState = 0;
    _connectState = IPCommDevice::notConnected;
    _bytesToWrite = 0;
    _bytesToReceive = 0;
    _bytesToRead = 0;
    _waitForReply = NULL;
    _stateBooleans = LINE_READ;
    _rssi = 99;
    _idStringBuffer[0] = '\0';
    _idStringBuffer[1] = noRequest;
}

bool SimCommDevice::fillLineBuffer()
{
    // Buffer reply from modem in line buffer
    // Returns true when enough data to be parsed is available.
    if (_stateBooleans & LINE_READ) {
        while (_serial.bytesAvailable()) {
            char c = _serial.read();
            _lineBuffer[_lbFill++] = c;
            if (c == '\n' || c == '>' || _lbFill == LINE_MAX_LENGTH) {
                _lineBuffer[_lbFill] = '\0';
                _lbFill = 0;
                return true;
            }
        }
    }
    return false;
}

void SimCommDevice::setApn(const char* apn)
{
    _apn = apn;
}

bool SimCommDevice::connect()
{
    if (_apn == NULL)
        return false;

    return IPCommDevice::connect();
}

bool SimCommDevice::serialLock()
{
    if (_waitForReply || _replyState != 0)
        return false;

    _stateBooleans |= SERIAL_LOCKED;
    return true;
}

void SimCommDevice::serialUnlock()
{
    _stateBooleans &= ~SERIAL_LOCKED;
}

bool SimCommDevice::parseDnsReply()
{
    if (strncmp(_lineBuffer, "+CDNSGIP: 1", 11) == 0) {
        char* tmpStr;
        uint8_t i = 0, q = 0;

        // Validate DNS reply string
        while (_lineBuffer[i]) {
            if (_lineBuffer[i++] == '\"')
                q++;
        }
        if (q < 4 || q > 10) {
            // Error in input string
            _connectState = dnsError;
            return false;
        }
        i = 0, q = 0;

        // Parse IP address
        while (q < 3)
            if (_lineBuffer[i++] == '\"')
                q++;
        tmpStr = _lineBuffer + i;
        while (_lineBuffer[i]) {
            if (_lineBuffer[i] == '\"')
                _lineBuffer[i] = '\0';
            i++;
        }
        strcpy(_ip, tmpStr);
        return true;
    } else if (strncmp(_lineBuffer, "+CDNSGIP: 0", 11) == 0) {
        _stateBooleans |= RESET_PENDING;
    }

    return false;
}

bool SimCommDevice::parseCiprxget4()
{
    if (strncmp(_lineBuffer, "+CIPRXGET: 4,0,", 15) == 0) {
        int bytesToReceive = strtol(_lineBuffer + 15, NULL, 10);
        _bytesToReceive += bytesToReceive;
        return true;
    }
    return false;
}

bool SimCommDevice::parseCiprxget2()
{
    if (strncmp(_lineBuffer, "+CIPRXGET: 2,0,", 15) == 0) {
        int bytesToReceive = strtol(_lineBuffer + 15, NULL, 10);
        _bytesToReceive -= bytesToReceive;
        _bytesToRead += bytesToReceive;
        _stateBooleans &= ~LINE_READ;
        return true;
    }
    return false;
}

bool SimCommDevice::parseCsq()
{
    if (strncmp(_lineBuffer, "+CSQ: ", 6) == 0) {
        unsigned int rssi = strtol(_lineBuffer + 6, NULL, 10);
        // Convert raw rssi to dBm
        if (rssi >= 0 && rssi <= 31) {
            // Conversion according to 3GPP TS 27.007
            _rssi = -113 + rssi * 2;
        } else if (rssi > 99 && rssi <= 199) {
            // Conversion for Simcom SIM7500/SIM7600 devices
            _rssi = -116 + (rssi - 100);
        } else {
            _rssi = 0;
        }
        return true;
    }
    return false;
}

bool SimCommDevice::parseIDReply()
{
    // Avoid parsing command echo in case it's turned on
    if (strncmp(_lineBuffer, "AT", 2) == 0 || _lineBuffer[0] == '\r') {
        return false;
    }

    char* src = _lineBuffer;

    // For Sim7x00: "AT+CICCID" replys with "+ICCID: <ICCID>"
    if (strncmp(_lineBuffer, "+ICCID: ", 8) == 0) {
        src = _lineBuffer + 8;
        // Avoid handling other messages from the modem here
    } else if (strncmp(_lineBuffer, "+", 1) == 0) {
        return false;
    }

    // Validation of reply
    RequestIDType type = (RequestIDType)_idStringBuffer[2];
    bool replyValid = false;

    switch (type) {
    case iccid: {
        // Count number of digits in reply
        int nDigits = 0;
        while (src[nDigits] >= '0' && src[nDigits] <= '9') {
            nDigits++;
        }
        src[nDigits] = '\r';

        // Calculate checksum with Luhn's algorithm
        int nSum = 0;
        bool isSecond = false;
        for (int i = nDigits - 1; i >= 0; i--) {
            int d = src[i] - '0';
            if (isSecond == true)
                d = d * 2;
            nSum += d / 10;
            nSum += d % 10;
            isSecond = !isSecond;
        }
        replyValid
            = nDigits >= 18 && nDigits <= 22 && nSum % 10 == 0 && src[0] == '8' && src[1] == '9';

        break;
    }

    default:
        // TODO: Other replies are not yet validated
        replyValid = true;
        break;
    }

    if (replyValid) {
        int copiedChars = 0;
        while (*src != '\r' && copiedChars < IDSTRING_MAX_LENGTH - 1) {
            _idStringBuffer[copiedChars++] = *src++;
        }
        _idStringBuffer[copiedChars] = '\0';

        return true;
    }

    return false;
}

bool SimCommDevice::sendDnsQuery()
{
    if (_serial.spaceAvailable() < strlen(_host) + 20)
        return false;

    _serial.write((const uint8_t*)"AT+CDNSGIP=\"");
    _serial.write((const uint8_t*)_host);
    _serial.write((const uint8_t*)_quoteEndStr);

    return true;
}

bool SimCommDevice::sendCiprxget2()
{
    if (_serial.readBufferSize() - _serial.bytesAvailable() > 8
        && _readBuffer.spaceAvailable() > 0) {
        Size bytesToReceive = _serial.readBufferSize() - _serial.bytesAvailable() - 8;
        if (bytesToReceive > _bytesToReceive)
            bytesToReceive = _bytesToReceive;
        if (bytesToReceive > _readBuffer.spaceAvailable())
            bytesToReceive = _readBuffer.spaceAvailable();
        if (bytesToReceive > _modemMaxReceiveSize)
            bytesToReceive = _modemMaxReceiveSize;

        const char str[] = "AT+CIPRXGET=2,0,";
        char sizeStr[6];
        sprintf(sizeStr, "%u", (unsigned int)bytesToReceive);
        _serial.write((const uint8_t*)str, sizeof(str) - 1);
        _serial.write((const uint8_t*)sizeStr);
        _serial.write((const uint8_t*)_lineEndStr);
        return true;
    } else {
        return false;
    }
}

void SimCommDevice::checkConnectionState(const char* closeVariant)
{
    if (strncmp(_lineBuffer, "+CIPRXGET: 1,0", 14) == 0) {
        _stateBooleans |= DATA_PENDING;
    } else if (strncmp(_lineBuffer, closeVariant, strlen(closeVariant)) == 0) {
        _waitForReply = NULL;
        _stateBooleans &= ~IP_CONNECTED;
    }
}

bool SimCommDevice::sendIDRequest(const char* modemSpecificICCIDCommand)
{
    if (_idStringBuffer[1] != noRequest && _idStringBuffer[0] == 0 && _stateBooleans & LINE_READ) {
        RequestIDType type = (RequestIDType)_idStringBuffer[1];
        _idStringBuffer[1] = noRequest;

        switch (type) {
        // ID Requests according to 3GPP TS 27.007
        case manufacturer:
            sendCommand("AT+CGMI");
            return true;
        case model:
            sendCommand("AT+CGMM");
            return true;
        case imei:
            sendCommand("AT+CGSN");
            return true;
        case imsi:
            sendCommand("AT+CIMI");
            return true;

        // Modem specific ID requests
        case iccid:
            sendCommand(modemSpecificICCIDCommand);
            return true;
        default:
            break;
        }
    }
    return false;
}

void SimCommDevice::requestIDString(RequestIDType type)
{
    _idStringBuffer[0] = '\0';
    _idStringBuffer[1] = type;
    _idStringBuffer[2] = type;
}

char* SimCommDevice::getIDString()
{
    if (_waitForReply == NULL && _idStringBuffer[0] != '\0') {
        return _idStringBuffer;
    } else {
        return NULL;
    }
}
