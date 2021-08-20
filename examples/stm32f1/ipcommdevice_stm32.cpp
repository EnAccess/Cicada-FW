/*
 * Example code for IP communication
 */

#include "cicada/commdevices/modemdetect.h"
#include "cicada/platform/stm32f1/stm32uart.h"
#include "cicada/scheduler.h"
#include "cicada/tick.h"
#include "printf.h"
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

using namespace Cicada;

static void SystemClock_Config(void);

class IPCommTask : public Task
{
  public:
    IPCommTask(ModemDetect& detector) :
        m_detector(detector),
        m_commDev(NULL),
        m_i(0) {}

    virtual void run()
    {
        E_BEGIN_TASK

        m_detector.startDetection();

        E_REENTER_COND(m_detector.modemDetected());
        m_commDev = m_detector.getDetectedModem(_commReadBuffer, _commWriteBuffer, _commBufferSize, _commBufferSize);

        printf("Connecting ...\r\n");

        m_commDev->setApn("internet");
        m_commDev->setHostPort("wttr.in", 80);
        m_commDev->connect();

        E_REENTER_COND(m_commDev->isConnected());

        printf("*** Connected! ***\r\n");

        {
            const char str[] = "GET / HTTP/1.1\r\n"
                               "Host: wttr.in\r\n"
                               "User-Agent: curl\r\n"
                               "Connection: close\r\n\r\n";
            m_commDev->write((uint8_t*)str, sizeof(str) - 1);
        }

        E_REENTER_COND(m_commDev->bytesAvailable());

        for (m_i = 0; m_i < 400; m_i++) {
            if (m_commDev->bytesAvailable()) {
                char buf[41];
                uint16_t bytesRead = m_commDev->read((uint8_t*)buf, 40);
                for (int i = 0; i < bytesRead; i++) {
                    if (buf[i] == '\n') {
                        _putchar('\r');
                    }
                    _putchar(buf[i]);
                }
            } else {
                E_REENTER_DELAY(20);
            }
        }

        m_commDev->disconnect();
        E_REENTER_COND(m_commDev->isIdle());

        printf("*** Disconnected ***\r\n");

        E_END_TASK
    }

  private:
    static const uint16_t _commBufferSize = 1200;
    uint8_t _commReadBuffer[_commBufferSize];
    uint8_t _commWriteBuffer[_commBufferSize];

    ModemDetect& m_detector;
    SimCommDevice* m_commDev;
    int m_i;
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
    Stm32Uart serial(readBufferSerial, writeBufferSerial, serialBufferSize,
                     USART1, GPIOA, GPIO_PIN_9, GPIO_PIN_10);

    ModemDetect detector(serial);

    IPCommTask task(detector);

    Task* taskList[] = { &task, &detector, NULL };

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
