/*
 * Example code for IP communication
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "escheduler.h"
#include "etermios.h"
#include "esim7x00.h"

class IPCommTask : public ETask
{
public:
    IPCommTask(ESim7x00CommDevice& commDev) :
        m_commDev(commDev)
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
            m_commDev.write((uint8_t*)str, sizeof(str));
        }

        E_REENTER_DELAY(250);

        for(;;)
        {
            if (m_commDev.bytesAvailable())
            {
                char buf[41];
                uint16_t bytesRead = m_commDev.read((uint8_t*)buf, 40);
                if (bytesRead == 0)
                {
                    printf("Error!!!\n");
                    exit(1);
                }
                if (bytesRead)
                {
                    buf[bytesRead] = '\0';
                    printf("%s", buf);
                }
            }
            else
            {
                E_REENTER_DELAY(100);
            }
        }

    E_END_TASK
    }

private:
    ESim7x00CommDevice& m_commDev;
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
    ETermios serial("/dev/ttyUSB0");
    ESim7x00CommDevice commDev(serial);
    IPCommTask task(commDev);

    ETask* taskList[] = {&task, &serial, &commDev, NULL};

    EScheduler s(&tickFunction, taskList);
    s.start();
}
