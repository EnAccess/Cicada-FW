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
#include "etermios.h"

ETermios::ETermios(const char* port) :
    m_isOpen(false),
    m_port(port),
    m_fd(-1)
{
}

bool ETermios::open()
{
    m_fd = ::open(m_port, O_RDWR | O_NOCTTY | O_NDELAY);
    if (m_fd == -1)
    {
        return false;
    }

    if (!isatty(m_fd))
    {
        close();
        return false;
    }

    m_isOpen = true;
    return true;
}

bool ETermios::setSerialConfig(uint32_t baudRate, uint8_t dataBits)
{
    struct termios config;
    speed_t speed;
    tcflag_t bits;

    switch(baudRate)
    {
    case 0:
        speed = B0;
        break;
    case 50:
        speed = B50;
        break;
    case 75:
        speed = B75;
        break;
    case 110:
        speed = B110;
        break;
    case 134:
        speed = B134;
        break;
    case 150:
        speed = B150;
        break;
    case 200:
        speed = B200;
        break;
    case 300:
        speed = B300;
        break;
    case 600:
        speed = B600;
        break;
    case 1200:
        speed = B1200;
        break;
    case 1800:
        speed = B1800;
        break;
    case 2400:
        speed = B2400;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 57600:
        speed = B57600;
        break;
    case 115200:
        speed = B115200;
        break;
    case 230400:
        speed = B230400;
        break;
    case 460800:
        speed = B460800;
        break;
    case 576000:
        speed = B576000;
        break;
    case 921600:
        speed = B921600;
        break;
    case 1000000:
        speed = B1000000;
        break;
    case 1152000:
        speed = B1152000;
        break;
    case 1500000:
        speed = B1500000;
        break;
    case 2000000:
        speed = B2000000;
        break;
    case 2500000:
        speed = B2500000;
        break;
    case 3000000:
        speed = B3000000;
        break;
    case 3500000:
        speed = B3500000;
        break;
    case 4000000:
        speed = B4000000;
        break;
    default:
        return false;
    }

    switch (dataBits)
    {
    case 5:
        bits = CS5;
        break;
    case 6:
        bits = CS6;
        break;
    case 7:
        bits = CS7;
        break;
    case 8:
        bits = CS8;
        break;
    default:
        return false;
    }

    if(tcgetattr(m_fd, &config) < 0)
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
    config.c_cflag |= bits;
    config.c_cc[VMIN]  = 1;
    config.c_cc[VTIME] = 0;

    if(cfsetispeed(&config, speed) < 0 || cfsetospeed(&config, speed) < 0)
    {
        return false;
    }

    if(tcsetattr(m_fd, TCSAFLUSH, &config) < 0)
    {
        return false;
    }

    return true;
}

void ETermios::close()
{
    m_isOpen = false;

    if (m_fd >= 0)
        ::close(m_fd);

    m_fd = -1;
}

uint16_t ETermios::rawBytesAvailable() const
{
    if (m_fd < 0)
        return 0;

    int bytes;
    ioctl(m_fd, FIONREAD, &bytes);

    return bytes > UINT16_MAX ? UINT16_MAX : bytes;
}

uint16_t ETermios::rawRead(uint8_t* data, uint16_t maxSize)
{
    ssize_t nBytes = ::read(m_fd, data, maxSize);
    if (nBytes < 0)
        return 0;

    return nBytes;
}

uint16_t ETermios::rawWrite(const uint8_t* data, uint16_t size)
{
    return ::write(m_fd, data, size);
}
