/*
 * Example code for serial communication
 */

#include "cicada/platform/linux/unixserial.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Cicada;

class SerialTask : public Task
{
  public:
    SerialTask(BufferedSerial& serial) : m_serial(serial), m_i(0) {}

    virtual void run()
    {
        E_BEGIN_TASK

        if (!m_serial.setSerialConfig(115200, 8)) {
            printf("Error setting serial configuration\n");
            exit(1);
        }

        if (!m_serial.open()) {
            printf("Error opening serial port %s\n", m_serial.portName());
            exit(1);
        }

        if (m_serial.portName())
            printf("Serial port %s open\n", m_serial.portName());
        else
            printf("Serial port open\n");

        for (m_i = 0; m_i < 100; m_i++) {
            {
                const char* send_str = "AT\r\n";
                printf("Sending command: %s", send_str);
                int bytesWritten = m_serial.write((const uint8_t*)send_str, strlen(send_str));
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

            E_REENTER_DELAY(500);
        }

        m_serial.close();

        printf("Serial port closed\n");

        E_END_TASK
    }

  private:
    BufferedSerial& m_serial;
    int m_i;
};

int main(int argc, char* argv[])
{
    const uint16_t bufferSize = 1504;
    char readBuffer[bufferSize];
    char writeBuffer[bufferSize];
    UnixSerial serial(readBuffer, writeBuffer, bufferSize);
    SerialTask task(serial);

    Task* taskList[] = { &task, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);
    s.start();
}
