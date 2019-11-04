#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "cicada/bufferedserial.h"

using namespace Cicada;

TEST_GROUP(BufferedSerialTest)
{
    class BufferedSerialMock : public BufferedSerial
    {
      public:
        BufferedSerialMock() :
            BufferedSerial(_rawReadBuffer, _rawWriteBuffer, 1200),
            _inBufferMock(_rawInBuffer, 120),
            _outBufferMock(_rawOutBuffer, 120)
        {}

        bool open()
        {
            return true;
        }
        void close() {}

        bool isOpen()
        {
            return true;
        }

        bool setSerialConfig(uint32_t baudRate, uint8_t dataBits)
        {
            return true;
        }

        const char* portName() const
        {
            return NULL;
        }

        bool rawRead(uint8_t& data)
        {
            if (!_inBufferMock.isEmpty()) {
                data = _inBufferMock.pull();
                return true;
            }

            return false;
        }

        virtual bool rawWrite(uint8_t data)
        {
            if (!_outBufferMock.isFull()) {
                _outBufferMock.push(data);
                return true;
            }

            return false;
        }

        virtual void startTransmit()
        {
            mock().actualCall("startTransmit");
        }

        char _rawReadBuffer[1200];
        char _rawWriteBuffer[1200];
        char _rawInBuffer[120];
        char _rawOutBuffer[120];
        CircularBuffer<char> _inBufferMock;
        CircularBuffer<char> _outBufferMock;
    };
};

TEST(BufferedSerialTest, ShouldReadDataAfterTransferFromUnderlyingMockSerial)
{
    BufferedSerialMock bs;
    const uint8_t SIZE = 20;
    char dataIn[SIZE] = "123456789 987654321";
    char dataOut[SIZE];

    bs._inBufferMock.push(dataIn, SIZE);

    for (int i = 0; i < 100; i++)
        bs.transferToAndFromBuffer();

    uint8_t readLen = bs.read((uint8_t*)dataOut, SIZE);

    CHECK_EQUAL(SIZE, readLen);
    STRNCMP_EQUAL(dataIn, dataOut, SIZE);
}

TEST(BufferedSerialTest, ShouldHaveDataInMockSerialAfterWrite)
{
    BufferedSerialMock bs;
    const uint8_t SIZE = 20;
    char dataIn[SIZE] = "123456789 987654321";
    char dataOut[SIZE];

    mock().expectOneCall("startTransmit");

    bs.write((uint8_t*)dataIn, SIZE);

    for (int i = 0; i < 100; i++)
        bs.transferToAndFromBuffer();

    bs._outBufferMock.pull(dataOut, SIZE);

    STRNCMP_EQUAL(dataIn, dataOut, SIZE);
}

TEST(BufferedSerialTest, ShouldDetectLineBreaksAndReadIndividualLines)
{
    BufferedSerialMock bs;
    const uint8_t SIZE = 21;
    char dataIn[SIZE] = "A line\nAnother line\n";
    char dataOut[SIZE];

    bs._inBufferMock.push(dataIn, SIZE);

    for (int i = 0; i < 100; i++)
        bs.transferToAndFromBuffer();

    int outLen = bs.readLine((uint8_t*)dataOut, SIZE);
    dataOut[outLen] = '\0';
    STRNCMP_EQUAL("A line\n", dataOut, SIZE);
    outLen = bs.readLine((uint8_t*)dataOut, SIZE);
    dataOut[outLen] = '\0';
    STRNCMP_EQUAL("Another line\n", dataOut, SIZE);
}
