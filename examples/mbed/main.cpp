/*
 * Example code for IP communication using mbed,
 * also blinks an LED in another thread
 */

#include "mbed.h"
#include "cicada/platform/mbed/mbedserial.h"
#include "cicada/tick.h"
#include "cicada/commdevices/sim7x00.h"
#include "cicada/scheduler.h"

#define OS_MAINSTKSIZE 256

using namespace EnAccess;

Thread enAccessThread(osPriorityNormal, 8192);

class IPCommTask : public Task
{
  public:
    IPCommTask(Sim7x00CommDevice& commDev) :
        m_commDev(commDev),
        m_i(0)
    { }

    virtual void run()
    {
        E_BEGIN_TASK

        printf("Conneting ...\n");

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

void runEnAccess()
{
    MbedSerial serial(PA_9, PA_10);
    Sim7x00CommDevice commDev(serial);
    IPCommTask task(commDev);

    Task* taskList[] = {&task, &commDev, NULL};

    Scheduler s(&eTickFunction, taskList);
    s.start();
}

void blinkLed()
{
    DigitalOut led1(LED1);
    while (true) {
        // Blink LED and wait 0.5 seconds
        led1 = !led1;
        ThisThread::sleep_for(250);
    }
}

int main()
{
    enAccessThread.start(callback(runEnAccess));

    blinkLed();
}
