// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/ast_raise/while_to_loop.h"

#include "src/tint/lang/wgsl/ast/transform/helper_test.h"

namespace tint::spirv::writer {
namespace {

using WhileToLoopTest = ast::transform::TransformTest;

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

}  // namespace tint::spirv::writer
