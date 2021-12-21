/*
 * Example code to autodetect comms module and update NTP Time
 */

#define MQTTCLIENT_QOS2 1
#include "cicada/commdevices/modemdetect.h"
#include "cicada/platform/linux/unixserial.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include "cicada/commdevices/blockingcommdev.h"
#include "cicada/commdevices/espressif.h"
#include "cicada/mqttcountdown.h"
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>


#include <MQTTClient.h>

using namespace Cicada;

class AutodetectNtp : public Task
{
  public:
    AutodetectNtp(ModemDetect& detector, const char* apn, const char* ssid, const char* pw) :
        m_detector(detector),
        m_commDev(NULL),
        _apn(apn),
        _ssid(ssid),
        _pw(pw) {}

    virtual void run()
    {
        SimCommDevice* simCommDev;
        EspressifDevice* espressifDev;

        E_BEGIN_TASK
        printf("*** EOL Begin! ***\n");
        printf("*** Modem Detection ***\n");
        m_detector.startDetection();

        E_REENTER_COND_TIMEOUT(m_detector.modemDetected(), 30000);
        checkExitEol(m_detector.modemDetected(), "*** Failed Modem Detection ***");
        m_commDev = m_detector.getDetectedModem(_commReadBuffer, _commWriteBuffer, _commBufferSize, _commBufferSize);

        simCommDev = dynamic_cast<SimCommDevice*>(m_commDev);
        if (simCommDev) {
            printf("*** SimCommDevice Detected ***\n");
            simCommDev->setApn(_apn);
        }
        espressifDev = dynamic_cast<EspressifDevice*>(m_commDev);
        if (espressifDev) {
            printf("*** ATCommDevice Detected ***\n");
            printf("Set SSID : \"%s\" and PW: \"%s\"\n", _ssid, _pw);
            espressifDev->setSSID(_ssid);
            espressifDev->setPassword(_pw);
        }


        printf("*** Start NTP Update Test ***\n");
        memset(m_ntpPacket, 0, sizeof(m_ntpPacket));

        m_commDev->setHostPort("pool.ntp.org", 123, IIPCommDevice::UDP);
        m_commDev->connect();

        E_REENTER_COND_TIMEOUT(m_commDev->isConnected(), 60000); // timeout 60 seconds
        checkExitEol(m_commDev->isConnected(), "*** Failed to connect! ***");
        printf("*** Connected! ***\n");

        m_ntpPacket[0] = 0x1b;
        m_commDev->write((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        E_REENTER_COND_TIMEOUT(m_commDev->bytesAvailable(), 60000);
        checkExitEol(m_commDev->bytesAvailable(), "*** Failed Receiving Data ***");

        m_commDev->read((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        {
            uint32_t epoch = ntohl(m_ntpPacket[10]) - 2208988800U;
            printf("Seconds since the Epoch: %u\n", epoch);
            checkExitEol(epoch>1636647985, "*** Failed NTP Time Update ***");
        }   

        m_commDev->disconnect();
        E_REENTER_COND_TIMEOUT(m_commDev->isIdle(), 30000);
        checkExitEol(m_commDev->isIdle(), "*** Failed Modem Disconnect ***");

        printf("*** Disconnected ***\n");

        E_END_TASK

        printf("*** EOL Success! ***\n");
        exit(EXIT_SUCCESS);
    }

    void checkExitEol(bool condition, const char* message){
        if(!condition){
            printf("*** FAILED EOL TEST %s\n", message);
            exit(EXIT_FAILURE);
        }
    }

  private:
    static const uint16_t _commBufferSize = 1200;
    uint8_t _commReadBuffer[_commBufferSize];
    uint8_t _commWriteBuffer[_commBufferSize];

    ModemDetect& m_detector;
    IPCommDevice* m_commDev;
    const char* _apn;
    const char* _ssid;
    const char* _pw;
    uint32_t m_ntpPacket[12];

    int arrivedcount = 0;
};

int main(int argc, char* argv[])
{
    const char* apn = "";
    const char* ssid = "";
    const char* pw = "";
    for(uint8_t i = 1; i<argc; i++){
        if(strcmp(argv[i],"apn")==0){
            apn=argv[i+1];
            i++;
        } else if(strcmp(argv[i],"ssid")==0){
            ssid=argv[i+1];
            i++;
        } else if(strcmp(argv[i],"pw")==0){
            pw=argv[i+1];
            i++;
        }
    }
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, serialBufferSize);

    ModemDetect detector(serial);

    AutodetectNtp task(detector, apn, ssid, pw);

    Task* taskList[] = { &task, &detector, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);
    s.start();
}
