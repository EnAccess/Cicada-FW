#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"

TEST_GROUP(FirstTest){};

TEST(FirstTest, ShouldRunFirstUnitTest)
{
    printf("Unit tests running...");
    
    bool success = true;

    CHECK_TRUE(success); 
}
