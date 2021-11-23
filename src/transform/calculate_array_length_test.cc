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

#include "src/transform/calculate_array_length.h"

#include "src/transform/simplify_pointers.h"
#include "src/transform/test_helper.h"
#include "src/transform/unshadow.h"

namespace tint {
namespace transform {
namespace {

using CalculateArrayLengthTest = TransformTest;

TEST_F(CalculateArrayLengthTest, Error_MissingCalculateArrayLength) {
  auto* src = "";

  auto* expect =
      "error: tint::transform::CalculateArrayLength depends on "
      "tint::transform::SimplifyPointers but the dependency was not run";

  auto got = Run<CalculateArrayLength>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, Basic) {
  auto* src = R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

  auto* expect = R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[internal(intrinsic_buffer_size)]]
fn tint_symbol([[internal(disable_validation__ignore_constructible_function_parameter)]] buffer : SB, result : ptr<function, u32>)

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(sb, &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var len : u32 = tint_symbol_2;
}
)";

  auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, InSameBlock) {
  auto* src = R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var a : u32 = arrayLength(&sb.arr);
  var b : u32 = arrayLength(&sb.arr);
  var c : u32 = arrayLength(&sb.arr);
}
)";

  auto* expect = R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[internal(intrinsic_buffer_size)]]
fn tint_symbol([[internal(disable_validation__ignore_constructible_function_parameter)]] buffer : SB, result : ptr<function, u32>)

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(sb, &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var a : u32 = tint_symbol_2;
  var b : u32 = tint_symbol_2;
  var c : u32 = tint_symbol_2;
}
)";

  auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, WithStride) {
  auto* src = R"(
[[block]]
struct SB {
  x : i32;
  y : f32;
  arr : [[stride(64)]] array<i32>;
};

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

  auto* expect = R"(
[[block]]
struct SB {
  x : i32;
  y : f32;
  arr : [[stride(64)]] array<i32>;
};

[[internal(intrinsic_buffer_size)]]
fn tint_symbol([[internal(disable_validation__ignore_constructible_function_parameter)]] buffer : SB, result : ptr<function, u32>)

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(sb, &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 8u) / 64u);
  var len : u32 = tint_symbol_2;
}
)";

  auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, Nested) {
  auto* src = R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  if (true) {
    var len : u32 = arrayLength(&sb.arr);
  } else {
    if (true) {
      var len : u32 = arrayLength(&sb.arr);
    }
  }
}
)";

  auto* expect = R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[internal(intrinsic_buffer_size)]]
fn tint_symbol([[internal(disable_validation__ignore_constructible_function_parameter)]] buffer : SB, result : ptr<function, u32>)

[[group(0), binding(0)]] var<storage, read> sb : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  if (true) {
    var tint_symbol_1 : u32 = 0u;
    tint_symbol(sb, &(tint_symbol_1));
    let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
    var len : u32 = tint_symbol_2;
  } else {
    if (true) {
      var tint_symbol_3 : u32 = 0u;
      tint_symbol(sb, &(tint_symbol_3));
      let tint_symbol_4 : u32 = ((tint_symbol_3 - 4u) / 4u);
      var len : u32 = tint_symbol_4;
    }
  }
}
)";

  auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, MultipleStorageBuffers) {
  auto* src = R"(
[[block]]
struct SB1 {
  x : i32;
  arr1 : array<i32>;
};

[[block]]
struct SB2 {
  x : i32;
  arr2 : array<vec4<f32>>;
};

[[group(0), binding(0)]] var<storage, read> sb1 : SB1;

[[group(0), binding(1)]] var<storage, read> sb2 : SB2;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var x : u32 = (len1 + len2);
}
)";

  auto* expect = R"(
[[block]]
struct SB1 {
  x : i32;
  arr1 : array<i32>;
};

[[internal(intrinsic_buffer_size)]]
fn tint_symbol([[internal(disable_validation__ignore_constructible_function_parameter)]] buffer : SB1, result : ptr<function, u32>)

[[block]]
struct SB2 {
  x : i32;
  arr2 : array<vec4<f32>>;
};

[[internal(intrinsic_buffer_size)]]
fn tint_symbol_3([[internal(disable_validation__ignore_constructible_function_parameter)]] buffer : SB2, result : ptr<function, u32>)

[[group(0), binding(0)]] var<storage, read> sb1 : SB1;

[[group(0), binding(1)]] var<storage, read> sb2 : SB2;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(sb1, &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var tint_symbol_4 : u32 = 0u;
  tint_symbol_3(sb2, &(tint_symbol_4));
  let tint_symbol_5 : u32 = ((tint_symbol_4 - 16u) / 16u);
  var len1 : u32 = tint_symbol_2;
  var len2 : u32 = tint_symbol_5;
  var x : u32 = (len1 + len2);
}
)";

  auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

  EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, Shadowing) {
  auto* src = R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[group(0), binding(0)]] var<storage, read> a : SB;
[[group(0), binding(1)]] var<storage, read> b : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  let x = &a;
  var a : u32 = arrayLength(&a.arr);
  {
    var b : u32 = arrayLength(&((*x).arr));
  }
}
)";

  auto* expect =
      R"(
[[block]]
struct SB {
  x : i32;
  arr : array<i32>;
};

[[internal(intrinsic_buffer_size)]]
fn tint_symbol([[internal(disable_validation__ignore_constructible_function_parameter)]] buffer : SB, result : ptr<function, u32>)

[[group(0), binding(0)]] var<storage, read> a : SB;

[[group(0), binding(1)]] var<storage, read> b : SB;

[[stage(compute), workgroup_size(1)]]
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(a, &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var a_1 : u32 = tint_symbol_2;
  {
    var tint_symbol_3 : u32 = 0u;
    tint_symbol(a, &(tint_symbol_3));
    let tint_symbol_4 : u32 = ((tint_symbol_3 - 4u) / 4u);
    var b_1 : u32 = tint_symbol_4;
  }
}
)";

  auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

  EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace transform
}  // namespace tint
