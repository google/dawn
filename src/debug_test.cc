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

#include "src/debug.h"

#include "gtest/gtest-spi.h"

namespace tint {
namespace {

TEST(DebugTest, Unreachable) {
  EXPECT_FATAL_FAILURE(
      {
        diag::List diagnostics;
        TINT_UNREACHABLE(diagnostics);
      },
      "internal compiler error");

  // Ensure that this test does not leak memory.
  // This will be automatically called by main() in src/test_main.cc, but
  // chromium uses it's own test entry point.
  // TODO(ben-clayton): Add this call to the end of Chromium's main(), and we
  // can remove this call.
  FreeInternalCompilerErrors();
}

}  // namespace
}  // namespace tint
