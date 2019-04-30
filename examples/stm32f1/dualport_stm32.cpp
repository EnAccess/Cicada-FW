/*
 * Example code for serial communication on STM32 bare metal
 */

#include <cstring>
#include "cicada/scheduler.h"
#include "cicada/platform/stm32f1/stm32uart.h"
#include "cicada/tick.h"
#include "stm32f1xx_hal.h"
#include "printf.h"

using namespace EnAccess;

static void SystemClock_Config(void);

class SerialTask : public Task
{
  public:
    SerialTask(BufferedSerial& serial) :
        m_serial(serial),
        m_i(0)
    { }

    virtual void run()
    {
        E_BEGIN_TASK

        for (m_i = 0; m_i < 100; m_i++) {
            {
                const char* send_str = "AT\r\n";
                printf("Sending command: %s", send_str);
                int bytesWritten =
                    m_serial.write(send_str, strlen(send_str));
                printf("%d bytes written\r\n", bytesWritten);
            }

            E_REENTER_COND_DELAY(m_serial.bytesAvailable(), 100);

            {
                char buf[32];
                int bytesReceived;
                bytesReceived = m_serial.read(buf, 31);
                printf("%d bytes received\r\n", bytesReceived);

                buf[bytesReceived] = '\0';
                printf("Received message: %s", buf);
            }

            E_REENTER_DELAY(500);
        }

        m_serial.close();

        E_END_TASK
    }

  private:
    BufferedSerial& m_serial;
    int m_i;
};

int main(int argc, char* argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    Stm32Uart debug;
    Stm32Uart serial(USART1, GPIOA, GPIO_PIN_9, GPIO_PIN_10);
    SerialTask task(serial);

    Task* taskList[] = {&task, NULL};

    Scheduler s(&eTickFunction, taskList);

    debug.open();
    serial.open();

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
