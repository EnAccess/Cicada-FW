/*
 * Example code to get a timestamp via NTP.
 * This demonstrates the usage of UDP.
 */

#include "cicada/commdevices/modemdetect.h"
#include "cicada/platform/linux/unixserial.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Cicada;

class NTPTask : public Task
{
  public:
    NTPTask(ModemDetect& detector) :
        m_detector(detector),
        m_commDev(NULL)
        {}

    virtual void run()
    {
        SimCommDevice* simCommDev;
        Esp8266Device* esp8266Dev;

        E_BEGIN_TASK

        m_detector.startDetection();

        E_REENTER_COND(m_detector.modemDetected());
        m_commDev = m_detector.getDetectedModem(_commReadBuffer, _commWriteBuffer, _commBufferSize, _commBufferSize);

        simCommDev = dynamic_cast<SimCommDevice*>(m_commDev);
        if (simCommDev) {
            simCommDev->setApn("iot-eu.aer.net");
        }
        esp8266Dev = dynamic_cast<Esp8266Device*>(m_commDev);
        if (esp8266Dev) {
            esp8266Dev->setSSID("SSID");
            esp8266Dev->setPassword("password");
        }

        memset(m_ntpPacket, 0, sizeof(m_ntpPacket));

        m_commDev->setHostPort("pool.ntp.org", 123, IIPCommDevice::UDP);
        m_commDev->connect();

        E_REENTER_COND(m_commDev->isConnected());

        printf("*** Connected! ***\n");

        m_ntpPacket[0] = 0x1b;
        m_commDev->write((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        E_REENTER_COND(m_commDev->bytesAvailable());

        m_commDev->read((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        printf("Seconds since the Epoche: %u\n", ntohl(m_ntpPacket[10]) - 2208988800U);

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
    uint32_t m_ntpPacket[12];
};

int main(int argc, char* argv[])
{
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, serialBufferSize);

    ModemDetect detector(serial);

    NTPTask task(detector);

    Task* taskList[] = { &task, &detector, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);
    s.start();
}
