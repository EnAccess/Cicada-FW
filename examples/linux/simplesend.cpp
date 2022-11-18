/*
 * Example code for simply sending a sequence of bytes
 */

#include "cicada/platform/linux/unixserial.h"
#include "cicada/commdevices/rakrui3.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Cicada;

class SimpleSend : public Task
{
  public:
    SimpleSend(RakDevice& commDev) :
        m_commDev(commDev)
    {}

    virtual void run()
    {
        E_BEGIN_TASK

        m_commDev.setAppKey("cc20fa8c1811baa8d2855e8cbbcda48a");
        m_commDev.setPort(5);
        m_commDev.connect();

        E_REENTER_COND(m_commDev.isConnected());

        printf("*** Connected! ***\n");

        for (m_i=0; m_i<10; m_i++) {
            m_commDev.write((uint8_t*)m_str, strlen(m_str));
            E_REENTER_COND(m_commDev.writeBufferProcessed());
        }

        E_REENTER_COND(m_commDev.bytesAvailable());

        char buf[41];
        uint16_t bytesRead = m_commDev.read((uint8_t*)buf, 40);
        buf[bytesRead] = '\0';
        printf("*** Data ***\n");
        printf("%s\n", buf);
        printf("*** Data end ***\n");

        E_END_TASK
    }

  private:
    RakDevice& m_commDev;
    const char* m_str = "Hello, World!";
    int m_i;
};

int main(int argc, char* argv[])
{
    const uint16_t bufferSize = 1504;
    char serialReadBuffer[bufferSize];
    char serialWriteBuffer[bufferSize];
    uint8_t commReadBuffer[bufferSize];
    uint8_t commWriteBuffer[bufferSize];
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, bufferSize);
    RakDevice commDev(serial, commReadBuffer, commWriteBuffer, bufferSize);
    SimpleSend task(commDev);

    Task* taskList[] = { &task, &commDev, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);
    s.start();
}
