/*
 * E-Lib
 * Copyright (C) 2019 EnAccess
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#ifndef ESTM32UART_H
#define ESTM32UART_H

#include "bufferedserial.h"
#include "stm32f1xx_hal.h"

namespace EnAccess {

/*!
 * UART driver for STM32 micro controllers, using HAL.
 *
 * *NOTE:* If you use more than one UART port in your program, you need to set
 * the preprocessor define `E_MULTITON_MAX_INSTANCES` to the number of ports
 * used. You can set this macro with the -D compiler argument, for example
 * `-DE_MULTITON_MAX_INSTANCES=2`.
 *
 * In the UART's IRQ handler, get the instance with `Stm32Uart::getInstance()`
 * and call it's `handleInterrupt()` function. Example:
 * ```
 *     void USART3_IRQHandler()
 *     {
 *         static Stm32Uart* instance = Stm32Uart::getInstance(USART3);
 *         instance->handleInterrupt();
 *     }
 * ```
 */

class Stm32Uart : public BufferedSerial
{
  public:
    Stm32Uart(USART_TypeDef* uartInstance = USART2, GPIO_TypeDef* uartPort = GPIOA,
        uint16_t txPin = GPIO_PIN_2, uint16_t rxPin = GPIO_PIN_3);

    static Stm32Uart* getInstance(USART_TypeDef* uartInstance);

    bool open();
    bool isOpen();
    bool setSerialConfig(uint32_t baudRate, uint8_t dataBits);
    void close();
    const char* portName() const;
    uint16_t write(const char* data, uint16_t size);
    void write(char data);
    bool rawRead(uint8_t& data);
    bool rawWrite(uint8_t data);

    void handleInterrupt();

  private:
    // Private constructors to avoid copying
    Stm32Uart(const Stm32Uart&);
    Stm32Uart& operator=(const Stm32Uart&);

    static Stm32Uart* instance[E_MULTITON_MAX_INSTANCES];

    uint8_t _flags;
    UART_HandleTypeDef _handle;
    GPIO_TypeDef* _uartPort;
    uint16_t _txPin;
    uint16_t _rxPin;
    IRQn_Type _uartInterruptInstance;
};
}

#endif
