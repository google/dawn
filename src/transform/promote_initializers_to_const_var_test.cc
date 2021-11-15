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

#include "src/transform/promote_initializers_to_const_var.h"

#include "src/transform/test_helper.h"

namespace tint {
namespace transform {
namespace {

using PromoteInitializersToConstVarTest = TransformTest;

TEST_F(PromoteInitializersToConstVarTest, BasicArray) {
  auto* src = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
  var f0 : f32 = 1.0;
  var f1 : f32 = 2.0;
  var f2 : f32 = 3.0;
  var f3 : f32 = 4.0;
  var i : f32 = array<f32, 4u>(f0, f1, f2, f3)[2];
}
)";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
  var f0 : f32 = 1.0;
  var f1 : f32 = 2.0;
  var f2 : f32 = 3.0;
  var f3 : f32 = 4.0;
  let tint_symbol : array<f32, 4u> = array<f32, 4u>(f0, f1, f2, f3);
  var i : f32 = tint_symbol[2];
}
)";

  auto got = Run<PromoteInitializersToConstVar>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, BasicStruct) {
  auto* src = R"(
struct S {
  a : i32;
  b : f32;
  c : vec3<f32>;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  var x : f32 = S(1, 2.0, vec3<f32>()).b;
}
)";

  auto* expect = R"(
struct S {
  a : i32;
  b : f32;
  c : vec3<f32>;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  let tint_symbol : S = S(1, 2.0, vec3<f32>());
  var x : f32 = tint_symbol.b;
}
)";

  auto got = Run<PromoteInitializersToConstVar>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, ArrayInArrayArray) {
  auto* src = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
  var i : f32 = array<array<f32, 2u>, 2u>(array<f32, 2u>(1.0, 2.0), array<f32, 2u>(3.0, 4.0))[0][1];
}
)";

  auto* expect = R"(
[[stage(compute), workgroup_size(1)]]
fn main() {
  let tint_symbol : array<f32, 2u> = array<f32, 2u>(1.0, 2.0);
  let tint_symbol_1 : array<f32, 2u> = array<f32, 2u>(3.0, 4.0);
  let tint_symbol_2 : array<array<f32, 2u>, 2u> = array<array<f32, 2u>, 2u>(tint_symbol, tint_symbol_1);
  var i : f32 = tint_symbol_2[0][1];
}
)";

  auto got = Run<PromoteInitializersToConstVar>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, StructNested) {
  auto* src = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : i32;
  b : S1;
  c : i32;
};

struct S3 {
  a : S2;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  var x : i32 = S3(S2(1, S1(2), 3)).a.b.a;
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : i32;
  b : S1;
  c : i32;
};

struct S3 {
  a : S2;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  let tint_symbol : S1 = S1(2);
  let tint_symbol_1 : S2 = S2(1, tint_symbol, 3);
  let tint_symbol_2 : S3 = S3(tint_symbol_1);
  var x : i32 = tint_symbol_2.a.b.a;
}
)";

  auto got = Run<PromoteInitializersToConstVar>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, Mixed) {
  auto* src = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : array<S1, 3u>;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  var x : i32 = S2(array<S1, 3u>(S1(1), S1(2), S1(3))).a[1].a;
}
)";

  auto* expect = R"(
struct S1 {
  a : i32;
};

struct S2 {
  a : array<S1, 3u>;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  let tint_symbol : S1 = S1(1);
  let tint_symbol_1 : S1 = S1(2);
  let tint_symbol_2 : S1 = S1(3);
  let tint_symbol_3 : array<S1, 3u> = array<S1, 3u>(tint_symbol, tint_symbol_1, tint_symbol_2);
  let tint_symbol_4 : S2 = S2(tint_symbol_3);
  var x : i32 = tint_symbol_4.a[1].a;
}
)";

  auto got = Run<PromoteInitializersToConstVar>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, NoChangeOnVarDecl) {
  auto* src = R"(
struct S {
  a : i32;
  b : f32;
  c : i32;
};

[[stage(compute), workgroup_size(1)]]
fn main() {
  var local_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);
  var local_str : S = S(1, 2.0, 3);
}

let module_arr : array<f32, 4u> = array<f32, 4u>(0.0, 1.0, 2.0, 3.0);

let module_str : S = S(1, 2.0, 3);
)";

  auto* expect = src;

  auto got = Run<PromoteInitializersToConstVar>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(PromoteInitializersToConstVarTest, EmptyModule) {
  auto* src = "";
  auto* expect = "";

  auto got = Run<PromoteInitializersToConstVar>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
