# EnAccess cellular library

The EnAccess library is an easy to use embedded library for IOT communication using a
cellular modem. It's focus is in transmission of MQTT packages using the
Eclipse Paho MQTT Embedded C/C++ library, but it can be used for general
IP communication as well. The library is platform agnostic and runs either on a
bare metal microcontroller, as well as on top of an embedded OS.
Dialing up the cellular modem, opening an IP channel and sending a MQTT packet
can be done in less than 20 lines of code.

It's easy to add support for a new microcontroller or embedded os. There is also
support for Unix (Linux, OS X) to test code on a PC without to need of having
access to actual microcontroller hardware.

## Supported microcontrollers:
- STM32F1 with HAL

## Supported cellular modems:
- Simcom SIM7x00
