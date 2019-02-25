project(
    'CppUTest',
    'cpp',
    version : '3.8'
)

cpputest_inc = include_directories('src/include')

cpputest_src = files([
    'src/src/CppUTest/CommandLineArguments.cpp',
    'src/src/CppUTest/CommandLineTestRunner.cpp',
    'src/src/CppUTest/JUnitTestOutput.cpp',
    'src/src/CppUTest/MemoryLeakDetector.cpp',
    'src/src/CppUTest/MemoryLeakWarningPlugin.cpp',
    'src/src/CppUTest/SimpleMutex.cpp',
    'src/src/CppUTest/SimpleString.cpp',
    'src/src/CppUTest/TeamCityTestOutput.cpp',
    'src/src/CppUTest/TestFailure.cpp',
    'src/src/CppUTest/TestFilter.cpp',
    'src/src/CppUTest/TestHarness_c.cpp',
    'src/src/CppUTest/TestMemoryAllocator.cpp',
    'src/src/CppUTest/TestOutput.cpp',
    'src/src/CppUTest/TestPlugin.cpp',
    'src/src/CppUTest/TestRegistry.cpp',
    'src/src/CppUTest/TestResult.cpp',
    'src/src/CppUTest/TestTestingFixture.cpp',
    'src/src/CppUTest/Utest.cpp',
])

cpputest_ext_src = files([
    'src/src/CppUTestExt/CodeMemoryReportFormatter.cpp',
    'src/src/CppUTestExt/IEEE754ExceptionsPlugin.cpp',
    'src/src/CppUTestExt/MemoryReportAllocator.cpp',
    'src/src/CppUTestExt/MemoryReporterPlugin.cpp',
    'src/src/CppUTestExt/MemoryReportFormatter.cpp',
    'src/src/CppUTestExt/MockActualCall.cpp',
    'src/src/CppUTestExt/MockExpectedCall.cpp',
    'src/src/CppUTestExt/MockExpectedCallsList.cpp',
    'src/src/CppUTestExt/MockFailure.cpp',
    'src/src/CppUTestExt/MockNamedValue.cpp',
    'src/src/CppUTestExt/MockSupport.cpp',
    'src/src/CppUTestExt/MockSupport_c.cpp',
    'src/src/CppUTestExt/MockSupportPlugin.cpp',
    'src/src/CppUTestExt/OrderedTest.cpp',
])

platform_src = files([
    'src/src/Platforms/Gcc/UtestPlatform.cpp'
])

cpputest_dep = declare_dependency(
    include_directories : [ cpputest_inc ],
	sources             : [ cpputest_src, cpputest_ext_src, platform_src ]
)
