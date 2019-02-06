/*
 * Example code for IP communication in blocking mode
 */

#include <cstdint>
#include <cstdio>
#include <time.h>

#include "escheduler.h"
#include "etermios.h"
#include "esim7x00.h"

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

    ETask* taskList[] = {&serial, &commDev, NULL};

    EScheduler s(&tickFunction, taskList);

    commDev.setApn("internet");
    commDev.setHostPort("wttr.in", 80);
    commDev.connect();

    while (!commDev.isConnected())
        s.runTask();

    printf("*** Connected! ***\n");

    const char str[] =
        "GET / HTTP/1.1\r\n"
        "Host: wttr.in\r\n"
        "User-Agent: curl\r\n"
        "Connection: close\r\n\r\n";
    commDev.write((uint8_t*)str, sizeof(str) - 1);

    while(!commDev.bytesAvailable())
        s.runTask();

    uint64_t idleTime = tickFunction();
    while(tickFunction() - idleTime < 200)
    {
        if (commDev.bytesAvailable())
        {
            idleTime = tickFunction();
            char buf[41];
            uint16_t bytesRead = commDev.read((uint8_t*)buf, 40);
            buf[bytesRead] = '\0';
            printf("%s", buf);
        }

        s.runTask();
    }

    commDev.disconnect();
    while (!commDev.isIdle())
        s.runTask();

    return 0;
}
