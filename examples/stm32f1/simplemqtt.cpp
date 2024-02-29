/*
 * Simplest MQTT sample, which only connects and sends a message
 */

#include <string.h>

#include "cicada/commdevices/blockingcommdev.h"
#include "cicada/commdevices/sim7x00.h"
#include "cicada/mqttcountdown.h"
#include "cicada/platform/stm32f1/stm32uart.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"

#include <MQTTClient.h>

void System_Config(void)
{
    HAL_Init();

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

using namespace Cicada;

void yieldFunction(void* sched)
{
    ((Scheduler*)sched)->runTask();
}

int main(int argc, char* argv[])
{
    // System configuration for microcontroller
    System_Config();

    // Set up serial port
    const uint16_t serialBufferSize = 1504;
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];
    Stm32Uart serial(serialReadBuffer, serialWriteBuffer, serialBufferSize, USART1, GPIOA,
        GPIO_PIN_9, GPIO_PIN_10);

    // Set up modem driver connected to serial port
    const uint16_t commBufferSize = 1200;
    uint8_t commReadBuffer[commBufferSize];
    uint8_t commWriteBuffer[commBufferSize];
    Sim7x00CommDevice commDev(serial, commReadBuffer, commWriteBuffer, commBufferSize);

    // Set up task scheduler to call the modem driver's run() function
    Task* taskList[] = { &commDev, NULL };
    Scheduler s(&eTickFunction, taskList);

    // Set up MQTT client
    BlockingCommDevice bld(&commDev, eTickFunction, yieldFunction, &s);
    MQTT::Client<BlockingCommDevice, MQTTCountdown> client
        = MQTT::Client<BlockingCommDevice, MQTTCountdown>(bld);

    // Connect modem and IP channel
    commDev.setApn("internet");
    commDev.setHostPort("test.mosquitto.org", 1883);
    commDev.connect();
    while (!commDev.isConnected()) {
        yieldFunction(&s);
    }

    // Connect MQTT client
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)"enaccess";
    client.connect(data);

    // Send a message
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)"Hello World!";
    message.payloadlen = 13;
    client.publish("enaccess/test", message);

    // Disconnect everything
    client.disconnect();
    commDev.disconnect();
    while (!commDev.isIdle()) {
        yieldFunction(&s);
    }
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

void _putchar(char c) {}
}
