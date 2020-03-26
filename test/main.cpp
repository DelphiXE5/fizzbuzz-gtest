#include <gtest/gtest.h>
#include "gtest_reporter_eh.hpp"
#include "../lib/googletest/googletest/src/gtest-internal-inl.h"

using namespace testing;

int main(int argc, char* argv[])
{

    bool terse_output = false;
    if (argc > 1 && strcmp(argv[1], "--gtest_eh_output") == 0) {
        terse_output = true;
        argv[1][0] = 0;
    }
    else
        printf("%s\n", "Run this program with --terse_output to change the way "
            "it prints its output.");
    InitGoogleTest(&argc, argv);
    UnitTest& unit_test = *UnitTest::GetInstance();
    if (terse_output) {
        delete unit_test.listeners().Release(unit_test.listeners().default_result_printer());
        unit_test.listeners().Append(new testing::EHReporter(UnitTestOptions::GetAbsolutePathToOutputFile().c_str()));
    }
    return RUN_ALL_TESTS();
}