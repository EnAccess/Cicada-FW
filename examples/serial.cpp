/*
 * Example code for serial communication
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "escheduler.h"
#include "eserial.h"
#include "etick.h"

class SerialTask : public ETask
{
public:
    SerialTask(ESerial& serial) :
        m_serial(serial)
    { }

    virtual void run()
    {
    E_BEGIN_TASK

        if (!m_serial.open())
        {
            printf("Error opening serial port %s\n", m_serial.portName());
            exit(1);
        }

        if (!m_serial.setSerialConfig(115200, 8))
        {
            printf("Error setting serial configuration\n");
            exit(1);
        }

        if (m_serial.portName())
            printf("Serial port %s open\n", m_serial.portName());
        else
            printf("Serial port open\n");

        {
            const char* send_str = "AT\r\n";
            printf("Sending command: %s", send_str);
            int bytesWritten =
                m_serial.write(send_str, strlen(send_str));
            printf("%d bytes written\n", bytesWritten);
        }

        E_REENTER_COND_DELAY(m_serial.bytesAvailable(), 100);

        {
            char buf[32];
            int bytesReceived;
            bytesReceived = m_serial.read(buf, 31);
            printf("%d bytes received\n", bytesReceived);

            buf[bytesReceived] = '\0';
            printf("Received message: %s", buf);
        }

        m_serial.close();

    E_END_TASK
    }

private:
    ESerial& m_serial;
};

int main(int argc, char * argv[])
{
    ESerial serial;
    SerialTask task(serial);

    ETask* taskList[] = {&task, dynamic_cast<ETask*>(&serial), NULL};

    EScheduler s(&eTickFunction, taskList);
    s.start();
}
