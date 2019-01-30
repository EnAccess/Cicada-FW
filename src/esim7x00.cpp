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

#include <cstddef>
#include <cstdint>
#include <cstring>
#include "esim7x00.h"

#define LINE_MAX_LENGTH 60

#define OK_STR_LENGTH        2
#define LINE_END_STR_LENGTH  2
#define QUOTE_END_STR_LENGTH 3
const char* ESim7x00CommDevice::m_okStr = "OK";
const char* ESim7x00CommDevice::m_lineEndStr = "\r\n";
const char* ESim7x00CommDevice::m_quoteEndStr = "\"\r\n";

ESim7x00CommDevice::ESim7x00CommDevice(EDefaultBufferedSerial& serial) :
    m_serial(serial),
    m_connectState(notConnected),
    m_replyState(normalReply),
    m_apn(NULL),
    m_host(NULL),
    m_port(0),
    m_waitForReply(NULL),
    m_lineRead(true),
    m_ipConnected(false),
    m_bytesToReceive(0),
    m_bytesToRead(0)
{ }

void ESim7x00CommDevice::setHostPort(const char* host, uint16_t port)
{
    m_host = host;
    m_port = port;
}

void ESim7x00CommDevice::setApn(const char* apn)
{
    m_apn = apn;
}

bool ESim7x00CommDevice::connect()
{
    if (m_connectState != notConnected || m_apn == NULL ||
        m_host == NULL || m_port == 0)
    {
        return false;
    }

    m_connectState = connecting;

    return true;
}

bool ESim7x00CommDevice::disconnect()
{
    if (m_connectState < connected)
        return false;

    m_connectState = sendNetclose;
    return true;
}

bool ESim7x00CommDevice::isConnected()
{
    return m_connectState >= connected &&
        m_connectState < sendNetclose && m_ipConnected;
}

bool ESim7x00CommDevice::isIdle()
{
    return m_connectState == notConnected;
}

uint16_t ESim7x00CommDevice::bytesAvailable() const
{
    if (m_connectState != receiving)
        return 0;

    if (m_bytesToRead > m_serial.bytesAvailable())
        return m_serial.bytesAvailable();
    else
        return m_bytesToRead;
}

uint16_t ESim7x00CommDevice::read(uint8_t* data, uint16_t maxSize)
{
    if (m_connectState != receiving)
        return 0;

    if (maxSize > m_bytesToRead)
        maxSize = m_bytesToRead;

    uint16_t bytesRead = m_serial.read((char*)data, maxSize);
    m_bytesToRead -= bytesRead;

    if (m_bytesToRead == 0)
    {
        m_lineRead = true;

        if (m_bytesToReceive == 0)
        {
            m_connectState = sendCiprxget4;
        }
        else
        {
            m_connectState = sendCiprxget2;
        }
    }

    return bytesRead;
}

uint16_t ESim7x00CommDevice::write(const uint8_t* data, uint16_t size)
{
    const char minSpace = 22;

    if (m_connectState != connected)
        return 0;

    if (m_serial.spaceAvailable() < minSpace)
        return 0;

    if (size > m_serial.spaceAvailable() - minSpace + 1)
        size = m_serial.spaceAvailable() - minSpace + 1;

    m_connectState = sendCipsend;
    m_lineRead = false;

    const char str[] = "AT+CIPSEND=0,";
    char sizeStr[6];

    sprintf(sizeStr, "%d", size);

    m_serial.write(str, sizeof(str) - 1);
    m_serial.write(sizeStr, strlen(sizeStr));
    m_serial.write(m_lineEndStr, LINE_END_STR_LENGTH);
    m_serial.setWriteBarrier();

    return m_serial.write((char*)data, size);
}

#define SEND_COMMAND(cmd, expectedReply, nextState)             \
    {                                                           \
        const char sendStr[] = cmd;                             \
        m_serial.write(sendStr, sizeof(sendStr) - 1);           \
        m_serial.write(m_lineEndStr, LINE_END_STR_LENGTH);      \
        m_waitForReply = expectedReply;                         \
        m_connectState = nextState;                             \
        break;                                                  \
    }

