# List of examples

* linux
Examples for systems running GNU/Linux. Most of the examples also run on OS X .

* linux/blocking.cpp
Fetches a website from a server with the blocking API wrapper.

* linux/blockingmqtt.cpp
Connects to an MQTT broker using the blocking API wrapper
and sends/receives different type of messages.

* linux/eventloop.cpp
Fetches a website from a server using the non-blocking API and
a main loop.

* linux/ipcommdevice.cpp
Fetches a website from a server with the standard non-blocking API.
Instead of a custom main loop, it makes use of Cicada's Task API
and it's scheduler.

* linux/scheduler.cpp
Demonstration of the task scheduler.

* linux/serial_linux.cpp
Sends the string "AT" to a serial device and prints the reply.

* stm32f1
Examples for STM32F1 microcontrollers. All examples run on the
Nucleo-F103RB board. The modem used for communication is the
Simcom Sim7600, which should be connected to UART on
GPIO pins 9 and 10.

* stm32f1/blinky.cpp
Just blinks the LED on the Nucleo board.

* stm32f1/blockingmqtt_freertos.cpp
Connects to an MQTT broker and send messages. For task scheduling,
uses FreeRTOS tasks and scheduler.

* stm32f1/blockingmqtt_stm32.cpp
Bare metal application which connects to an MQTT broker and sends messages.

* stm32f1/dualport_freertos.cpp
Sends 'AT' to the USART1 and prints log messages on USART2. For task scheduling,
uses FreeRTOS tasks and scheduler.

* stm32f1/dualport_stm32.cpp
Bare metal application which sends 'AT' to the USART1 and prints
log messages on USART2.

* stm32f1/eventloop.cpp
Bare metal application, which fetches a website from a server using
the non-blocking API and a main loop. This example is a recommended
starting point for many microcontroller applications.

* stm32f1/ipcommdevice_freertos.cpp
Fetches a website from a server. For task scheduling,
uses FreeRTOS tasks and scheduler.

* stm32f1/ipcommdevice_stm32.cpp
Bare metal application, which fetches a website from a server.
It makes use of Cicada's Task API and scheduler.

* stm32f1/mqttsequence.cpp
Sends packets of pseudo-random data. This example is meant for
long term testing of MQTT packat transmission.

* stm32f1/printf.cpp
Sends "Hello world" to the serial port in a loop.

* stm32f1/serial.cpp
Sends the string "AT" to the serial port.

* stm32f1/simplemqtt.cpp
Simplest MQTT sample, which only connects and sends a message
