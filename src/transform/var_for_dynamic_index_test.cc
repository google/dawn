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

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using VarForDynamicIndexTest = TransformTest;

TEST_F(VarForDynamicIndexTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexDynamic) {
  auto* src = R"(
fn f() {
  var i : i32;
  let p : array<i32, 4> = array<i32, 4>(1, 2, 3, 4);
  let x : i32 = p[i];
}
)";

  auto* expect = R"(
fn f() {
  var i : i32;
  let p : array<i32, 4> = array<i32, 4>(1, 2, 3, 4);
  var var_for_index : array<i32, 4> = p;
  let x : i32 = var_for_index[i];
}
)";

  auto got = Run<VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexDynamicChain) {
  auto* src = R"(
fn f() {
  var i : i32;
  var j : i32;
  let p : array<array<i32, 2>, 2> = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  let x : i32 = p[i][j];
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
  let p : array<array<i32, 2>, 2> = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  var var_for_index : array<array<i32, 2>, 2> = p;
  var var_for_index_1 : array<i32, 2> = var_for_index[i];
  let x : i32 = var_for_index_1[j];
}
)";

  auto got = Run<VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexLiteral) {
  auto* src = R"(
fn f() {
  let p : array<i32, 4> = array<i32, 4>(1, 2, 3, 4);
  let x : i32 = p[1];
}
)";

  auto* expect = src;

  auto got = Run<VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(VarForDynamicIndexTest, ArrayIndexLiteralChain) {
  auto* src = R"(
fn f() {
  let p : array<array<i32, 2>, 2> = array<array<i32, 2>, 2>(array<i32, 2>(1, 2), array<i32, 2>(3, 4));
  let x : i32 = p[0][1];
}
)";

  auto* expect = src;

  auto got = Run<VarForDynamicIndex>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
