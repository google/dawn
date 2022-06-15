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

#include "src/tint/transform/calculate_array_length.h"

#include "src/tint/transform/simplify_pointers.h"
#include "src/tint/transform/test_helper.h"
#include "src/tint/transform/unshadow.h"

namespace tint::transform {
namespace {

using CalculateArrayLengthTest = TransformTest;

TEST_F(CalculateArrayLengthTest, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<CalculateArrayLength>(src));
}

TEST_F(CalculateArrayLengthTest, ShouldRunNoArrayLength) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
}
)";

    EXPECT_FALSE(ShouldRun<CalculateArrayLength>(src));
}

TEST_F(CalculateArrayLengthTest, ShouldRunWithArrayLength) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

    EXPECT_TRUE(ShouldRun<CalculateArrayLength>(src));
}

TEST_F(CalculateArrayLengthTest, BasicArray) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 4u);
  var len : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, BasicInStruct) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var len : u32 = arrayLength(&sb.arr);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var len : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, ArrayOfStruct) {
    auto* src = R"(
struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<S>;

@compute @workgroup_size(1)
fn main() {
  let len = arrayLength(&arr);
}
)";
    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<S>, read>, result : ptr<function, u32>)

struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<S>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(arr), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 4u);
  let len = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, ArrayOfArrayOfStruct) {
    auto* src = R"(
struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<array<S, 4>>;

@compute @workgroup_size(1)
fn main() {
  let len = arrayLength(&arr);
}
)";
    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<array<S, 4u>>, read>, result : ptr<function, u32>)

struct S {
  f : f32,
}

@group(0) @binding(0) var<storage, read> arr : array<array<S, 4>>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(arr), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 16u);
  let len = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, InSameBlock) {
    auto* src = R"(
@group(0) @binding(0) var<storage, read> sb : array<i32>;;

@compute @workgroup_size(1)
fn main() {
  var a : u32 = arrayLength(&sb);
  var b : u32 = arrayLength(&sb);
  var c : u32 = arrayLength(&sb);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

@group(0) @binding(0) var<storage, read> sb : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = (tint_symbol_1 / 4u);
  var a : u32 = tint_symbol_2;
  var b : u32 = tint_symbol_2;
  var c : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, InSameBlock_Struct) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var a : u32 = arrayLength(&sb.arr);
  var b : u32 = arrayLength(&sb.arr);
  var c : u32 = arrayLength(&sb.arr);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var a : u32 = tint_symbol_2;
  var b : u32 = tint_symbol_2;
  var c : u32 = tint_symbol_2;
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, Nested) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
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
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> sb : SB;

@compute @workgroup_size(1)
fn main() {
  if (true) {
    var tint_symbol_1 : u32 = 0u;
    tint_symbol(&(sb), &(tint_symbol_1));
    let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
    var len : u32 = tint_symbol_2;
  } else {
    if (true) {
      var tint_symbol_3 : u32 = 0u;
      tint_symbol(&(sb), &(tint_symbol_3));
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
struct SB1 {
  x : i32,
  arr1 : array<i32>,
};

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
};

@group(0) @binding(0) var<storage, read> sb1 : SB1;

@group(0) @binding(1) var<storage, read> sb2 : SB2;

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var len3 : u32 = arrayLength(&sb3);
  var x : u32 = (len1 + len2 + len3);
}
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB1, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB2, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

struct SB1 {
  x : i32,
  arr1 : array<i32>,
}

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
}

@group(0) @binding(0) var<storage, read> sb1 : SB1;

@group(0) @binding(1) var<storage, read> sb2 : SB2;

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb1), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var tint_symbol_4 : u32 = 0u;
  tint_symbol_3(&(sb2), &(tint_symbol_4));
  let tint_symbol_5 : u32 = ((tint_symbol_4 - 16u) / 16u);
  var tint_symbol_7 : u32 = 0u;
  tint_symbol_6(&(sb3), &(tint_symbol_7));
  let tint_symbol_8 : u32 = (tint_symbol_7 / 4u);
  var len1 : u32 = tint_symbol_2;
  var len2 : u32 = tint_symbol_5;
  var len3 : u32 = tint_symbol_8;
  var x : u32 = ((len1 + len2) + len3);
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, Shadowing) {
    auto* src = R"(
struct SB {
  x : i32,
  arr : array<i32>,
};

@group(0) @binding(0) var<storage, read> a : SB;
@group(0) @binding(1) var<storage, read> b : SB;

@compute @workgroup_size(1)
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
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB, read>, result : ptr<function, u32>)

struct SB {
  x : i32,
  arr : array<i32>,
}

@group(0) @binding(0) var<storage, read> a : SB;

@group(0) @binding(1) var<storage, read> b : SB;

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(a), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var a_1 : u32 = tint_symbol_2;
  {
    var tint_symbol_3 : u32 = 0u;
    tint_symbol(&(a), &(tint_symbol_3));
    let tint_symbol_4 : u32 = ((tint_symbol_3 - 4u) / 4u);
    var b_1 : u32 = tint_symbol_4;
  }
}
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(CalculateArrayLengthTest, OutOfOrder) {
    auto* src = R"(
@compute @workgroup_size(1)
fn main() {
  var len1 : u32 = arrayLength(&(sb1.arr1));
  var len2 : u32 = arrayLength(&(sb2.arr2));
  var len3 : u32 = arrayLength(&sb3);
  var x : u32 = (len1 + len2 + len3);
}

@group(0) @binding(0) var<storage, read> sb1 : SB1;

struct SB1 {
  x : i32,
  arr1 : array<i32>,
};

@group(0) @binding(1) var<storage, read> sb2 : SB2;

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
};

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;
)";

    auto* expect = R"(
@internal(intrinsic_buffer_size)
fn tint_symbol(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB1, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_3(@internal(disable_validation__function_parameter) buffer : ptr<storage, SB2, read>, result : ptr<function, u32>)

@internal(intrinsic_buffer_size)
fn tint_symbol_6(@internal(disable_validation__function_parameter) buffer : ptr<storage, array<i32>, read>, result : ptr<function, u32>)

@compute @workgroup_size(1)
fn main() {
  var tint_symbol_1 : u32 = 0u;
  tint_symbol(&(sb1), &(tint_symbol_1));
  let tint_symbol_2 : u32 = ((tint_symbol_1 - 4u) / 4u);
  var tint_symbol_4 : u32 = 0u;
  tint_symbol_3(&(sb2), &(tint_symbol_4));
  let tint_symbol_5 : u32 = ((tint_symbol_4 - 16u) / 16u);
  var tint_symbol_7 : u32 = 0u;
  tint_symbol_6(&(sb3), &(tint_symbol_7));
  let tint_symbol_8 : u32 = (tint_symbol_7 / 4u);
  var len1 : u32 = tint_symbol_2;
  var len2 : u32 = tint_symbol_5;
  var len3 : u32 = tint_symbol_8;
  var x : u32 = ((len1 + len2) + len3);
}

@group(0) @binding(0) var<storage, read> sb1 : SB1;

struct SB1 {
  x : i32,
  arr1 : array<i32>,
}

@group(0) @binding(1) var<storage, read> sb2 : SB2;

struct SB2 {
  x : i32,
  arr2 : array<vec4<f32>>,
}

@group(0) @binding(2) var<storage, read> sb3 : array<i32>;
)";

    auto got = Run<Unshadow, SimplifyPointers, CalculateArrayLength>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
