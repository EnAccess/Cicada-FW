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

#include <cstdint>
#include "eblockingcommdev.h"

EBlockingCommDevice::EBlockingCommDevice(EICommDevice& dev,
                                         E_TICK_TYPE (*tickFunction)(void),
                                         void (*_yieldFunction)(void)) :
    _commDev(dev),
    _tickFunction(tickFunction)
{ }

int EBlockingCommDevice::read(unsigned char* buffer, int len, int timeout)
{
    E_TICK_TYPE startTime = _tickFunction();

    while (_commDev.bytesAvailable() < len)
    {
        if (_tickFunction() - startTime > (E_TICK_TYPE)timeout)
            return 0;

        _yieldFunction();
    }

    return _commDev.read(buffer, len);
}

int EBlockingCommDevice::write(unsigned char* buffer, int len, int timeout)
{
    E_TICK_TYPE startTime = _tickFunction();

    while (_commDev.spaceAvailable() < len)
    {
        if (_tickFunction() - startTime > (E_TICK_TYPE)timeout)
            return 0;

        _yieldFunction();
    }

    return _commDev.write(buffer, len);
}
