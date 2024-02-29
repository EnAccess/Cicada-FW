/*
 * Example code for IP communication
 */

#include "cicada/commdevices/modemdetect.h"
#include "cicada/platform/linux/unixserial.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Cicada;

class IPCommTask : public Task
{
  public:
    IPCommTask(ModemDetect& detector) : m_detector(detector), m_commDev(NULL) {}

    virtual void run()
    {
        SimCommDevice* simCommDev;
        EspressifDevice* espressifDev;

        E_BEGIN_TASK

        m_detector.startDetection();

        E_REENTER_COND(m_detector.modemDetected());
        m_commDev = m_detector.getDetectedModem(
            _commReadBuffer, _commWriteBuffer, _commBufferSize, _commBufferSize);

        simCommDev = dynamic_cast<SimCommDevice*>(m_commDev);
        if (simCommDev) {
            simCommDev->setApn("iot-eu.aer.net");
        }
        espressifDev = dynamic_cast<EspressifDevice*>(m_commDev);
        if (espressifDev) {
            espressifDev->setSSID("your_ssid");
            espressifDev->setPassword("your_pass");
        }

        m_commDev->setHostPort("wttr.in", 80);
        m_commDev->connect();

        E_REENTER_COND(m_commDev->isConnected());

        printf("*** Connected! ***\n");

        {
            const char str[] = "GET / HTTP/1.1\r\n"
                               "Host: wttr.in\r\n"
                               "User-Agent: curl\r\n"
                               "Connection: close\r\n\r\n";
            m_commDev->write((uint8_t*)str, sizeof(str) - 1);
        }

        E_REENTER_COND(m_commDev->bytesAvailable());

        while (m_commDev->isConnected()) {
            if (m_commDev->bytesAvailable()) {
                char buf[41];
                uint16_t bytesRead = m_commDev->read((uint8_t*)buf, 40);
                buf[bytesRead] = '\0';
                printf("%s", buf);
            } else {
                E_REENTER_COND(m_commDev->bytesAvailable() || !m_commDev->isConnected());
            }
        }

        m_commDev->disconnect();
        E_REENTER_COND(m_commDev->isIdle());

        printf("*** Disconnected ***\n");

        E_END_TASK
    }

  private:
    static const uint16_t _commBufferSize = 1200;
    uint8_t _commReadBuffer[_commBufferSize];
    uint8_t _commWriteBuffer[_commBufferSize];

    ModemDetect& m_detector;
    IPCommDevice* m_commDev;
};

int main(int argc, char* argv[])
{
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, serialBufferSize);

    ModemDetect detector(serial);

    IPCommTask task(detector);

    Task* taskList[] = { &task, &detector, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);
    s.start();
}
