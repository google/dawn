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

#include "src/tint/utils/string.h"

#include "gtest/gtest.h"

namespace tint::utils {
namespace {

TEST(StringTest, ReplaceAll) {
    ASSERT_EQ("xybbcc", ReplaceAll("aabbcc", "aa", "xy"));
    ASSERT_EQ("aaxycc", ReplaceAll("aabbcc", "bb", "xy"));
    ASSERT_EQ("aabbxy", ReplaceAll("aabbcc", "cc", "xy"));
    ASSERT_EQ("xyxybbcc", ReplaceAll("aabbcc", "a", "xy"));
    ASSERT_EQ("aaxyxycc", ReplaceAll("aabbcc", "b", "xy"));
    ASSERT_EQ("aabbxyxy", ReplaceAll("aabbcc", "c", "xy"));
    // Replacement string includes the searched-for string.
    // This proves that the algorithm needs to advance 'pos'
    // past the replacement.
    ASSERT_EQ("aabxybbxybcc", ReplaceAll("aabbcc", "b", "bxyb"));
}

}  // namespace
}  // namespace tint::utils
