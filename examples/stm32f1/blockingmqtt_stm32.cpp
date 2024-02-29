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
#include "cicada/platform/stm32f1/stm32uart.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include "printf.h"

#include <MQTTClient.h>

using namespace Cicada;

static void SystemClock_Config(void);

void yieldFunction(void* sched)
{
    ((Scheduler*)sched)->runTask();
}

int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message& message = md.message;

    printf("Message %d arrived: qos %d, retained %d, dup %d, packetid %d\r\n", ++arrivedcount,
        message.qos, message.retained, message.dup, message.id);
    printf("Payload %.*s\r\n", (int)message.payloadlen, (char*)message.payload);
}

// Most of the code taken from MQTT hello.cpp
int main(int argc, char* argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    HAL_Delay(2000);

    const uint16_t serialBufferSize = 1504;
    char readBufferDebug[serialBufferSize];
    char writeBufferDebug[serialBufferSize];
    Stm32Uart debug(readBufferDebug, writeBufferDebug, serialBufferSize);
    char readBufferSerial[serialBufferSize];
    char writeBufferSerial[serialBufferSize];
    Stm32Uart serial(readBufferSerial, writeBufferSerial, serialBufferSize, USART1, GPIOA,
        GPIO_PIN_9, GPIO_PIN_10);

    const uint16_t commBufferSize = 1200;
    uint8_t commReadBuffer[commBufferSize];
    uint8_t commWriteBuffer[commBufferSize];
    // Change this class to the modem driver you want
    Sim7x00CommDevice commDev(serial, commReadBuffer, commWriteBuffer, commBufferSize);

    debug.open();

    Task* taskList[] = { &commDev, NULL };

    Scheduler s(&eTickFunction, taskList);

    BlockingCommDevice bld(&commDev, eTickFunction, yieldFunction, &s);

    const char* topic = "enaccess/test";

    MQTT::Client<BlockingCommDevice, MQTTCountdown> client
        = MQTT::Client<BlockingCommDevice, MQTTCountdown>(bld);

    const char* hostname = "test.mosquitto.org";
    int port = 1883;
    printf("Connecting to %s:%d\r\n", hostname, port);
    commDev.setApn("internet");
    commDev.setHostPort(hostname, port);
    commDev.connect();
    while (!commDev.isConnected()) {
        yieldFunction(&s);
    }

    printf("MQTT connecting\r\n");
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)"enaccess";
    int rc = client.connect(data);
    if (rc != 0)
        printf("rc from MQTT connect is %d\r\n", rc);
    printf("MQTT connected\r\n");

    rc = client.subscribe(topic, MQTT::QOS2, messageArrived);
    if (rc != 0)
        printf("rc from MQTT subscribe is %d\r\n", rc);

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
        printf("Error %d from sending QoS 0 message\r\n", rc);
    else
        while (arrivedcount == 0)
            client.yield(100);

    // QoS 1
    printf("Now QoS 1\r\n");
    sprintf(buf, "Hello World!  QoS 1 message from app");
    message.qos = MQTT::QOS1;
    message.payloadlen = strlen(buf) + 1;
    rc = client.publish(topic, message);
    if (rc != 0)
        printf("Error %d from sending QoS 1 message\r\n", rc);
    else
        while (arrivedcount == 1)
            client.yield(100);

    // QoS 2
    sprintf(buf, "Hello World!  QoS 2 message from app");
    message.qos = MQTT::QOS2;
    message.payloadlen = strlen(buf) + 1;
    rc = client.publish(topic, message);
    if (rc != 0)
        printf("Error %d from sending QoS 2 message\r\n", rc);
    while (arrivedcount == 2)
        client.yield(100);

    rc = client.unsubscribe(topic);
    if (rc != 0)
        printf("rc from unsubscribe was %d\r\n", rc);

    rc = client.disconnect();
    if (rc != 0)
        printf("rc from disconnect was %d\r\n", rc);

    commDev.disconnect();

    while (!commDev.isIdle()) {
        yieldFunction(&s);
    }

    printf("Finishing with %d messages received\r\n", arrivedcount);

    for (;;)
        ;
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /**Initializes the CPU, AHB and APB busses clocks
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /**Initializes the CPU, AHB and APB busses clocks
     */
    RCC_ClkInitStruct.ClockType
        = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* Interrupt handler */
extern "C" {
void SysTick_Handler()
{
    HAL_IncTick();
}

void USART1_IRQHandler()
{
    static Stm32Uart* instance = Stm32Uart::getInstance(USART1);
    instance->handleInterrupt();
}

void USART2_IRQHandler()
{
    static Stm32Uart* instance = Stm32Uart::getInstance(USART2);
    instance->handleInterrupt();
}

void _putchar(char c)
{
    static Stm32Uart* serial = NULL;
    if (!serial) {
        serial = Stm32Uart::getInstance(USART2);
    }
    serial->write(c);
}
}
