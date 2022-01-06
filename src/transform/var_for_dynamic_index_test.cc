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

#include "src/transform/var_for_dynamic_index.h"
#include "src/transform/for_loop_to_loop.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using VarForDynamicIndexTest = TransformTest;

TEST_F(VarForDynamicIndexTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexDynamic) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<i32, 4>(1, 2, 3, 4);
  let x = p[i];
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = array<i32, 4>(1, 2, 3, 4);
  var var_for_index = p;
  let x = var_for_index[i];
}
)";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexDynamic) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[i];
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  var var_for_index = p;
  let x = var_for_index[i];
}
)";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexDynamicChain) {
  auto* src = R"(
fn f() {
  var i : i32;
  var j : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  let x = p[i][j];
}
)";

  // TODO(bclayton): Optimize this case:
  // This output is not as efficient as it could be.
  // We only actually need to hoist the inner-most array to a `var`
  // (`var_for_index`), as later indexing operations will be working with
  // references, not values.

  auto* expect = R"(
fn f() {
  var i : i32;
  var j : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  var var_for_index = p;
  var var_for_index_1 = var_for_index[i];
  let x = var_for_index_1[j];
}
)";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexInForLoopInit) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  for(let x = p[i]; ; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  {
    var var_for_index = p;
    let x = var_for_index[i];
    loop {
      break;
    }
  }
}
)";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexInForLoopInit) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  for(let x = p[i]; ; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  {
    var var_for_index = p;
    let x = var_for_index[i];
    loop {
      break;
    }
  }
}
)";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexInForLoopCond) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = array<i32, 2>(1, 2);
  for(; p[i] < 3; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = array<i32, 2>(1, 2);
  loop {
    var var_for_index = p;
    if (!((var_for_index[i] < 3))) {
      break;
    }
    break;
  }
}
)";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexInForLoopCond) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  for(; p[i].x < 3.0; ) {
    break;
  }
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  loop {
    var var_for_index = p;
    if (!((var_for_index[i].x < 3.0))) {
      break;
    }
    break;
  }
}
)";

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexLiteral) {
  auto* src = R"(
fn f() {
  let p = array<i32, 4>(1, 2, 3, 4);
  let x = p[1];
}
)";

  auto* expect = src;

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexLiteral) {
  auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[1];
}
)";

  auto* expect = src;

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexConstantLet) {
  auto* src = R"(
fn f() {
  let p = array<i32, 4>(1, 2, 3, 4);
  let c = 1;
  let x = p[c];
}
)";

  auto* expect = src;

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexConstantLet) {
  auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let c = 1;
  let x = p[c];
}
)";

  auto* expect = src;

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexLiteralChain) {
  auto* src = R"(
fn f() {
  let p = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  let x = p[0][1];
}
)";

  auto* expect = src;

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, MatrixIndexLiteralChain) {
  auto* src = R"(
fn f() {
  let p = mat2x2(1.0, 2.0, 3.0, 4.0);
  let x = p[0][1];
}
)";

  auto* expect = src;

  auto got = Run<ForLoopToLoop, VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
