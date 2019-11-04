#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

#include <string.h>

#include "cicada/linecircularbuffer.h"

using namespace Cicada;

TEST_GROUP(LineCircularBufferTest){};

TEST(LineCircularBufferTest, ShouldDetectLineBreaksAndReadIndividualLines)
{
    const uint16_t bufferSize = 255;
    char rawBuffer[bufferSize];
    LineCircularBuffer buffer(rawBuffer, bufferSize);

    const char* line1 = "Hello world!\n";
    const char* line2 = "Another line\nYet another line\n";

    buffer.push(line1, strlen(line1));
    buffer.push(line2, strlen(line2));

    uint8_t linesBeforePull = buffer.numBufferedLines();
    for (int i = 0; i < 20; i++)
        buffer.pull();
    uint8_t linesAfterPull = buffer.numBufferedLines();
    char pulledLine[20];
    buffer.readLine(pulledLine, 20);
    int pulledLineLength = buffer.readLine(pulledLine, 20);
    pulledLine[pulledLineLength] = '\0';

    CHECK_EQUAL(linesBeforePull, 3);
    CHECK_EQUAL(linesAfterPull, 2);
    CHECK_EQUAL(buffer.numBufferedLines(), 0);
    STRCMP_EQUAL(pulledLine, "Yet another line\n");
}
