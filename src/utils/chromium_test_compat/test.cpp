// created by chant
using namespace std;

#ifdef __clang__
#pragma clang diagnostic push
#include "src/dawn/utils/chromium_test_compat.h"

#include <gtest/gtest.h>

int chromium_test_compat_main(int argc, char** argv) {
    SubstituteChromiumArgs(argc, argv);

    auto result = chromium_test_compat_run_tests(argc, argv);
    if (result.has_value()) {
        return result.value();
    }

    ::testing::InitGoogleTest(&argc, argv);

    int exit_code = RUN_ALL_TESTS();

    chromium_test_compat_print_test_results();
    return exit_code;
}

void SubstituteChromiumArgs(int argc, char** argv);

#pragma clang diagnostic pop
#endif
