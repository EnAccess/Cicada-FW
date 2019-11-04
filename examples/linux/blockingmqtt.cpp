/*
 * Example code for using Eclipse Paho MQTT C/C++
 * client for Embedded platforms
 */

#define MQTTCLIENT_QOS2 1

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cicada/commdevices/blockingcommdev.h"
#include "cicada/commdevices/sim7x00.h"
#include "cicada/mqttcountdown.h"
#include "cicada/platform/linux/unixserial.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"

#include <MQTTClient.h>

using namespace Cicada;

void yieldFunction(void* sched)
{
    ((Scheduler*)sched)->runTask();
}

int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message& message = md.message;

    printf("Message %d arrived: qos %d, retained %d, dup %d, packetid %d\n", ++arrivedcount,
        message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\n", (int)message.payloadlen, (char*)message.payload);
}

// Most of the code taken from MQTT hello.cpp
int main(int argc, char* argv[])
{
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];
    UnixSerial serial(serialReadBuffer, serialWriteBuffer, serialBufferSize);
    const uint16_t commBufferSize = 1200;
    uint8_t commReadBuffer[commBufferSize];
    uint8_t commWriteBuffer[commBufferSize];
    Sim7x00CommDevice commDev(serial, commReadBuffer, commWriteBuffer, commBufferSize);

    Task* taskList[] = { &commDev, &serial, NULL };

    Scheduler s(&eTickFunction, taskList);

    BlockingCommDevice bld(commDev, eTickFunction, yieldFunction, &s);

    const char* topic = "mbed-sample";

    MQTT::Client<BlockingCommDevice, MQTTCountdown> client
        = MQTT::Client<BlockingCommDevice, MQTTCountdown>(bld);

    const char* hostname = "iot.eclipse.org";
    int port = 1883;
    printf("Connecting to %s:%d\n", hostname, port);
    commDev.setApn("internet");
    commDev.setHostPort(hostname, port);
    commDev.connect();
    while (!commDev.isConnected()) {
        yieldFunction(&s);
    }

    printf("MQTT connecting\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)"mbed-icraggs";
    int rc = client.connect(data);
    if (rc != 0)
        printf("rc from MQTT connect is %d\n", rc);
    printf("MQTT connected\n");

    rc = client.subscribe(topic, MQTT::QOS2, messageArrived);
    if (rc != 0)
        printf("rc from MQTT subscribe is %d\n", rc);

    MQTT::Message message;

    // QoS 0
    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app");
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf) + 1;
    rc = client.publish(topic, message);
    if (rc != 0)
        printf("Error %d from sending QoS 0 message\n", rc);
    else
        while (arrivedcount == 0)
            client.yield(100);

    // QoS 1
    printf("Now QoS 1\n");
    sprintf(buf, "Hello World!  QoS 1 message from app");
    message.qos = MQTT::QOS1;
    message.payloadlen = strlen(buf) + 1;
    rc = client.publish(topic, message);
    if (rc != 0)
        printf("Error %d from sending QoS 1 message\n", rc);
    else
        while (arrivedcount == 1)
            client.yield(100);

    // QoS 2
    sprintf(buf, "Hello World!  QoS 2 message from app");
    message.qos = MQTT::QOS2;
    message.payloadlen = strlen(buf) + 1;
    rc = client.publish(topic, message);
    if (rc != 0)
        printf("Error %d from sending QoS 2 message\n", rc);
    while (arrivedcount == 2)
        client.yield(100);

    rc = client.unsubscribe(topic);
    if (rc != 0)
        printf("rc from unsubscribe was %d\n", rc);

    rc = client.disconnect();
    if (rc != 0)
        printf("rc from disconnect was %d\n", rc);

    commDev.disconnect();

    while (!commDev.isIdle()) {
        yieldFunction(&s);
    }

    printf("Finishing with %d messages received\n", arrivedcount);

    return 0;
}
