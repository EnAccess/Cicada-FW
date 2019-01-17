/*
 * Example code for serial communication on Linux / Max OS X / Unix
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include "escheduler.h"
#include "etermios.h"

class SerialTask : public ETask
{
public:
    SerialTask() :
        m_serial("/dev/ttyUSB0")
    { }

    virtual void run()
    {
    E_BEGIN_TASK

        if (!m_serial.open())
        {
            printf("Error opening serial port\n");
            exit(1);
        }

        if (!m_serial.setSerialConfig(115200, 8))
        {
            printf("Error setting serial configuration\n");
            exit(1);
        }

        printf("Serial port %s open\n", m_serial.portName());

        {
            const char* send_str = "AT\r\n";
            printf("Sending command: %s", send_str);
            int bytesWritten =
                m_serial.write((uint8_t*)send_str, strlen(send_str));
            printf("%d bytes written\n", bytesWritten);
        }

        E_REENTER_COND_DELAY(m_serial.bytesAvailable(), 100);

        {
            char buf[32];
            int bytesReceived;
            bytesReceived = m_serial.read((uint8_t*)buf, 31);
            printf("%d bytes received\n", bytesReceived);

            buf[bytesReceived] = '\0';
            printf("Received message: %s", buf);
        }

        m_serial.close();

    E_END_TASK
    }

private:
    ETermios m_serial;
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
    SerialTask task;

    ETask* taskList[] = {&task, NULL};

    EScheduler s(&tickFunction, taskList);
    s.start();
}
