/*
 * Example code for serial communication on STM32
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

        if (!m_serial.setSerialConfig(115200, 8)) {
            //TODO: Error
        }

        if (!m_serial.open()) {
            //TODO: Error
        }

        E_REENTER_DELAY(100);

        for (;;) {
            printf("Hello world\r\n");
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

    Stm32Uart serial;
    SerialTask task(serial);

    Task* taskList[] = {&task, NULL};

    Scheduler s(&eTickFunction, taskList);
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
