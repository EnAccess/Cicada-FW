/*
 * Example code for IP communication
 */

#include "cicada/commdevices/sim7x00.h"
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
    IPCommTask(Sim7x00CommDevice& commDev) : m_commDev(commDev), m_i(0) {}

    virtual void run()
    {
        E_BEGIN_TASK

        m_commDev.setApn("internet");
        m_commDev.setHostPort("wttr.in", 80);
        m_commDev.connect();

        E_REENTER_COND(m_commDev.isConnected());

        printf("*** Connected! ***\n");

        {
            const char str[] = "GET / HTTP/1.1\r\n"
                               "Host: wttr.in\r\n"
                               "User-Agent: curl\r\n"
                               "Connection: close\r\n\r\n";
            m_commDev.write((uint8_t*)str, sizeof(str) - 1);
        }

        E_REENTER_COND(m_commDev.bytesAvailable());

        for (m_i = 0; m_i < 400; m_i++) {
            if (m_commDev.bytesAvailable()) {
                char buf[41];
                uint16_t bytesRead = m_commDev.read((uint8_t*)buf, 40);
                buf[bytesRead] = '\0';
                printf("%s", buf);
            } else {
                E_REENTER_DELAY(10);
            }
        }

        m_commDev.disconnect();
        E_REENTER_COND(m_commDev.isIdle());

        printf("*** Disconnected ***\n");

        E_END_TASK
    }

  private:
    Sim7x00CommDevice& m_commDev;
    int m_i;
};

int main(int argc, char* argv[])
{
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, serialBufferSize);
    const uint16_t commBufferSize = 1200;
    uint8_t commReadBuffer[commBufferSize];
    uint8_t commWriteBuffer[commBufferSize];
    Sim7x00CommDevice commDev(serial, commReadBuffer, commWriteBuffer, commBufferSize);
    IPCommTask task(commDev);

    Task* taskList[] = { &task, &commDev, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);
    s.start();
}
