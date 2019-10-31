/*
 * Example code to get a timestamp via NTP.
 * This demonstrates the usage of UDP.
 */

#include "cicada/commdevices/sim7x00.h"
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
    NTPTask(SimCommDevice& commDev) : m_commDev(commDev) {}

    virtual void run()
    {
        E_BEGIN_TASK

        memset(m_ntpPacket, 0, sizeof(m_ntpPacket));

        m_commDev.setApn("internet");
        m_commDev.setHostPort("pool.ntp.org", 123, IIPCommDevice::UDP);
        m_commDev.connect();

        E_REENTER_COND(m_commDev.isConnected());

        printf("*** Connected! ***\n");

        m_ntpPacket[0] = 0x1b;
        m_commDev.write((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        E_REENTER_COND(m_commDev.bytesAvailable());

        m_commDev.read((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        printf("Seconds since the Epoche: %u\n", ntohl(m_ntpPacket[10]) - 2208988800U);

        m_commDev.disconnect();
        E_REENTER_COND(m_commDev.isIdle());

        printf("*** Disconnected ***\n");

        E_END_TASK
    }

  private:
    SimCommDevice& m_commDev;
    uint32_t m_ntpPacket[12];
};

int main(int argc, char* argv[])
{
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, serialBufferSize);
    const uint16_t commBufferSize = 1504;
    uint8_t commReadBuffer[commBufferSize];
    uint8_t commWriteBuffer[commBufferSize];
    Sim7x00CommDevice commDev(serial, commReadBuffer, commWriteBuffer, commBufferSize);
    NTPTask task(commDev);

    Task* taskList[] = { &task, &commDev, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);
    s.start();
}
