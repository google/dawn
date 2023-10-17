// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/spirv/writer/ast_raise/for_loop_to_loop.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::spirv::writer {
namespace {

using ForLoopToLoopTest = ast::transform::TransformTest;

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
}  // namespace tint::spirv::writer
