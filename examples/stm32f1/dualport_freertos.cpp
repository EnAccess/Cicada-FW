/*
 * Example using the FreeRTOS cooperative scheduler (instead of the one included with the library).
 * NOTE: Cicada is not thread safe and does not support preemption! Don't use the preemptive
 * scheduler of FreeRTOS.
 */

#include "FreeRTOS.h"
#include "cicada/platform/stm32f1/stm32uart.h"
#include "printf.h"
#include "stm32f1xx_hal.h"
#include "task.h"

#define STACK_SIZE 200

using namespace Cicada;

// Stack for FreeRTOS task
StackType_t xStack[STACK_SIZE];
StaticTask_t xTaskBuffer;

static void SystemClock_Config(void);
extern "C" void vApplicationGetIdleTaskMemory(StaticTask_t** ppxIdleTaskTCBBuffer,
    StackType_t** ppxIdleTaskStackBuffer, uint32_t* pulIdleTaskStackSize);

void serialTask(void* parameters)
{
    Stm32Uart* serial = static_cast<Stm32Uart*>(parameters);

    for (;;) {
        const char* send_str = "AT\r\n";
        printf("Sending command: %s", send_str);
        int bytesWritten = serial->write(send_str);
        printf("%d bytes written\r\n", bytesWritten);

        while (!serial->bytesAvailable()) {
            vTaskDelay(100);
        }

        char buf[32];
        int bytesReceived;
        bytesReceived = serial->read(buf, 31);
        printf("%d bytes received\r\n", bytesReceived);

        buf[bytesReceived] = '\0';
        printf("Received message: %s", buf);

        vTaskDelay(500);
    }
}

int main(int argc, char* argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    Stm32Uart debug;
    Stm32Uart serial(USART1, GPIOA, GPIO_PIN_9, GPIO_PIN_10);

    // Create serial task
    xTaskCreateStatic(
        serialTask, "serialTask", STACK_SIZE, &serial, tskIDLE_PRIORITY, xStack, &xTaskBuffer);

    debug.open();
    serial.open();

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
