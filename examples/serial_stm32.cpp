/*
 * Example code for serial communication on STM32
 */

#include <cstring>
#include "escheduler.h"
#include "eserial.h"
#include "etick.h"
#include "stm32f1xx_hal.h"

static void SystemClock_Config(void);

class SerialTask : public ETask
{
public:
    SerialTask(ESerial& serial) :
        m_serial(serial),
        m_i(0)
    { }

    virtual void run()
    {
    E_BEGIN_TASK

        if (!m_serial.setSerialConfig(115200, 8))
        {
            //TODO: Error
        }

        if (!m_serial.open())
        {
            //TODO: Error
        }

        for (m_i=0; m_i<100; m_i++)
        {
            {
                const char* send_str = "AT\r\n";
                int bytesWritten =
                    m_serial.write(send_str, strlen(send_str));
            }

            E_REENTER_COND_DELAY(m_serial.bytesAvailable(), 100);

            {
                char buf[32];
                m_serial.read(buf, 31);
            }

            E_REENTER_DELAY(500);
        }

        m_serial.close();

    E_END_TASK
    }

private:
    EStm32Uart& m_serial;
    int m_i;
};

int main(int argc, char * argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    ESerial serial;
    SerialTask task(serial);

    ETask* taskList[] = {&task, dynamic_cast<ETask*>(&serial), NULL};

    EScheduler s(&eTickFunction, taskList);
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
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
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
        EStm32Uart* instance = EStm32Uart::getInstance(USART3);
        if (instance)
            instance->handleInterrupt();
    }
}
