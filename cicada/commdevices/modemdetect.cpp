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

#include "cicada/commdevices/modemdetect.h"
#include <cstring>
#include <new>

using namespace Cicada;

ModemDetect::ModemDetect(IBufferedSerial& serial) :
    _serial(serial),
    _detectState(noState),
    _startDetection(false),
    _detectedModem(NULL)
{}

void ModemDetect::startDetection()
{
    _startDetection = true;
}

bool ModemDetect::modemDetected()
{
    return _detectState > modemDetectedState;
}

SimCommDevice* ModemDetect::getDetectedModem(
    uint8_t* readBuffer, uint8_t* writeBuffer, Size readBufferSize, Size writeBufferSize)
{
    switch (_detectState) {
    case detectedSim800:
        new (&_md.sim800)
            Sim800CommDevice(_serial, readBuffer, writeBuffer, readBufferSize, writeBufferSize);
        _detectedModem = &_md.sim800;
        break;
    case detectedSim7x00:
        new (&_md.sim7x00)
            Sim7x00CommDevice(_serial, readBuffer, writeBuffer, readBufferSize, writeBufferSize);
        _detectedModem = &_md.sim7x00;
        break;
    default:
        break;
    }

    return _detectedModem;
}

SimCommDevice* ModemDetect::getDetectedModem()
{
    return _detectedModem;
}

void ModemDetect::run()
{
    if (_detectedModem) {
        _detectedModem->run();
        return;
    }

    // If the serial device is net yet open, try to open it
    if (!_serial.isOpen()) {
        if (!_serial.open()) {
            _detectState = errorState;
        }
        return;
    }

    switch (_detectState) {
    case noState:
        if (_startDetection) {
            _detectState = beginState;
        }
        break;
    case beginState:
        _serial.flushReceiveBuffers();
        _serial.write((const uint8_t*)"AT+CGMM\r\n");
        _detectState = cgmmSent;
        setDelay(10);
        break;
    case cgmmSent:
        if (_serial.canReadLine()) {
            uint16_t bufSize = _serial.bytesAvailable() + 1;
            char readBuf[bufSize];
            bufSize = _serial.readLine((uint8_t*)readBuf, bufSize);
            readBuf[bufSize] = '\0';

            if (strncmp("SIMCOM_SIM800", readBuf, 13) == 0) {
                _detectState = detectedSim800;
            } else if (strncmp("SIMCOM_SIM7600", readBuf, 14) == 0) {
                _detectState = detectedSim7x00;
            }
        }
        break;
    default:
        break;
    }
}
