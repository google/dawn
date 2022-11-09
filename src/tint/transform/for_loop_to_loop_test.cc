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

#include "src/tint/transform/for_loop_to_loop.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using ForLoopToLoopTest = TransformTest;

TEST_F(ForLoopToLoopTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<ForLoopToLoop>(src));
}

TEST_F(ForLoopToLoopTest, ShouldRunHasForLoop) {
    auto* src = R"(
fn f() {
  for (;;) {
    break;
  }
}
)";

    EXPECT_TRUE(ShouldRun<ForLoopToLoop>(src));
}

TEST_F(ForLoopToLoopTest, EmptyModule) {
    auto* src = "";
    auto* expect = src;

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test an empty for loop.
TEST_F(ForLoopToLoopTest, Empty) {
    auto* src = R"(
fn f() {
  for (;;) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  loop {
    break;
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop with non-empty body.
TEST_F(ForLoopToLoopTest, Body) {
    auto* src = R"(
fn f() {
  for (;;) {
    return;
  }
}
)";

    auto* expect = R"(
fn f() {
  loop {
    return;
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop declaring a variable in the initializer statement.
TEST_F(ForLoopToLoopTest, InitializerStatementDecl) {
    auto* src = R"(
fn f() {
  for (var i: i32;;) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  {
    var i : i32;
    loop {
      break;
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop declaring and initializing a variable in the initializer
// statement.
TEST_F(ForLoopToLoopTest, InitializerStatementDeclEqual) {
    auto* src = R"(
fn f() {
  for (var i: i32 = 0;;) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  {
    var i : i32 = 0;
    loop {
      break;
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop declaring a const variable in the initializer statement.
TEST_F(ForLoopToLoopTest, InitializerStatementConstDecl) {
    auto* src = R"(
fn f() {
  for (let i: i32 = 0;;) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  {
    let i : i32 = 0;
    loop {
      break;
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop assigning a variable in the initializer statement.
TEST_F(ForLoopToLoopTest, InitializerStatementAssignment) {
    auto* src = R"(
fn f() {
  var i: i32;
  for (i = 0;;) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var i : i32;
  {
    i = 0;
    loop {
      break;
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop calling a function in the initializer statement.
TEST_F(ForLoopToLoopTest, InitializerStatementFuncCall) {
    auto* src = R"(
fn a(x : i32, y : i32) {
}

fn f() {
  var b : i32;
  var c : i32;
  for (a(b,c);;) {
    break;
  }
}
)";

    auto* expect = R"(
fn a(x : i32, y : i32) {
}

fn f() {
  var b : i32;
  var c : i32;
  {
    a(b, c);
    loop {
      break;
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop with a break condition
TEST_F(ForLoopToLoopTest, BreakCondition) {
    auto* src = R"(
fn f() {
  for (; 0 == 1;) {
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

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop assigning a variable in the continuing statement.
TEST_F(ForLoopToLoopTest, ContinuingAssignment) {
    auto* src = R"(
fn f() {
  var x: i32;
  for (;;x = 2) {
    break;
  }
}
)";

    auto* expect = R"(
fn f() {
  var x : i32;
  loop {
    break;

    continuing {
      x = 2;
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop calling a function in the continuing statement.
TEST_F(ForLoopToLoopTest, ContinuingFuncCall) {
    auto* src = R"(
fn a(x : i32, y : i32) {
}

fn f() {
  var b : i32;
  var c : i32;
  for (;;a(b,c)) {
    break;
  }
}
)";

    auto* expect = R"(
fn a(x : i32, y : i32) {
}

fn f() {
  var b : i32;
  var c : i32;
  loop {
    break;

    continuing {
      a(b, c);
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

// Test a for loop with all statements non-empty.
TEST_F(ForLoopToLoopTest, All) {
    auto* src = R"(
fn f() {
  var a : i32;
  for(var i : i32 = 0; i < 4; i = i + 1) {
    if (a == 0) {
      continue;
    }
    a = a + 2;
  }
}
)";

    auto* expect = R"(
fn f() {
  var a : i32;
  {
    var i : i32 = 0;
    loop {
      if (!((i < 4))) {
        break;
      }
      if ((a == 0)) {
        continue;
      }
      a = (a + 2);

      continuing {
        i = (i + 1);
      }
    }
  }
}
)";

    auto got = Run<ForLoopToLoop>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
