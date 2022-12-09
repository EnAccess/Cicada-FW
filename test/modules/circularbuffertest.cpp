#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include "cicada/circularbuffer.h"

using namespace Cicada;

TEST_GROUP(CircularBufferTest){};

TEST(CircularBufferTest, ShouldPushAndPullDataAsExpected)
{
    const uint8_t MAX_BUFFER_SIZE = 255;
    char rawBuffer[MAX_BUFFER_SIZE];
    CircularBuffer<char> buffer(rawBuffer, MAX_BUFFER_SIZE);

    const uint8_t SIZE = 20;
    char dataIn[SIZE] = "123456789 987654321";
    char dataOut[SIZE];

    uint8_t writeLen = buffer.push(dataIn, SIZE);
    uint8_t readLen = buffer.pull(dataOut, SIZE);
    uint8_t availableData = buffer.bytesAvailable();
    uint8_t availableSpace = buffer.spaceAvailable();
    bool isEmpty = buffer.isEmpty();
    bool isFull = buffer.isFull();

    CHECK_EQUAL(SIZE, writeLen);
    CHECK_EQUAL(SIZE, readLen);
    CHECK_EQUAL(0, availableData);
    CHECK_EQUAL(MAX_BUFFER_SIZE, availableSpace);
    STRNCMP_EQUAL(dataIn, dataOut, SIZE);
    CHECK(isEmpty);
    CHECK_FALSE(isFull);
}

TEST(CircularBufferTest, ShouldTruncateDataIfItDoesntFitInTheBuffer)
{
    const uint8_t MAX_BUFFER_SIZE = 9;
    char rawBuffer[MAX_BUFFER_SIZE];
    CircularBuffer<char> buffer(rawBuffer, MAX_BUFFER_SIZE);

    const uint8_t SIZE = 20;
    char dataIn[SIZE] = "123456789 987654321";
    char dataOut[SIZE];

    char expectedDataOut[] = "123456789";

    uint8_t writeLen = buffer.push(dataIn, SIZE);
    uint8_t readLen = buffer.pull(dataOut, SIZE);

    CHECK_EQUAL(MAX_BUFFER_SIZE, writeLen);
    CHECK_EQUAL(MAX_BUFFER_SIZE, readLen);
    STRNCMP_EQUAL(expectedDataOut, dataOut, MAX_BUFFER_SIZE);
}

TEST(CircularBufferTest, ShouldFillBufferExactly)
{
    const uint8_t SIZE = 20;
    char rawBuffer[SIZE];
    CircularBuffer<char> buffer(rawBuffer, SIZE);

    char dataIn[SIZE] = "123456789 987654321";
    char dataOut[SIZE];

    uint8_t writeLen = buffer.push(dataIn, SIZE);
    uint8_t availableData = buffer.bytesAvailable();
    uint8_t availableSpace = buffer.spaceAvailable();
    uint8_t readLen = buffer.pull(dataOut, SIZE);

    CHECK_EQUAL(SIZE, writeLen);
    CHECK_EQUAL(SIZE, readLen);
    CHECK_EQUAL(SIZE, availableData);
    CHECK_EQUAL(0, availableSpace);
    STRNCMP_EQUAL(dataIn, dataOut, SIZE);
}

TEST(CircularBufferTest, MaximumBufferSize)
{
    const uint16_t SIZE = UINT16_MAX;
    uint8_t rawBuffer[SIZE];
    CircularBuffer<uint8_t> buffer(rawBuffer, SIZE);
    uint8_t dataOut[SIZE];
    uint8_t dataExpected[SIZE];

    for (int i = 0; i < SIZE; i++) {
        dataExpected[i] = i % 255;
        buffer.push(dataExpected[i]);
    }
    uint16_t availableDataBeforeRead = buffer.bytesAvailable();
    uint16_t availableSpaceBeforeRead = buffer.spaceAvailable();
    uint16_t readLen = buffer.pull(dataOut, SIZE);
    uint16_t availableDataAfterRead = buffer.bytesAvailable();
    uint16_t availableSpaceAfterRead = buffer.spaceAvailable();

    CHECK_EQUAL(SIZE, availableDataBeforeRead);
    CHECK_EQUAL(0, availableSpaceBeforeRead);
    CHECK_EQUAL(SIZE, readLen);
    CHECK_EQUAL(0, availableDataAfterRead);
    CHECK_EQUAL(SIZE, availableSpaceAfterRead);
    MEMCMP_EQUAL(dataExpected, dataOut, SIZE);
}

