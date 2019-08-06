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

#include "cicada/commdevices/ipcommdevice.h"
#include <cstddef>

using namespace Cicada;

IPCommDevice::IPCommDevice() :
    _host(NULL),
    _port(0),
    _stateBooleans(LINE_READ),
    _connectState(notConnected),
    _waitForReply(NULL)
{}

void IPCommDevice::setHostPort(IPCommDevice::ConnectionType type, const char* host, uint16_t port)
{
    _host = host;
    _port = port;
    _type = type;
}

bool IPCommDevice::connect()
{
    if (_host == NULL || _port == 0)
        return false;

    _stateBooleans |= CONNECT_PENDING;

    return true;
}

void IPCommDevice::disconnect()
{
    _stateBooleans |= DISCONNECT_PENDING;
}

bool IPCommDevice::isConnected()
{
    return _connectState == connected || _connectState == transmitting;
}

bool IPCommDevice::isIdle()
{
    return _connectState == notConnected;
}

Size IPCommDevice::bytesAvailable() const
{
    return _readBuffer.bytesAvailable();
}

Size IPCommDevice::spaceAvailable() const
{
    if (_connectState != connected)
        return 0;

    return _writeBuffer.spaceAvailable();
}

Size IPCommDevice::read(uint8_t* data, Size maxSize)
{
    return _readBuffer.pull(data, maxSize);
}

Size IPCommDevice::write(const uint8_t* data, Size size)
{
    if (_connectState != connected)
        return 0;

    return _writeBuffer.push(data, size);
}
