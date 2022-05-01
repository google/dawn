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

#include "src/tint/debug.h"

#include "gtest/gtest-spi.h"

namespace tint {
namespace {

TEST(DebugTest, Unreachable) {
    EXPECT_FATAL_FAILURE(
        {
            diag::List diagnostics;
            TINT_UNREACHABLE(Test, diagnostics);
        },
        "internal compiler error");
}

TEST(DebugTest, AssertTrue) {
    TINT_ASSERT(Test, true);
}

TEST(DebugTest, AssertFalse) {
    EXPECT_FATAL_FAILURE({ TINT_ASSERT(Test, false); }, "internal compiler error");
}

}  // namespace
}  // namespace tint
