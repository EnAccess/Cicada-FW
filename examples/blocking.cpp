/*
 * Example code for IP communication in blocking mode
 */

#include <cstdint>
#include <cstdio>
#include <time.h>

#include "escheduler.h"
#include "eserial.h"
#include "eblockingcommdev.h"
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

void yieldFunction(void *sched) {
    ((EScheduler*)sched)->runTask();
}

int main(int argc, char * argv[])
{
    ESerial serial;
    ESim7x00CommDevice commDev(serial);

    ETask* taskList[] = {&serial, &commDev, NULL};

    EScheduler s(&tickFunction, taskList);

    EBlockingCommDevice bld(commDev, tickFunction, yieldFunction, &s);

    commDev.setApn("internet");
    commDev.setHostPort("wttr.in", 80);
    commDev.connect();

    while (!commDev.isConnected())
        yieldFunction(&s);

    printf("*** Connected! ***\n");

    const char str[] =
        "GET / HTTP/1.1\r\n"
        "Host: wttr.in\r\n"
        "User-Agent: curl\r\n"
        "Connection: close\r\n\r\n";
    bld.write((uint8_t*)str, sizeof(str) - 1, 200);

    int bytesRead;
    do {
        char buf[41];
        bytesRead = bld.read((uint8_t*)buf, 40, 200);
        buf[bytesRead] = '\0';
        printf("%s", buf);
    } while(bytesRead);

    commDev.disconnect();
    while (!commDev.isIdle())
        yieldFunction(&s);

    return 0;
}
