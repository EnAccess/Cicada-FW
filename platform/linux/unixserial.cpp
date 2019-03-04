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

#include <termios.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "unixserial.h"

UnixSerial::UnixSerial(const char* port) :
    _isOpen(false),
    _port(port),
    _fd(-1),
    _speed(B115200),
    _dataBits(CS8)
{
}

bool UnixSerial::open()
{
    struct termios config;

    _fd = ::open(_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (_fd == -1)
    {
        return false;
    }

    if (!isatty(_fd))
    {
        close();
        return false;
    }

        if(tcgetattr(_fd, &config) < 0)
    {
        return false;
    }

    // Configuration example taken from
    // https://en.wikibooks.org/wiki/Serial_Programming/termios
    config.c_iflag &= ~(IGNBRK | BRKINT | ICRNL |
                        INLCR | PARMRK | INPCK | ISTRIP | IXON);
    config.c_oflag = 0;
    config.c_lflag &= ~(ECHO | ECHONL | ICANON | IEXTEN | ISIG);
    config.c_cflag &= ~(CSIZE | PARENB);
    config.c_cflag |= _dataBits;
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    if(cfsetispeed(&config, _speed) < 0 || cfsetospeed(&config, _speed) < 0)
    {
        return false;
    }

    if(tcsetattr(_fd, TCSAFLUSH, &config) < 0)
    {
        return false;
    }

    _isOpen = true;
    return true;
}

bool UnixSerial::setSerialConfig(uint32_t baudRate, uint8_t dataBits)
{
    switch(baudRate)
    {
    case 0:
        _speed = B0;
        break;
    case 50:
        _speed = B50;
        break;
    case 75:
        _speed = B75;
        break;
    case 110:
        _speed = B110;
        break;
    case 134:
        _speed = B134;
        break;
    case 150:
        _speed = B150;
        break;
    case 200:
        _speed = B200;
        break;
    case 300:
        _speed = B300;
        break;
    case 600:
        _speed = B600;
        break;
    case 1200:
        _speed = B1200;
        break;
    case 1800:
        _speed = B1800;
        break;
    case 2400:
        _speed = B2400;
        break;
    case 9600:
        _speed = B9600;
        break;
    case 19200:
        _speed = B19200;
        break;
    case 38400:
        _speed = B38400;
        break;
    case 57600:
        _speed = B57600;
        break;
    case 115200:
        _speed = B115200;
        break;
    case 230400:
        _speed = B230400;
        break;
    case 460800:
        _speed = B460800;
        break;
    case 576000:
        _speed = B576000;
        break;
    case 921600:
        _speed = B921600;
        break;
    case 1000000:
        _speed = B1000000;
        break;
    case 1152000:
        _speed = B1152000;
        break;
    case 1500000:
        _speed = B1500000;
        break;
    case 2000000:
        _speed = B2000000;
        break;
    case 2500000:
        _speed = B2500000;
        break;
    case 3000000:
        _speed = B3000000;
        break;
    case 3500000:
        _speed = B3500000;
        break;
    case 4000000:
        _speed = B4000000;
        break;
    default:
        return false;
    }

    switch (dataBits)
    {
    case 5:
        _dataBits = CS5;
        break;
    case 6:
        _dataBits = CS6;
        break;
    case 7:
        _dataBits = CS7;
        break;
    case 8:
        _dataBits = CS8;
        break;
    default:
        return false;
    }

    return true;
}

void UnixSerial::close()
{
    _isOpen = false;

    if (_fd >= 0)
        ::close(_fd);

    _fd = -1;
}

uint16_t UnixSerial::rawBytesAvailable() const
{
    if (_fd < 0)
        return 0;

    int bytes;
    ioctl(_fd, FIONREAD, &bytes);

    return bytes > UINT16_MAX ? UINT16_MAX : bytes;
}

bool UnixSerial::rawRead(uint8_t& data)
{
    return ::read(_fd, &data, 1) == 1;
}

bool UnixSerial::rawWrite(uint8_t data)
{
    return ::write(_fd, &data, 1) == 1;
}
