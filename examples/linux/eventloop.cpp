/*
 * Example fetching a website using a loop
 */

#include "cicada/commdevices/modemdetect.h"
#include "cicada/platform/linux/unixserial.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Cicada;

int main(int argc, char* argv[])
{
    // Function variables
    IPCommDevice* commDevice = nullptr;
    bool detectedFlag = false;
    bool connectedFlag = false;

    // UART driver buffers
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];

    // Modem driver buffers
    const uint16_t commBufferSize = 1200;
    uint8_t commReadBuffer[commBufferSize];
    uint8_t commWriteBuffer[commBufferSize];

    // Driver objects
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, serialBufferSize);
    ModemDetect detector(serial);

    // Cicada scheduler to run internal tasks
    Task* taskList[] = { &serial, &detector, NULL };
    Scheduler cicadaTasks(&eTickFunction, taskList);

    detector.startDetection();

    // Main event loop
    while (true) {

        // Periodically run Cicada's internal tasks
        cicadaTasks.runTask();

        // Event: Modem has been detected
        if (!detectedFlag && detector.modemDetected()) {
            commDevice = detector.getDetectedModem(
                commReadBuffer, commWriteBuffer, commBufferSize, commBufferSize);

            // If the device is a SIMCom 2G/4G modem, set the APN
            SimCommDevice* simCommDev = dynamic_cast<SimCommDevice*>(commDevice);
            if (simCommDev) {
                printf("Detected SIMCom 2G/4G modem\n");
                simCommDev->setApn("iot-eu.aer.net");
            }

            // If the device is a Espressif WiFi module, set SSID and password
            EspressifDevice* espressifDev = dynamic_cast<EspressifDevice*>(commDevice);
            if (espressifDev) {
                printf("Detected Espressif WiFi module\n");
                espressifDev->setSSID("your_ssid");
                espressifDev->setPassword("your_pass");
            }

            // Set host and port to connect to
            commDevice->setHostPort("wttr.in", 80);

            // Ask to modem driver to connect
            commDevice->connect();

            detectedFlag = true;
        }

        // Event: Modem connected to peer
        if (!connectedFlag && commDevice && commDevice->isConnected()) {
            printf("*** Connected! ***\n");

            // Send HTTP request to web server
            const char str[] = "GET / HTTP/1.1\r\n"
                               "Host: wttr.in\r\n"
                               "User-Agent: curl\r\n"
                               "Connection: close\r\n\r\n";
            commDevice->write((uint8_t*)str, sizeof(str) - 1);

            connectedFlag = true;
        }

        // Event: Data received from the webserver
        if (commDevice && commDevice->bytesAvailable()) {
            // Read received data and print them on the terminal
            char buf[41];
            uint16_t bytesRead = commDevice->read((uint8_t*)buf, 40);
            buf[bytesRead] = '\0';
            printf("%s", buf);
        }

        // Event: Modem back to idle state (disconnected)
        if (connectedFlag && commDevice && commDevice->isIdle()) {
            printf("*** Disconnected ***\n");
            // Exit main event loop
            break;
        }
    }

    return 0;
}
