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

#ifndef MODEMDETECT_H
#define MODEMDETECT_H

#include "cicada/commdevices/iipcommdevice.h"
#include "cicada/commdevices/sim7x00.h"
#include "cicada/commdevices/sim800.h"

namespace Cicada {

class ModemDetect : public Task
{
  public:
    ModemDetect(IBufferedSerial& serial);

    bool modemDetected();
    SimCommDevice* getDetectedModem(
        uint8_t* readBuffer, uint8_t* writeBuffer, Size readBufferSize, Size writeBufferSize);
    SimCommDevice* getDetectedModem();

    virtual void run();

  private:
    union ModemDriver {
        ModemDriver() {}
        ~ModemDriver() {}
        Sim800CommDevice sim800;
        Sim7x00CommDevice sim7x00;
    } _md;

    IBufferedSerial& _serial;

    enum DetectState {
        beginState,
        errorState,
        cgmmSent,
        modemDetectedState,
        detectedSim800,
        detectedSim7x00
    } _detectState;

    SimCommDevice* _detectedModem;
};
}

#endif
