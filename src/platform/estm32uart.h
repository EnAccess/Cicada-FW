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

#include "stm32f1xx_hal.h"
#include "ebufferedserial.h"

class EStm32Uart : public EBufferedSerial
{
public:
    EStm32Uart(USART_TypeDef* uartInstance,
               GPIO_TypeDef* txPort = GPIOC, uint16_t txPin = GPIO_PIN_10,
               GPIO_TypeDef* rxPort = GPIOC, uint16_t rxPin = GPIO_PIN_11);

    static EStm32Uart* getInstance(USART_TypeDef* uartInstance);

    bool open();
    bool isOpen();
    bool setSerialConfig(uint32_t baudRate, uint8_t dataBits);
    void close();
    const char* portName();
    uint16_t write(const char* data, uint16_t size);
    void write(char data);
    bool rawRead(uint8_t& data);
    bool rawWrite(uint8_t data);
    uint16_t rawBytesAvailable();

private:
    // Private constructors to avoid copying
    EStm32Uart(const EStm32Uart&);
    EStm32Uart& operator=(const EStm32Uart&);

    void handleInterrupt();

    static EStm32Uart* instance[E_MULTITON_MAX_INSTANCES];

    uint8_t _flags;
    UART_HandleTypeDef _handle;
    GPIO_TypeDef* _txPort;
    GPIO_TypeDef* _rxPort;
    uint16_t _txPin;
    uint16_t _rxPin;
    IRQn_Type _uartInterruptInstance;
};

#endif
