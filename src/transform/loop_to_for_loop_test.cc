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

#include "src/transform/loop_to_for_loop.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using LoopToForLoopTest = TransformTest;

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

    ignore(123);

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
    ignore(123);
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

    ignore(123);

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
    ignore(123);
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

        ignore(i);
        ignore(j);

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
        ignore(i);
        ignore(j);
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
      ignore(i);
      break;
    }
    ignore(123);

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
      ignore(i);
      break;
    }
    ignore(123);

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
    ignore(123);

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
    ignore(123);

    continuing {
      i = (i + 1);
      ignore(i);
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

}  // namespace
}  // namespace transform
}  // namespace tint
