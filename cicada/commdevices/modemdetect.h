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

#include "cicada/commdevices/atcommdevice.h"
#include "cicada/commdevices/espressif.h"
#include "cicada/commdevices/iipcommdevice.h"
#include "cicada/commdevices/sim7x00.h"
#include "cicada/commdevices/sim800.h"
#include "cicada/commdevices/cc1352p7.h"

namespace Cicada {

/*!
 * Autodetects the modem and returns the correct driver.
 */

class ModemDetect : public Task
{
  public:
    ModemDetect(IBufferedSerial& serial);

    /*!
     * Starts detection of the modem, which is then performed in run()
     * */
    void startDetection();

    /*!
     * Stays false until the modem is detected.
     * \return true when the modem has been detected
     */
    bool modemDetected();

    /*!
     * Constructs a modem driver for the detected device and returns a pointer to it.
     * The parameters are passed to the constructor of the actual driver.
     * The pointer to the driver stays valid as long as the ModemDetect class
     * is valid. The ModemDetect class must not go out of scope as long as the
     * driver is in use.
     * \return Pointer to the modem driver for the detected device.
     */
    ATCommDevice* getDetectedModem(
        uint8_t* readBuffer, uint8_t* writeBuffer, Size readBufferSize, Size writeBufferSize);

    /*!
     * \return Pointer to the modem driver created
     */
    ATCommDevice* getDetectedModem();

    /*!
     * Performs the actual modem detection task
     */
    virtual void run();

  private:
    union ModemDriver {
        ModemDriver() {}
        ~ModemDriver() {}
        Sim800CommDevice sim800;
        Sim7x00CommDevice sim7x00;
        EspressifDevice espressif;
        CC1352P7CommDevice cc1352p7;
    } _md;

    IBufferedSerial& _serial;

    enum DetectState {
        noState,
        beginState,
        errorState,
        cgmmSent,
        gmrSent,
        espressifWaitOk,
        modemDetectedState,
        detectedSim800,
        detectedSim7x00,
        detectedEspressif,
        detectedCC1352P7,
    } _detectState;

    bool _startDetection;
    ATCommDevice* _detectedModem;
};
}

#endif
