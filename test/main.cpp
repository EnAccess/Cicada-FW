#include "CppUTest/CommandLineTestRunner.h"
#include "CppUTest/TestRegistry.h"
#include "CppUTestExt/MockSupportPlugin.h"

int main(int ac, char** av)
{
    // Install the MockPlugin to take care of checkExpectations() & clean() after each test
    MockSupportPlugin mockPlugin;
    TestRegistry::getCurrentRegistry()->installPlugin(&mockPlugin);

    printf("\r\n");
    return CommandLineTestRunner::RunAllTests(ac, av);
}