TEST(CircularBufferTest, WriteReadMultipleTimes)
{
    const uint8_t MAX_BUFFER_SIZE = 10;
    char rawBuffer[MAX_BUFFER_SIZE];
    CircularBuffer<char> buffer(rawBuffer, MAX_BUFFER_SIZE);

    const uint8_t SIZE = 7;
    char dataInFirst[SIZE] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G' };
    char dataOutFirst[SIZE];

    char dataInSecond[SIZE] = { '1', '2', '3', '4', '5', '6', '7' };
    char dataOutSecond[SIZE];

    uint8_t writeLen = buffer.push(dataInFirst, SIZE);
    uint8_t availLen = buffer.bytesAvailable();
    uint8_t readLen = buffer.pull(dataOutFirst, SIZE);

    CHECK_EQUAL(SIZE, writeLen);
    CHECK_EQUAL(SIZE, availLen);
    CHECK_EQUAL(SIZE, readLen);
    STRNCMP_EQUAL(dataInFirst, dataOutFirst, SIZE);

    writeLen = buffer.push(dataInSecond, SIZE);
    availLen = buffer.bytesAvailable();
    readLen = buffer.pull(dataOutSecond, SIZE);

    CHECK_EQUAL(SIZE, writeLen);
    CHECK_EQUAL(SIZE, availLen);
    CHECK_EQUAL(SIZE, readLen);
    STRNCMP_EQUAL(dataInSecond, dataOutSecond, SIZE);
}

TEST(CircularBufferTest, ShouldReadNothingAfterFlushingTheBuffer)
{
    const uint8_t MAX_BUFFER_SIZE = 10;
    char rawBuffer[MAX_BUFFER_SIZE];
    CircularBuffer<char> buffer(rawBuffer, MAX_BUFFER_SIZE);

    char expectedDataOut[MAX_BUFFER_SIZE] = {};
    char dataOut[MAX_BUFFER_SIZE] = {};

    const uint8_t SIZE = 5;
    char dataIn[SIZE] = { 'A', 'B', 'C', 'D', 'E' };

    uint8_t writeLen = buffer.push(dataIn, SIZE);

    buffer.flush();

    uint8_t readLen = buffer.pull(dataOut, MAX_BUFFER_SIZE);

    CHECK_EQUAL(SIZE, writeLen);
    CHECK_EQUAL(0, readLen);
    STRNCMP_EQUAL(expectedDataOut, dataOut, MAX_BUFFER_SIZE);
}

TEST(CircularBufferTest, ReReadSameData)
{
    const uint8_t MAX_BUFFER_SIZE = 10;
    char rawBuffer[MAX_BUFFER_SIZE];
    CircularBuffer<char> buffer(rawBuffer, MAX_BUFFER_SIZE);

    const uint8_t SIZE = 8;
    char dataIn[SIZE] = "1234567";
    char dataOut[SIZE];

    buffer.push(dataIn, SIZE);
    buffer.pull(dataOut, SIZE);
    STRNCMP_EQUAL(dataIn, dataOut, SIZE);

    buffer.rewindReadHead(4);
    buffer.pull(dataOut, 4);
    STRNCMP_EQUAL("567", dataOut, 4);
}


TEST(CircularBufferTest, ReReadSameDataWithWrapAround)
{
    const uint8_t MAX_BUFFER_SIZE = 10;
    char rawBuffer[MAX_BUFFER_SIZE];
    CircularBuffer<char> buffer(rawBuffer, MAX_BUFFER_SIZE);

    const uint8_t SIZE = 8;
    char dataIn[SIZE] = "1234567";
    char dataOut[SIZE];

    buffer.push(dataIn, SIZE);
    buffer.pull(dataOut, SIZE);
    STRNCMP_EQUAL(dataIn, dataOut, SIZE);

    buffer.push(dataIn, SIZE);
    buffer.pull(dataOut, SIZE);
    STRNCMP_EQUAL(dataIn, dataOut, SIZE);

    buffer.rewindReadHead(7);
    buffer.pull(dataOut, 7);
    STRNCMP_EQUAL("234567", dataOut, 7);
}