void ESim7x00CommDevice::run()
{
    // If the serial device is net yet open, try to open it
    if (!m_serial.isOpen())
    {
        if (!m_serial.open())
        {
            m_connectState = serialError;
        }
        return;
    }

    // Check if there is a reply from the modem
    if (m_lineRead && m_serial.canReadLine())
    {
        char data[LINE_MAX_LENGTH + 1];
        uint8_t size = m_serial.readLine(data, LINE_MAX_LENGTH);
        data[size] = '\0';

        if (m_connectState < connected || m_connectState > receiving)
        {
            if (m_waitForReply)
                printf("m_connectState=%d, m_replyState=%d, "
                       "m_waitForReply=\"%s\", data: %s",
                       m_connectState, m_replyState, m_waitForReply, data);
            else
                printf("m_connectState=%d, m_replyState=%d, "
                       "m_waitForReply=NULL, data: %s",
                       m_connectState, m_replyState, data);
        }

        // If we sent a command, we process the reply here
        if (m_waitForReply)
        {
            if (strncmp(data, m_waitForReply, strlen(m_waitForReply)) == 0)
            {
                m_replyState = normalReply;
                m_waitForReply = NULL;
            }
            else if (strncmp(data, "ERROR", 5) == 0)
            {
                m_connectState = modemError;
                return;
            }
        }

        switch(m_replyState)
        {
        case netopen:
            if (strncmp(data, "+NETOPEN: 1", 11) == 0)
            {
                // Send netopen again in case it didn't work
                setDelay(2000);
                m_connectState = sendNetopen;
                m_waitForReply = NULL;
            }
            break;

        case cdnsgip:
            if (strncmp(data, "+CDNSGIP: 1", 11) == 0)
            {
                m_replyState = normalReply;

                char* tmpStr;
                uint8_t i = 0, q = 0;

                // Validate DNS reply string
                while (data[i])
                {
                    if (data[i++] == '\"')
                        q++;
                }
                if (q != 4)
                {
                    // Error in input string
                    m_connectState = dnsError;
                    return;
                }
                i = 0, q = 0;

                // Parse IP address
                while(q < 3)
                    if (data[i++] == '\"')
                        q++;
                tmpStr = data + i;
                while(data[i])
                {
                    if (data[i] == '\"')
                        data[i] = '\0';
                    i++;
                }
                strcpy(m_ip, tmpStr);
            }
            else if (strncmp(data, "+CDNSGIP: 0", 11) == 0)
            {
                m_connectState = dnsError;
                return;
            }
            break;

        case waitForData:
            if (strncmp(data, "+CIPRXGET: 1,0", 14) == 0)
            {
                m_connectState = sendCiprxget4;
            }
            break;

        case ciprxget4:
            if (strncmp(data, "+CIPRXGET: 4,0,", 15) == 0)
            {
                int bytesToReceive;
                sscanf(data + 15, "%d", &bytesToReceive);
                m_bytesToReceive += bytesToReceive;
                if (m_bytesToReceive > 0)
                {
                    m_replyState = normalReply;
                    m_connectState = sendCiprxget2;
                }
                else if (m_ipConnected)
                {
                    m_replyState = waitForData;
                    m_connectState = connected;
                }
                else
                {
                    m_replyState = noReply;
                    m_connectState = sendCipopen;
                }
            }
            break;

        case ciprxget2:
            if (strncmp(data, "+CIPRXGET: 2,0,", 15) == 0)
            {
                int bytesToReceive;
                sscanf(data + 15, "%d", &bytesToReceive);
                m_bytesToReceive -= bytesToReceive;
                m_bytesToRead += bytesToReceive;
                m_lineRead = false;
                m_replyState = normalReply;
                m_connectState = receiving;
            }
            break;

        default:
            break;
        }

        // In connected state, check for connection close
        if (m_connectState >= connected)
        {
            if (strncmp(data, "+IPCLOSE: 0,", 12) == 0)
            {
                m_ipConnected = false;
            }
        }
    }

    // When disconnecting was requested, flush read buffer first
    else if (m_connectState == sendNetclose)
    {
        while (m_bytesToRead && m_serial.bytesAvailable())
        {
            m_serial.read();
            m_bytesToRead--;
        }
        m_bytesToReceive = 0;

        if (m_bytesToRead == 0)
            m_lineRead = true;
    }

    // Don't go on if we are in wait state
    if (m_waitForReply)
        return;

    // Don't go on if space in write buffer is low
    if (m_serial.spaceAvailable() < 20)
        return;

    // Connection state machine
    switch(m_connectState)
    {

    case connecting:
        setDelay(10);
        SEND_COMMAND("ATE1", m_okStr, sendCgdcont);

    case sendCgdcont:
    {
        const char str[] = "AT+CGDCONT=1,\"IP\",\"";
        m_serial.write(str, sizeof(str) - 1);
        m_serial.write(m_apn, strlen(m_apn));
        m_serial.write(m_quoteEndStr, QUOTE_END_STR_LENGTH);

        m_waitForReply = m_okStr;
        m_connectState = sendAtd;
        break;
    }

    case sendAtd:
        setDelay(1000);
        SEND_COMMAND("ATD*99#", "CONNECT", sendPpp);

    case sendPpp:
    {
        const char str[] = "+++";
        m_serial.write(str, sizeof(str) - 1);

        m_waitForReply = m_okStr;
        m_connectState = sendCsocksetpn;
        break;
    }

    case sendCsocksetpn:
        setDelay(10);
        SEND_COMMAND("AT+CSOCKSETPN=1", m_okStr, sendCipmode);

    case sendCipmode:
        SEND_COMMAND("AT+CIPMODE=0", m_okStr, sendNetopen);

    case sendNetopen:
        m_replyState = netopen;
        SEND_COMMAND("AT+NETOPEN", "+NETOPEN: 0", sendCiprxget);

    case sendCiprxget:
        setDelay(10);
        SEND_COMMAND("AT+CIPRXGET=1", m_okStr, sendDnsQuery);

    case sendDnsQuery:
    {
        if (m_serial.spaceAvailable() < strlen(m_host) + 20)
            break;
        m_replyState = cdnsgip;
        const char str[] = "AT+CDNSGIP=\"";
        m_serial.write(str, sizeof(str) - 1);
        m_serial.write(m_host, strlen(m_host));
        m_serial.write(m_quoteEndStr, QUOTE_END_STR_LENGTH);

        m_waitForReply = m_okStr;
        m_connectState = sendCipopen;
        break;
    }

    case sendCipopen:
    {
        const char str[] = "AT+CIPOPEN=0,\"TCP\",\"";
        const char midStr[] = "\",";
        char portStr[6];

        sprintf(portStr, "%d", m_port);

        m_serial.write(str, sizeof(str) - 1);
        m_serial.write(m_ip, strlen(m_ip));
        m_serial.write(midStr, sizeof(midStr) - 1);
        m_serial.write(portStr, strlen(portStr));
        m_serial.write(m_lineEndStr, LINE_END_STR_LENGTH);

        m_waitForReply = "+CIPOPEN: 0,0";
        m_connectState = finalizeConnect;
        break;
    }

    case finalizeConnect:
        setDelay(0);
        m_replyState = waitForData;
        m_connectState = connected;
        m_ipConnected = true;
        break;

    // States after connecting

    case sendCipsend:
    {
        char data;
        if (m_serial.read(&data, 1) == 1)
        {
            if (data == '>')
            {
                m_serial.clearWriteBarrier();
                m_lineRead = true;
                m_connectState = connected;
            }
        }
        break;
    }

    case sendCiprxget4:
        m_replyState = ciprxget4;
        SEND_COMMAND("AT+CIPRXGET=4,0", m_okStr, sendCiprxget2);

    case sendCiprxget2:
        if (m_bytesToReceive > 0 && m_serial.spaceAvailable() > 4)
        {
            int bytesToReceive = m_serial.spaceAvailable() - 4;
            if (bytesToReceive > m_bytesToReceive)
                bytesToReceive = m_bytesToReceive;
            const char str[] = "AT+CIPRXGET=2,0,";
            char sizeStr[6];
            sprintf(sizeStr, "%d", bytesToReceive);
            m_serial.write(str, sizeof(str) - 1);
            m_serial.write(sizeStr, strlen(sizeStr));
            m_serial.write(m_lineEndStr, LINE_END_STR_LENGTH);
            m_replyState = ciprxget2;
            m_waitForReply = m_okStr;
            m_connectState = waitReceive;
        }
        break;

    case sendNetclose:
        SEND_COMMAND("AT+NETCLOSE", "+NETCLOSE: 0", sendAth);

    case sendAth:
        SEND_COMMAND("ATH", m_okStr, finalizeDisconnect);

    case finalizeDisconnect:
        m_connectState = notConnected;
        break;

    default:
        break;
    }
}
