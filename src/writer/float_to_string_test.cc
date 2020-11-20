// Copyright 2020 The Tint Authors.
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

#include "src/writer/float_to_string.h"

#include <limits>

#include "gtest/gtest.h"

namespace tint {
namespace writer {
namespace {

TEST(FloatToStringTest, Zero) {
  EXPECT_EQ(FloatToString(0.0f), "0.0");
}

TEST(FloatToStringTest, One) {
  EXPECT_EQ(FloatToString(1.0f), "1.0");
}

TEST(FloatToStringTest, MinusOne) {
  EXPECT_EQ(FloatToString(-1.0f), "-1.0");
}

TEST(FloatToStringTest, Billion) {
  EXPECT_EQ(FloatToString(1e9f), "1000000000.0");
}

TEST(FloatToStringTest, Small) {
  EXPECT_NE(FloatToString(std::numeric_limits<float>::epsilon()), "0.0");
}

TEST(FloatToStringTest, Highest) {
  const auto highest = std::numeric_limits<float>::max();
  const auto expected_highest = 340282346638528859811704183484516925440.0f;
  if (highest < expected_highest || highest > expected_highest) {
    GTEST_SKIP() << "std::numeric_limits<float>::max() is not as expected for "
                    "this target";
  }
  EXPECT_EQ(FloatToString(std::numeric_limits<float>::max()),
            "340282346638528859811704183484516925440.0");
}

TEST(FloatToStringTest, Lowest) {
  // Some compilers complain if you test floating point numbers for equality.
  // So say it via two inequalities.
  const auto lowest = std::numeric_limits<float>::lowest();
  const auto expected_lowest = -340282346638528859811704183484516925440.0f;
  if (lowest < expected_lowest || lowest > expected_lowest) {
    GTEST_SKIP()
        << "std::numeric_limits<float>::lowest() is not as expected for "
           "this target";
  }
  EXPECT_EQ(FloatToString(std::numeric_limits<float>::lowest()),
            "-340282346638528859811704183484516925440.0");
}

}  // namespace
}  // namespace writer
}  // namespace tint
