# Cicada - IoT Communications Module for Energy Access

An easy way to get production ready, bi-directional communications for your 
IoT embedded device. 

This repository contains the source code for UART drivers, 2G, 3G, 4G modems
(and wifi in the future). **This library is platform agnostic, designed to be
portable** - including examples for mbed, FreeRTOS, or bare metal.

Cicada uses the MQTT protocol to connect to the cloud but it can be used for 
general IP communication as well. 

Dialing up the cellular modem, opening an IP channel and sending a MQTT packet
can be done in less than 50 lines of code.

It's easy to add support for a new microcontroller or embedded os. There is also
support for Unix (Linux, OS X) to test code on a PC without the need of having
access to actual microcontroller hardware.

## Supported UARTs:
- STM32F1
- Mbed
- Unix (Linux / OS X) termios

## Supported cellular modems:
- Simcom SIM7x00
- Simcom SIM800

## Documentation
[View the hosted Doxygen here](https://enaccess.github.io/Cicada/doc/).

## Build and test

### Build toolchain
- Meson + Ninja

### Unit tests
- CppUnit

### Build setup
To setup build dependencies, do:  
`git submodule init`  
`git submodule update`  

### Native build (for testing an a host PC):
Run `meson <builddirectory>` to generate build files. Finally, change
to the builddirectory and run `ninja`.

### Cross build (for microcontrollers):
Run `meson <builddirectory> --cross-file <crossfile>`

Example:
`meson stm32build --cross-file stm32.cross.build`

## Getting started
The following code shows a simple example for STM32 which dials up the modem,
connects to a host and sends an MQTT packet:
```
int main(int argc, char* argv[])
{
    // System configuration for microcontroller
    System_Config();

    // Set up serial port
    Stm32Uart serial(UART4, GPIOC);

    // Set up modem driver, replace this with the driver you want
    Sim7x00CommDevice commDev(serial);

    // Set up task scheduler to call the modem driver's run() function
    Task* taskList[] = {&commDev, NULL};
    Scheduler s(&eTickFunction, taskList);

    // Set up MQTT client
    BlockingCommDevice bld(commDev, eTickFunction, yieldFunction, &s);
    MQTT::Client<BlockingCommDevice, MQTTCountdown> client =
        MQTT::Client<BlockingCommDevice, MQTTCountdown>(bld);

    // Dial up modem and connect to IP host
    commDev.setApn("internet");
    commDev.setHostPort("test.mosquitto.org", 1883);
    commDev.connect();
    while (!commDev.isConnected()) {
        yieldFunction(&s);
    }

    // Connect MQTT client
    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char*)"enaccess";
    client.connect(data);

    // Send a message
    MQTT::Message message;
    message.qos = MQTT::QOS0;
    message.payload = (void*)"Hello World!";
    message.payloadlen = 13;
    client.publish("enaccess/test", message);

    // Disconnect everything
    client.disconnect();
    commDev.disconnect();
    while (!commDev.isIdle()) {
        yieldFunction(&s);
    }
}
```
See `examples/` directory for full example code.
