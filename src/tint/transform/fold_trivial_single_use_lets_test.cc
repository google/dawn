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

#include "src/tint/transform/fold_trivial_single_use_lets.h"

#include "src/tint/transform/test_helper.h"

namespace tint::transform {
namespace {

using FoldTrivialSingleUseLetsTest = TransformTest;

TEST_F(FoldTrivialSingleUseLetsTest, EmptyModule) {
    auto* src = "";
    auto* expect = "";

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, Single_Concrete) {
    auto* src = R"(
fn f() {
  let x = 1i;
  _ = x;
}
)";

    auto* expect = R"(
fn f() {
  _ = 1i;
}
)";

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, Single_Abstract) {
    auto* src = R"(
fn f() {
  let x = 1;
  _ = x;
}
)";

    auto* expect = R"(
fn f() {
  _ = i32(1);
}
)";

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, Multiple_Concrete) {
    auto* src = R"(
fn f() {
  let x = 1u;
  let y = 2u;
  let z = 3u;
  _ = x + y + z;
}
)";

    auto* expect = R"(
fn f() {
  _ = ((1u + 2u) + 3u);
}
)";

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, Multiple_Abstract) {
    auto* src = R"(
fn f() {
  let x = 1;
  let y = 2;
  let z = 3;
  _ = x + y + z;
}
)";

    auto* expect = R"(
fn f() {
  _ = ((i32(1) + i32(2)) + i32(3));
}
)";

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, Chained_Concrete) {
    auto* src = R"(
fn f() {
  let x = 1i;
  let y = x;
  let z = y;
  _ = z;
}
)";

    auto* expect = R"(
fn f() {
  _ = 1i;
}
)";

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, Chained_Abstract) {
    auto* src = R"(
fn f() {
  let x = 1;
  let y = x;
  let z = y;
  _ = z;
}
)";

    auto* expect = R"(
fn f() {
  _ = i32(1);
}
)";

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, NoFold_NonTrivialLet) {
    auto* src = R"(
fn function_with_possible_side_effect() -> i32 {
  return 1;
}

fn f() {
  let x = 1;
  let y = function_with_possible_side_effect();
  _ = (x + y);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, NoFold_NonTrivialLet_OutOfOrder) {
    auto* src = R"(
fn f() {
  let x = 1;
  let y = function_with_possible_side_effect();
  _ = (x + y);
}

fn function_with_possible_side_effect() -> i32 {
  return 1;
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, NoFold_UseInSubBlock) {
    auto* src = R"(
fn f() {
  let x = 1;
  {
    _ = x;
  }
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, NoFold_MultipleUses) {
    auto* src = R"(
fn f() {
  let x = 1;
  _ = (x + x);
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(FoldTrivialSingleUseLetsTest, NoFold_Shadowing) {
    auto* src = R"(
fn f() {
  var y = 1;
  let x = y;
  {
    let y = false;
    _ = (x + x);
  }
}
)";

    auto* expect = src;

    auto got = Run<FoldTrivialSingleUseLets>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
