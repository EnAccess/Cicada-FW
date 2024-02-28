/*
 * Example fetching a website using a loop
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

// Forward declaration
static void SystemClock_Config(void);

int main(int argc, char* argv[])
{
    // System configuration
    HAL_Init();
    SystemClock_Config();

    HAL_Delay(2000);

    // Function variables
    IPCommDevice* commDevice = nullptr;
    bool detectedFlag = false;
    bool connectedFlag = false;

    // Debug UART driver buffers
    const uint16_t serialBufferSize = 1504;
    char debugReadBuffer[serialBufferSize];
    char debugWriteBuffer[serialBufferSize];

    // UART driver buffers
    char serialReadBuffer[serialBufferSize];
    char serialWriteBuffer[serialBufferSize];

    // Modem driver buffers
    const uint16_t commBufferSize = 1200;
    uint8_t commReadBuffer[commBufferSize];
    uint8_t commWriteBuffer[commBufferSize];

    // Driver objects
    Stm32Uart debug(debugReadBuffer, debugWriteBuffer, serialBufferSize);
    Stm32Uart serial(serialReadBuffer, serialWriteBuffer, serialBufferSize,
                     USART1, GPIOA, GPIO_PIN_9, GPIO_PIN_10);
    ModemDetect detector(serial);

    // Cicada scheduler to run internal tasks
    Task* taskList[] = { &detector, NULL };
    Scheduler cicadaTasks(&eTickFunction, taskList);

    debug.open();

    detector.startDetection();

    // Main event loop
    while (true) {

        // Periodically run Cicada's internal tasks
        cicadaTasks.runTask();

        // Event: Modem has been detected
        if (!detectedFlag && detector.modemDetected()) {
            commDevice = detector.getDetectedModem(commReadBuffer, commWriteBuffer, commBufferSize, commBufferSize);

            // If the device is a SIMCom 2G/4G modem, set the APN
            SimCommDevice* simCommDev = dynamic_cast<SimCommDevice*>(commDevice);
            if (simCommDev) {
                printf("Detected SIMCom 2G/4G modem\r\n");
                simCommDev->setApn("iot-eu.aer.net");
            }

            // If the device is a Espressif WiFi module, set SSID and password
            EspressifDevice* espressifDev = dynamic_cast<EspressifDevice*>(commDevice);
            if (espressifDev) {
                printf("Detected Espressif WiFi module\r\n");
                espressifDev->setSSID("your_ssid");
                espressifDev->setPassword("your_pass");
            }

            // Set host and port to connect to
            commDevice->setHostPort("wttr.in", 80);

            // Ask to modem driver to connect
            commDevice->connect();

            detectedFlag = true;
        }

        // Event: Modem connected to peer
        if (!connectedFlag && commDevice && commDevice->isConnected()) {
            printf("*** Connected! ***\r\n");

            // Send HTTP request to web server
            const char str[] = "GET / HTTP/1.1\r\n"
                               "Host: wttr.in\r\n"
                               "User-Agent: curl\r\n"
                               "Connection: close\r\n\r\n";
            commDevice->write((uint8_t*)str, sizeof(str) - 1);

            connectedFlag = true;
        }

        // Event: Data received from the webserver
        if (commDevice && commDevice->bytesAvailable()) {
            // Read received data and print them on the terminal
            char buf[41];
            uint16_t bytesRead = commDevice->read((uint8_t*)buf, 40);
            for (int i = 0; i < bytesRead; i++) {
                if (buf[i] == '\n') {
                    _putchar('\r');
                }
                _putchar(buf[i]);
            }
        }

        // Event: Modem back to idle state (disconnected)
        if (connectedFlag && commDevice && commDevice->isIdle()) {
            printf("*** Disconnected ***\r\n");
            // Exit main event loop
            break;
        }
    }

    // Wait until reset by user
    while(true);
}

/* Following code is to setup micro controller and handle interrupts */

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
