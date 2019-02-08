/*
 * Example code for IP communication
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "escheduler.h"
#include "eserial.h"
#include "esim7x00.h"

class IPCommTask : public ETask
{
public:
    IPCommTask(ESim7x00CommDevice& commDev) :
        m_commDev(commDev),
        m_i(0)
    { }

    virtual void run()
    {
    E_BEGIN_TASK

        m_commDev.setApn("internet");
        m_commDev.setHostPort("wttr.in", 80);
        m_commDev.connect();

        E_REENTER_COND(m_commDev.isConnected());

        printf("*** Connected! ***\n");

        {
            const char str[] =
                "GET / HTTP/1.1\r\n"
                "Host: wttr.in\r\n"
                "User-Agent: curl\r\n"
                "Connection: close\r\n\r\n";
            m_commDev.write((uint8_t*)str, sizeof(str) - 1);
        }

        E_REENTER_COND(m_commDev.bytesAvailable());

        for(m_i = 0; m_i < 400; m_i++)
        {
            if (m_commDev.bytesAvailable())
            {
                char buf[41];
                uint16_t bytesRead = m_commDev.read((uint8_t*)buf, 40);
                buf[bytesRead] = '\0';
                printf("%s", buf);
            }
            else
            {
                E_REENTER_DELAY(10);
            }
        }

        m_commDev.disconnect();
        E_REENTER_COND(m_commDev.isIdle());

        printf("*** Disconnected ***\n");

    E_END_TASK
    }

private:
    ESim7x00CommDevice& m_commDev;
    int m_i;
};

uint64_t tickFunction()
{
    uint64_t ms;
    struct timespec spec;

    clock_gettime(CLOCK_REALTIME, &spec);
    
    ms  = spec.tv_sec * 1000;
    ms += spec.tv_nsec / 1.0e6;

    return ms;
}

int main(int argc, char * argv[])
{
    ESerial serial;
    ESim7x00CommDevice commDev(serial);
    IPCommTask task(commDev);

    ETask* taskList[] = {&task, &serial, &commDev, NULL};

    EScheduler s(&tickFunction, taskList);
    s.start();
}
