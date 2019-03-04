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

#include "emqttcountdown.h"

// MQTTCountdown::MQTTCountdown(E_TICK_TYPE (*sysTickHandler)()) :
//     _sysTickHandler(sysTickHandler),
//     _endTime(0)
// {}

// MQTTCountdown::MQTTCountdown(E_TICK_TYPE (*sysTickHandler)(), int ms) :
//     _sysTickHandler(sysTickHandler),
//     _endTime(0)
// {
//     countdown_ms(ms);
// }

MQTTCountdown::MQTTCountdown() :
    _endTime(0)
{}

MQTTCountdown::MQTTCountdown(int ms) :
    _endTime(0)
{
    countdown_ms(ms);
}

bool MQTTCountdown::expired()
{
    return left_ms() == 0;
}

void MQTTCountdown::countdown_ms(int ms)
{
    _endTime = _sysTickHandler() + ms;
}

void MQTTCountdown::countdown(int seconds)
{
    _endTime = _sysTickHandler() + seconds * 1000;
}

int MQTTCountdown::left_ms()
{
    int64_t left = (int64_t)_endTime - _sysTickHandler();
    if (left < 0)
        left = 0;

    return (int)left;
}
