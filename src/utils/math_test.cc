// Copyright 2021 The Tint Authors.
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

#include "src/utils/math.h"

#include "gtest/gtest.h"

namespace tint {
namespace utils {
namespace {

TEST(MathTests, RoundUp) {
  EXPECT_EQ(RoundUp(1, 0), 0);
  EXPECT_EQ(RoundUp(1, 1), 1);
  EXPECT_EQ(RoundUp(1, 2), 2);

  EXPECT_EQ(RoundUp(1, 1), 1);
  EXPECT_EQ(RoundUp(2, 1), 2);
  EXPECT_EQ(RoundUp(3, 1), 3);
  EXPECT_EQ(RoundUp(4, 1), 4);

  EXPECT_EQ(RoundUp(1, 2), 2);
  EXPECT_EQ(RoundUp(2, 2), 2);
  EXPECT_EQ(RoundUp(3, 2), 3);
  EXPECT_EQ(RoundUp(4, 2), 4);

  EXPECT_EQ(RoundUp(1, 3), 3);
  EXPECT_EQ(RoundUp(2, 3), 4);
  EXPECT_EQ(RoundUp(3, 3), 3);
  EXPECT_EQ(RoundUp(4, 3), 4);

  EXPECT_EQ(RoundUp(1, 4), 4);
  EXPECT_EQ(RoundUp(2, 4), 4);
  EXPECT_EQ(RoundUp(3, 4), 6);
  EXPECT_EQ(RoundUp(4, 4), 4);
}

TEST(MathTests, IsPowerOfTwo) {
  EXPECT_EQ(IsPowerOfTwo(1), true);
  EXPECT_EQ(IsPowerOfTwo(2), true);
  EXPECT_EQ(IsPowerOfTwo(3), false);
  EXPECT_EQ(IsPowerOfTwo(4), true);
  EXPECT_EQ(IsPowerOfTwo(5), false);
  EXPECT_EQ(IsPowerOfTwo(6), false);
  EXPECT_EQ(IsPowerOfTwo(7), false);
  EXPECT_EQ(IsPowerOfTwo(8), true);
  EXPECT_EQ(IsPowerOfTwo(9), false);
}

}  // namespace
}  // namespace utils
}  // namespace tint
