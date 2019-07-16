/*
 * Example code for IP communication using the FreeRTOS cooperative scheduler (instead of the
 * one included with the library).
 * NOTE: Cicada is not thread safe and does not support preemption! Don't use the preemptive
 * scheduler of FreeRTOS.
 */

#include "FreeRTOS.h"
#include "cicada/commdevices/sim7x00.h"
#include "cicada/platform/stm32f1/stm32uart.h"
#include "printf.h"
#include "stm32f1xx_hal.h"
#include "task.h"

#define STACK_SIZE_COMM 3072
#define STACK_SIZE_RUNTASK 256

using namespace Cicada;

// Stack for FreeRTOS task
StackType_t xStackTask[STACK_SIZE_COMM];
StackType_t xStackComm[STACK_SIZE_RUNTASK];
StaticTask_t xTaskBuffer;
StaticTask_t xCommBuffer;

static void SystemClock_Config(void);
extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
    StackType_t** ppxIdleTaskStackBuffer, uint32_t* pulIdleTaskStackSize);

void runTask(void* parameters)
{
    Task* t = (Task*)parameters;

    while (true) {
        t->run();
        vTaskDelay(t->delay());
    }
}

void ipCommTask(void* parameters)
{
    Stm32Uart debug;
    debug.open();

    Stm32Uart serial(USART1, GPIOA, GPIO_PIN_9, GPIO_PIN_10);

    // Change this class to the modem driver you want
    Sim7x00CommDevice commDev(serial);

    // Run modem driver task
    xTaskCreateStatic(runTask, "runTask", STACK_SIZE_RUNTASK, static_cast<Task*>(&commDev),
        tskIDLE_PRIORITY, xStackTask, &xTaskBuffer);

    printf("Connecting ...\r\n");

    commDev.setApn("internet");
    commDev.setHostPort("wttr.in", 80);
    commDev.connect();

    while (!commDev.isConnected()) {
        taskYIELD();
    }

    printf("*** Connected! ***\r\n");

    const char str[] = "GET / HTTP/1.1\r\n"
                       "Host: wttr.in\r\n"
                       "User-Agent: curl\r\n"
                       "Connection: close\r\n\r\n";
    commDev.write((uint8_t*)str, sizeof(str) - 1);

    while (!commDev.bytesAvailable()) {
        taskYIELD();
    }

    for (int i = 0; i < 400; i++) {
        if (commDev.bytesAvailable()) {
            char buf[41];
            uint16_t bytesRead = commDev.read((uint8_t*)buf, 40);
            for (int i = 0; i < bytesRead; i++) {
                if (buf[i] == '\n') {
                    _putchar('\r');
                }
                _putchar(buf[i]);
            }
        } else {
            vTaskDelay(20);
        }
    }

    commDev.disconnect();
    while (!commDev.isIdle()) {
        taskYIELD();
    }

    printf("*** Disconnected ***\r\n");

    for (;;)
        ;
}

int main(int argc, char* argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    // Run communication task
    xTaskCreateStatic(ipCommTask, "ipCommTask", STACK_SIZE_COMM, NULL, tskIDLE_PRIORITY, xStackComm,
        &xCommBuffer);

    vTaskStartScheduler();
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

extern "C" {
/* configSUPPORT_STATIC_ALLOCATION is set to 1, so the application must provide an
   implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
   used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
    StackType_t** ppxIdleTaskStackBuffer, uint32_t* pulIdleTaskStackSize)
{
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

void xPortSysTickHandler(void);
void SysTick_Handler()
{
    HAL_IncTick();
    xPortSysTickHandler();
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
