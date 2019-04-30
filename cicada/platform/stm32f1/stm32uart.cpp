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

#include "cicada/platform/stm32f1/stm32uart.h"
#include "cicada/irq.h"
#include <cstdint>

using namespace EnAccess;

#define FLAG_ISOPEN (1 << 0)

Stm32Uart* Stm32Uart::instance[E_MULTITON_MAX_INSTANCES] = { NULL };

Stm32Uart::Stm32Uart(
    USART_TypeDef* uartInstance, GPIO_TypeDef* uartPort, uint16_t txPin, uint16_t rxPin) :
    _flags(0),
    _handle(),
    _uartPort(uartPort),
    _txPin(txPin),
    _rxPin(rxPin),
    _uartInterruptInstance()
{
    _handle.Instance = uartInstance;
    _handle.Init.BaudRate = 115200;
    _handle.Init.WordLength = UART_WORDLENGTH_8B;

    for (int i = 0; i < E_MULTITON_MAX_INSTANCES; i++) {
        if (instance[i] == NULL) {
            instance[i] = this;
            break;
        }
    }
}

Stm32Uart* Stm32Uart::getInstance(USART_TypeDef* uartInstance)
{
    for (int i = 0; i < E_MULTITON_MAX_INSTANCES; i++) {
        Stm32Uart* uart = instance[i];
        if (uart != NULL && uart->_handle.Instance == uartInstance) {
            return instance[i];
        }
    }

    return NULL;
}

bool Stm32Uart::setSerialConfig(uint32_t baudRate, uint8_t dataBits)
{
    if (baudRate < 50 || baudRate > 4500000)
        return false;

    _handle.Init.BaudRate = baudRate;

    switch (dataBits) {
    case 8:
        _handle.Init.WordLength = UART_WORDLENGTH_8B;
        break;
    case 9:
        _handle.Init.WordLength = UART_WORDLENGTH_9B;
        break;
    default:
        return false;
    }

    return true;
}

bool Stm32Uart::open()
{
    // Enable USART/UART Clock
    if (_handle.Instance == USART1) {
        __HAL_RCC_USART1_CLK_ENABLE();
        _uartInterruptInstance = USART1_IRQn;
    } else if (_handle.Instance == USART2) {
        __HAL_RCC_USART2_CLK_ENABLE();
        _uartInterruptInstance = USART2_IRQn;
    } else if (_handle.Instance == USART3) {
        __HAL_RCC_USART3_CLK_ENABLE();
        _uartInterruptInstance = USART3_IRQn;
#ifdef UART4
    } else if (_handle.Instance == UART4) {
        __HAL_RCC_UART4_CLK_ENABLE();
        _uartInterruptInstance = UART4_IRQn;
#endif
#ifdef UART5
    } else if (_handle.Instance == UART5) {
        __HAL_RCC_UART5_CLK_ENABLE();
        _uartInterruptInstance = UART5_IRQn;
#endif
    } else {
        return false;
    }

    // Enable GPIO Clock
    if (_uartPort == GPIOA)
        __HAL_RCC_GPIOA_CLK_ENABLE();
    else if (_uartPort == GPIOB)
        __HAL_RCC_GPIOB_CLK_ENABLE();
    else if (_uartPort == GPIOC)
        __HAL_RCC_GPIOC_CLK_ENABLE();
    else if (_uartPort == GPIOD)
        __HAL_RCC_GPIOD_CLK_ENABLE();
    else if (_uartPort == GPIOE)
        __HAL_RCC_GPIOE_CLK_ENABLE();
#ifdef GPIOF
    else if (_uartPort == GPIOF)
        __HAL_RCC_GPIOF_CLK_ENABLE();
#endif
#ifdef GPIOG
    else if (_uartPort == GPIOG)
        __HAL_RCC_GPIOG_CLK_ENABLE();
#endif

    // Configure GPIO pins
    GPIO_InitTypeDef gpio = { 0 };
    gpio.Pin = _txPin;
    gpio.Mode = GPIO_MODE_AF_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(_uartPort, &gpio);

    gpio.Pin = _rxPin;
    gpio.Mode = GPIO_MODE_INPUT;
    HAL_GPIO_Init(_uartPort, &gpio);

    // Configure UART
    _handle.Init.StopBits = UART_STOPBITS_1;
    _handle.Init.Parity = UART_PARITY_NONE;
    _handle.Init.Mode = UART_MODE_TX_RX;
    _handle.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    _handle.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&_handle) != HAL_OK)
        return false;

    // Configure interrupt
    NVIC_SetPriority(_uartInterruptInstance, E_INTERRUPT_PRIORITY);
    NVIC_EnableIRQ(_uartInterruptInstance);
    SET_BIT(_handle.Instance->CR1, USART_CR1_RXNEIE);

    _flags |= FLAG_ISOPEN;

    return true;
}

void Stm32Uart::close()
{
    NVIC_DisableIRQ(_uartInterruptInstance);
    HAL_UART_DeInit(&_handle);
    HAL_GPIO_DeInit(_uartPort, _txPin);
    HAL_GPIO_DeInit(_uartPort, _rxPin);

    _flags &= ~FLAG_ISOPEN;
}

bool Stm32Uart::isOpen()
{
    return _flags & FLAG_ISOPEN;
}

const char* Stm32Uart::portName() const
{
    return NULL;
}

bool Stm32Uart::rawRead(uint8_t& data)
{
    if (__HAL_UART_GET_FLAG(&_handle, UART_FLAG_RXNE)) {
        data = (uint8_t)READ_REG(_handle.Instance->DR);
        return true;
    }

    return false;
}

bool Stm32Uart::rawWrite(uint8_t data)
{
    if (__HAL_UART_GET_FLAG(&_handle, UART_FLAG_TXE)) {
        WRITE_REG(_handle.Instance->DR, (uint16_t)data);
        return true;
    }

    return false;
}

void Stm32Uart::startTransmit()
{
    SET_BIT(_handle.Instance->CR1, USART_CR1_TXEIE);
}

void Stm32Uart::handleInterrupt()
{
    transferToAndFromBuffer();

    if (!_writeBuffer.availableData())
        CLEAR_BIT(_handle.Instance->CR1, USART_CR1_TXEIE);
}
