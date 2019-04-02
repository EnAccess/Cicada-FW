/*
 * Sends a packet of pseudo-random data every 4 seconds and loops indefinitely.
 * This example is meant for long term testing of MQTT packat transmission.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "scheduler.h"
#include "stm32uart.h"
#include "sim7x00.h"
#include "blockingcommdev.h"
#include "tick.h"
#include "mqttcountdown.h"

#include <MQTTClient.h>

#define PAYLOAD_LENGTH 80

using namespace EnAccess;

static void SystemClock_Config(void);

void yieldFunction(void* sched)
{
    ((Scheduler*)sched)->runTask();
}

uint32_t generateRand(uint32_t rseed)
{
    return (rseed * 1103515245 + 12345) & INT32_MAX;
}

int main(int argc, char* argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    Stm32Uart serial(USART1, GPIOA, GPIO_PIN_9, GPIO_PIN_10);
    Sim7x00CommDevice commDev(serial);

    Task* taskList[] = {&commDev, NULL};

    Scheduler s(&eTickFunction, taskList);

    BlockingCommDevice bld(commDev, eTickFunction, yieldFunction, &s);

    const char* topic = "enaccess/test";

    MQTT::Client<BlockingCommDevice, MQTTCountdown> client =
        MQTT::Client<BlockingCommDevice, MQTTCountdown>(bld);

    // Connect modem
    const char* hostname = "test.mosquitto.org";
    int port = 1883;
    commDev.setApn("internet");
    commDev.setHostPort(hostname, port);
    commDev.connect();
    while (!commDev.isConnected()) {
        yieldFunction(&s);
    }

    // Setup MQTT client
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)"enaccess";
    client.connect(data);

    // Start transmitting
    uint32_t sequenceNumber = 0;
    while(true) {
        char buf[PAYLOAD_LENGTH];
        *(uint32_t*)buf = sequenceNumber;

        uint32_t seed = sequenceNumber;
        for (int i = 4; i < PAYLOAD_LENGTH; i++) {
            seed = generateRand(seed);
            buf[i] = seed & UINT8_MAX;
        }
        MQTT::Message message;
        message.qos = MQTT::QOS0;
        message.retained = false;
        message.dup = false;
        message.payload = (void*)buf;
        message.payloadlen = PAYLOAD_LENGTH;
        client.publish(topic, message);

        uint32_t endTime = eTickFunction() + 4000;
        while (eTickFunction() < endTime) {
            client.yield(4000);
        }

        sequenceNumber++;
    }
}

void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);
}

/* Interrupt handler */
extern "C"
{
    void SysTick_Handler()
    {
        HAL_IncTick();
    }

    void USART1_IRQHandler()
    {
        static Stm32Uart* instance = Stm32Uart::getInstance(USART1);
        instance->handleInterrupt();
    }
}
