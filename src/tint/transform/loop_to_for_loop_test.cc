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

#include "src/tint/transform/loop_to_for_loop.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using LoopToForLoopTest = TransformTest;

TEST_F(LoopToForLoopTest, ShouldRunEmptyModule) {
  auto* src = R"()";

  EXPECT_FALSE(ShouldRun<LoopToForLoop>(src));
}

TEST_F(LoopToForLoopTest, ShouldRunHasForLoop) {
  auto* src = R"(
fn f() {
  loop {
    break;
  }
}
)";

  EXPECT_TRUE(ShouldRun<LoopToForLoop>(src));
}

TEST_F(LoopToForLoopTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, IfBreak) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if (i > 15) {
      break;
    }

    _ = 123;

    continuing {
      i = i + 1;
    }
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  i = 0;
  for(; !((i > 15)); i = (i + 1)) {
    _ = 123;
  }
}
)";

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, IfElseBreak) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if (i < 15) {
    } else {
      break;
    }

    _ = 123;

    continuing {
      i = i + 1;
    }
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  i = 0;
  for(; (i < 15); i = (i + 1)) {
    _ = 123;
  }
}
)";

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, Nested) {
  auto* src = R"(
let N = 16u;

fn f() {
  var i : u32 = 0u;
  loop {
    if (i >= N) {
      break;
    }
    {
      var j : u32 = 0u;
      loop {
        if (j >= N) {
          break;
        }

        _ = i;
        _ = j;

        continuing {
          j = (j + 1u);
        }
      }
    }

    continuing {
      i = (i + 1u);
    }
  }
}
)";

  auto* expect = R"(
let N = 16u;

fn f() {
  var i : u32 = 0u;
  for(; !((i >= N)); i = (i + 1u)) {
    {
      var j : u32 = 0u;
      for(; !((j >= N)); j = (j + 1u)) {
        _ = i;
        _ = j;
      }
    }
  }
}
)";

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, NoTransform_IfMultipleStmts) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if ((i < 15)) {
      _ = i;
      break;
    }
    _ = 123;

    continuing {
      i = (i + 1);
    }
  }
}
)";

  auto* expect = src;

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, NoTransform_IfElseMultipleStmts) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if ((i < 15)) {
    } else {
      _ = i;
      break;
    }
    _ = 123;

    continuing {
      i = (i + 1);
    }
  }
}
)";

  auto* expect = src;

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, NoTransform_ContinuingIsCompound) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if ((i < 15)) {
      break;
    }
    _ = 123;

    continuing {
      if (false) {
      }
    }
  }
}
)";

  auto* expect = src;

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, NoTransform_ContinuingMultipleStmts) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if ((i < 15)) {
      break;
    }
    _ = 123;

    continuing {
      i = (i + 1);
      _ = i;
    }
  }
}
)";

  auto* expect = src;

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, NoTransform_ContinuingUsesVarDeclInLoopBody) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if ((i < 15)) {
      break;
    }
    var j : i32;

    continuing {
      i = (i + j);
    }
  }
}
)";

  auto* expect = src;

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, NoTransform_IfBreakWithElse) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if ((i > 15)) {
      break;
    } else {
    }
    _ = 123;

    continuing {
      i = (i + 1);
    }
  }
}
)";

  auto* expect = src;

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(LoopToForLoopTest, NoTransform_IfBreakWithElseIf) {
  auto* src = R"(
fn f() {
  var i : i32;
  i = 0;
  loop {
    if ((i > 15)) {
      break;
    } else if (true) {
    }
    _ = 123;

    continuing {
      i = (i + 1);
    }
  }
}
)";

  auto* expect = src;

  auto got = Run<LoopToForLoop>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
