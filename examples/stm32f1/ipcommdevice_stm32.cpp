/*
 * Example code for IP communication
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "scheduler.h"
#include "stm32uart.h"
#include "sim7x00.h"
#include "tick.h"
#include "stm32f1xx_hal.h"
#include "printf.h"

using namespace EnAccess;

static void SystemClock_Config(void);

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

int main(int argc, char* argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    Stm32Uart debug(USART3, GPIOB);
    Stm32Uart serial(UART4, GPIOC);
    Sim7x00CommDevice commDev(serial);
    IPCommTask task(commDev);

    Task* taskList[] = {&task, &commDev, NULL};

    Scheduler s(&eTickFunction, taskList);
    debug.open();
    s.start();
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

    void USART3_IRQHandler()
    {
        static volatile Stm32Uart* instance = Stm32Uart::getInstance(USART3);
        instance->handleInterrupt();
    }

    void UART4_IRQHandler()
    {
        static volatile Stm32Uart* instance = Stm32Uart::getInstance(UART4);
        instance->handleInterrupt();
    }

    void _putchar(char c)
    {
        static Stm32Uart* serial = NULL;
        if (!serial) {
            serial = Stm32Uart::getInstance(USART3);
        }
        serial->write(c);
    }
}
