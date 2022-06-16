// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/while_to_loop.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using WhileToLoopTest = TransformTest;

TEST_F(WhileToLoopTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<WhileToLoop>(src));
}

TEST_F(WhileToLoopTest, ShouldRunHasWhile) {
    auto* src = R"(
fn f() {
  while (true) {
    break;
  }
}
)";

    EXPECT_TRUE(ShouldRun<WhileToLoop>(src));
}

TEST_F(WhileToLoopTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    auto got = Run<WhileToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test an empty for loop.
TEST_F(WhileToLoopTest, Empty) {
    auto* src = R"(
fn f() {
  while (true) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  loop {
    if (!(true)) {
      break;
    }
    break;
  }
}
)";

    auto got = Run<WhileToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop with non-empty body.
TEST_F(WhileToLoopTest, Body) {
    auto* src = R"(
fn f() {
  while (true) {
    discard;
  }
}
)";

    auto* expect = R"(
fn f() {
  loop {
    if (!(true)) {
      break;
    }
    discard;
  }
}
)";

    auto got = Run<WhileToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a loop with a break condition
TEST_F(WhileToLoopTest, BreakCondition) {
    auto* src = R"(
fn f() {
  while (0 == 1) {
  }
}
)";

    auto* expect = R"(
fn f() {
  loop {
    if (!((0 == 1))) {
      break;
    }
  }
}
)";

    auto got = Run<WhileToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace

}  // namespace tint::transform
