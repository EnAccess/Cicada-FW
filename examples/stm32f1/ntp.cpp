/*
 * Example code to get a timestamp via NTP.
 * This demonstrates the usage of UDP.
 */

#include "cicada/commdevices/sim7x00.h"
#include "cicada/platform/stm32f1/stm32uart.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include "printf.h"
#include "stm32f1xx_hal.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Cicada;

static void SystemClock_Config(void);

uint32_t byteSwap(uint32_t x)
{
    uint32_t ret = (x & 0xff) << 24;
    ret |= (x & 0xff00) << 8;
    ret |= (x & 0xff0000UL) >> 8;
    ret |= (x & 0xff000000UL) >> 24;
    return ret;
}

class NTPTask : public Task
{
  public:
    NTPTask(SimCommDevice& commDev) : m_commDev(commDev) {}

    virtual void run()
    {
        E_BEGIN_TASK

        memset(m_ntpPacket, 0, sizeof(m_ntpPacket));

        m_commDev.setApn("internet");
        m_commDev.setHostPort("pool.ntp.org", 123, IIPCommDevice::UDP);
        m_commDev.connect();

        E_REENTER_COND(m_commDev.isConnected());

        printf("*** Connected! ***\r\n");

        m_ntpPacket[0] = 0x1b;
        m_commDev.write((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        E_REENTER_COND(m_commDev.bytesAvailable());

        m_commDev.read((uint8_t*)m_ntpPacket, sizeof(m_ntpPacket));

        printf(
            "Seconds since the Epoche: %" PRIu32 "\r\n", byteSwap(m_ntpPacket[10]) - 2208988800U);

        m_commDev.disconnect();
        E_REENTER_COND(m_commDev.isIdle());

        printf("*** Disconnected ***\r\n");

        E_END_TASK
    }

  private:
    SimCommDevice& m_commDev;
    uint32_t m_ntpPacket[12];
};

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

    NTPTask task(commDev);

    Task* taskList[] = { &task, &commDev, NULL };

    Scheduler s(&eTickFunction, taskList);
    debug.open();
    s.start();
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
