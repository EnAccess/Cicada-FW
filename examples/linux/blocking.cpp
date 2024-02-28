/*
 * Example code for IP communication in blocking mode
 */

#include "cicada/commdevices/blockingcommdev.h"
#include "cicada/commdevices/sim7x00.h"
#include "cicada/platform/linux/unixserial.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include <cstdint>
#include "printf.h"

using namespace Cicada;

void yieldFunction(void* sched)
{
    ((Scheduler*)sched)->runTask();
}

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

    Task* taskList[] = { &commDev, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);

    BlockingCommDevice bld(&commDev, eTickFunction, yieldFunction, &s);

    commDev.setApn("internet");
    commDev.setHostPort("wttr.in", 80);
    commDev.connect();

    while (!commDev.isConnected())
        yieldFunction(&s);

    printf("*** Connected! ***\n");

    const char str[] = "GET / HTTP/1.1\r\n"
                       "Host: wttr.in\r\n"
                       "User-Agent: curl\r\n"
                       "Connection: close\r\n\r\n";
    bld.write((uint8_t*)str, sizeof(str) - 1, 2000);

    int bytesRead;
    do {
        char buf[41];
        bytesRead = bld.read((uint8_t*)buf, 40, 2000);
        buf[bytesRead] = '\0';
        printf("%s", buf);
    } while (bytesRead);

    commDev.disconnect();
    while (!commDev.isIdle())
        yieldFunction(&s);

    return 0;
}
