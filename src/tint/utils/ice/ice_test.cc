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

#include "src/tint/utils/ice/ice.h"

#include "gtest/gtest-spi.h"

namespace tint {
namespace {

TEST(ICETest_AssertTrue_Test, Unreachable) {
    EXPECT_FATAL_FAILURE({ TINT_UNREACHABLE(); }, "internal compiler error");
}

TEST(ICETest_AssertTrue_Test, AssertTrue) {
    TINT_ASSERT(true);
}

TEST(ICETest_AssertTrue_Test, AssertFalse) {
    EXPECT_FATAL_FAILURE({ TINT_ASSERT(false); }, "internal compiler error");
}

}  // namespace
}  // namespace tint
