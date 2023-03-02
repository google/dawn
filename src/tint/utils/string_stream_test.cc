// Copyright 2023 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/utils/string_stream.h"

#include <math.h>
#include <cstring>
#include <limits>

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

using StringStreamTest = testing::Test;

TEST_F(StringStreamTest, Zero) {
    StringStream s;
    s << 0.0f;
    EXPECT_EQ(s.str(), "0.0");
}

TEST_F(StringStreamTest, One) {
    StringStream s;
    s << 1.0f;
    EXPECT_EQ(s.str(), "1.0");
}

TEST_F(StringStreamTest, MinusOne) {
    StringStream s;
    s << -1.0f;
    EXPECT_EQ(s.str(), "-1.0");
}

TEST_F(StringStreamTest, Billion) {
    StringStream s;
    s << 1e9f;
    EXPECT_EQ(s.str(), "1000000000.0");
}

TEST_F(StringStreamTest, Small) {
    StringStream s;
    s << std::numeric_limits<float>::epsilon();
    EXPECT_NE(s.str(), "0.0");
}

TEST_F(StringStreamTest, Highest) {
    const auto highest = std::numeric_limits<float>::max();
    const auto expected_highest = 340282346638528859811704183484516925440.0f;

    if (highest < expected_highest || highest > expected_highest) {
        GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                        "this target";
    }

    StringStream s;
    s << std::numeric_limits<float>::max();
    EXPECT_EQ(s.str(), "340282346638528859811704183484516925440.0");
}

TEST_F(StringStreamTest, Lowest) {
    // Some compilers complain if you test floating point numbers for equality.
    // So say it via two inequalities.
    const auto lowest = std::numeric_limits<float>::lowest();
    const auto expected_lowest = -340282346638528859811704183484516925440.0f;
    if (lowest < expected_lowest || lowest > expected_lowest) {
        GTEST_SKIP() << "std::numeric_limits<float>::lowest() is not as expected for "
                        "this target";
    }

    StringStream s;
    s << std::numeric_limits<float>::lowest();
    EXPECT_EQ(s.str(), "-340282346638528859811704183484516925440.0");
}

TEST_F(StringStreamTest, Precision) {
    {
        StringStream s;
        s << 1e-8f;
        EXPECT_EQ(s.str(), "0.00000000999999993923");
    }
    {
        StringStream s;
        s << 1e-9f;
        EXPECT_EQ(s.str(), "0.00000000099999997172");
    }
    {
        StringStream s;
        s << 1e-10f;
        EXPECT_EQ(s.str(), "0.00000000010000000134");
    }
    {
        StringStream s;
        s << 1e-20f;
        EXPECT_EQ(s.str(), "0.00000000000000000001");
    }
}

}  // namespace
}  // namespace tint::utils
