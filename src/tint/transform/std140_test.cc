// Copyright 2022 The Tint Authors.
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

#include "src/tint/transform/std140.h"

#include <string>
#include <utility>
#include <vector>

#include "src/tint/transform/test_helper.h"
#include "src/tint/utils/string.h"

namespace tint::transform {
namespace {

using Std140Test = TransformTest;

TEST_F(Std140Test, ShouldRunEmptyModule) {
    auto* src = R"()";

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

TEST_F(Std140Test, ShouldRunStructMat2x2Unused) {
    auto* src = R"(
struct Unused {
  m : mat2x2<f32>,
}
)";

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

struct ShouldRunCase {
    uint32_t columns;
    uint32_t rows;
    bool should_run;

    std::string Mat() const { return "mat" + std::to_string(columns) + "x" + std::to_string(rows); }
};

inline std::ostream& operator<<(std::ostream& os, const ShouldRunCase& c) {
    return os << c.Mat();
}

using Std140TestShouldRun = TransformTestWithParam<ShouldRunCase>;

TEST_P(Std140TestShouldRun, StructStorage) {
    std::string src = R"(
struct S {
  m : ${mat}<f32>,
}

@group(0) @binding(0) var<storage> s : S;
)";

    src = utils::ReplaceAll(src, "${mat}", GetParam().Mat());

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

TEST_P(Std140TestShouldRun, StructUniform) {
    std::string src = R"(
struct S {
  m : ${mat}<f32>,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    src = utils::ReplaceAll(src, "${mat}", GetParam().Mat());

    EXPECT_EQ(ShouldRun<Std140>(src), GetParam().should_run);
}

TEST_P(Std140TestShouldRun, ArrayStorage) {
    std::string src = R"(
@group(0) @binding(0) var<storage> s : array<${mat}<f32>, 2>;
)";

    src = utils::ReplaceAll(src, "${mat}", GetParam().Mat());

    EXPECT_FALSE(ShouldRun<Std140>(src));
}

TEST_P(Std140TestShouldRun, ArrayUniform) {
    if (GetParam().columns == 3u && GetParam().rows == 2u) {
        // This permutation is invalid. Skip the test:
        // error: uniform storage requires that array elements be aligned to 16 bytes, but array
        // element alignment is currently 24. Consider wrapping the element type in a struct and
        // using the @size attribute.
        return;
    }

    std::string src = R"(
@group(0) @binding(0) var<uniform> s : array<${mat}<f32>, 2>;
)";

    src = utils::ReplaceAll(src, "${mat}", GetParam().Mat());

    EXPECT_EQ(ShouldRun<Std140>(src), GetParam().should_run);
}

INSTANTIATE_TEST_SUITE_P(Std140TestShouldRun,
                         Std140TestShouldRun,
                         ::testing::ValuesIn(std::vector<ShouldRunCase>{
                             {2, 2, true},
                             {2, 3, false},
                             {2, 4, false},
                             {3, 2, true},
                             {3, 3, false},
                             {3, 4, false},
                             {4, 2, true},
                             {4, 3, false},
                             {4, 4, false},
                         }));

TEST_F(Std140Test, EmptyModule) {
    auto* src = R"()";

    auto* expect = src;

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, SingleStructMat4x4Uniform) {
    auto* src = R"(
struct S {
  m : mat4x4<f32>,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = src;  // Nothing violates std140 layout

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, SingleStructMat2x2Uniform) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, CustomAlignMat3x2) {
    auto* src = R"(
struct S {
  before : i32,
  @align(128) m : mat3x2<f32>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
struct S {
  before : i32,
  @align(128)
  m : mat3x2<f32>,
  after : i32,
}

struct S_std140 {
  before : i32,
  @align(128u)
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  m_2 : vec2<f32>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, CustomSizeMat3x2) {
    auto* src = R"(
struct S {
  before : i32,
  @size(128) m : mat3x2<f32>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
struct S {
  before : i32,
  @size(128)
  m : mat3x2<f32>,
  after : i32,
}

struct S_std140 {
  before : i32,
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(112)
  m_2 : vec2<f32>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, CustomAlignAndSizeMat3x2) {
    auto* src = R"(
struct S {
  before : i32,
  @align(128) @size(128) m : mat3x2<f32>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
struct S {
  before : i32,
  @align(128) @size(128)
  m : mat3x2<f32>,
  after : i32,
}

struct S_std140 {
  before : i32,
  @align(128u)
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(112)
  m_2 : vec2<f32>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMatricesUniform) {
    auto* src = R"(
struct S2x2 {
  m : mat2x2<f32>,
}
struct S3x2 {
  m : mat3x2<f32>,
}
struct S4x2 {
  m : mat4x2<f32>,
}
struct S2x3 {
  m : mat2x3<f32>,
}
struct S3x3 {
  m : mat3x3<f32>,
}
struct S4x3 {
  m : mat4x3<f32>,
}
struct S2x4 {
  m : mat2x4<f32>,
}
struct S3x4 {
  m : mat3x4<f32>,
}
struct S4x4 {
  m : mat4x4<f32>,
}

@group(2) @binding(2) var<uniform> s2x2 : S2x2;
@group(3) @binding(2) var<uniform> s3x2 : S3x2;
@group(4) @binding(2) var<uniform> s4x2 : S4x2;
@group(2) @binding(3) var<uniform> s2x3 : S2x3;
@group(3) @binding(3) var<uniform> s3x3 : S3x3;
@group(4) @binding(3) var<uniform> s4x3 : S4x3;
@group(2) @binding(4) var<uniform> s2x4 : S2x4;
@group(3) @binding(4) var<uniform> s3x4 : S3x4;
@group(4) @binding(4) var<uniform> s4x4 : S4x4;
)";

    auto* expect = R"(
struct S2x2 {
  m : mat2x2<f32>,
}

struct S2x2_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

struct S3x2 {
  m : mat3x2<f32>,
}

struct S3x2_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  m_2 : vec2<f32>,
}

struct S4x2 {
  m : mat4x2<f32>,
}

struct S4x2_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  m_2 : vec2<f32>,
  m_3 : vec2<f32>,
}

struct S2x3 {
  m : mat2x3<f32>,
}

struct S3x3 {
  m : mat3x3<f32>,
}

struct S4x3 {
  m : mat4x3<f32>,
}

struct S2x4 {
  m : mat2x4<f32>,
}

struct S3x4 {
  m : mat3x4<f32>,
}

struct S4x4 {
  m : mat4x4<f32>,
}

@group(2) @binding(2) var<uniform> s2x2 : S2x2_std140;

@group(3) @binding(2) var<uniform> s3x2 : S3x2_std140;

@group(4) @binding(2) var<uniform> s4x2 : S4x2_std140;

@group(2) @binding(3) var<uniform> s2x3 : S2x3;

@group(3) @binding(3) var<uniform> s3x3 : S3x3;

@group(4) @binding(3) var<uniform> s4x3 : S4x3;

@group(2) @binding(4) var<uniform> s2x4 : S2x4;

@group(3) @binding(4) var<uniform> s3x4 : S3x4;

@group(4) @binding(4) var<uniform> s4x4 : S4x4;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_NameCollision) {
    auto* src = R"(
struct S {
  m_1 : i32,
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
struct S {
  m_1 : i32,
  m : mat2x2<f32>,
}

struct S_std140 {
  m_1 : i32,
  m__0 : vec2<f32>,
  m__1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadStruct) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s;
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_S(val : S_std140) -> S {
  return S(mat2x2<f32>(val.m_0, val.m_1));
}

fn f() {
  let l = conv_S(s);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadMatrix) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m;
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m() -> mat2x2<f32> {
  let s = &(s);
  return mat2x2<f32>((*(s)).m_0, (*(s)).m_1);
}

fn f() {
  let l = load_s_m();
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadColumn0) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[0];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadColumn1) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[1];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadColumnI) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0(p0 : u32) -> vec2<f32> {
  switch(p0) {
    case 0u: {
      return s.m_0;
    }
    case 1u: {
      return s.m_1;
    }
    default: {
      return vec2<f32>();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_s_m_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalar00) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[0][0];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_0[0u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalar10) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[1][0];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_1[0u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalarI0) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I][0];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_0(p0 : u32) -> f32 {
  switch(p0) {
    case 0u: {
      return s.m_0[0u];
    }
    case 1u: {
      return s.m_1[0u];
    }
    default: {
      return f32();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_s_m_p0_0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalar01) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[0][1];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_0[1u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalar11) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[1][1];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_1[1u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalarI1) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I][1];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_1(p0 : u32) -> f32 {
  switch(p0) {
    case 0u: {
      return s.m_0[1u];
    }
    case 1u: {
      return s.m_1[1u];
    }
    default: {
      return f32();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_s_m_p0_1(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalar0I) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[0][I];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let I = 0;
  let l = s.m_0[I];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalar1I) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[1][I];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let I = 0;
  let l = s.m_1[I];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructMat2x2Uniform_LoadScalarII) {
    auto* src = R"(
struct S {
  m : mat2x2<f32>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I][I];
}
)";

    auto* expect = R"(
struct S {
  m : mat2x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_p1(p0 : u32, p1 : u32) -> f32 {
  switch(p0) {
    case 0u: {
      return s.m_0[p1];
    }
    case 1u: {
      return s.m_1[p1];
    }
    default: {
      return f32();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_s_m_p0_p1(u32(I), u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadArray) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a;
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(mat3x2<f32>(val.m_0, val.m_1, val.m_2));
}

fn conv_arr3_S(val : array<S_std140, 3u>) -> array<S, 3u> {
  var arr : array<S, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_S(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_S(a);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadStruct0) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[0];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(mat3x2<f32>(val.m_0, val.m_1, val.m_2));
}

fn f() {
  let l = conv_S(a[0u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadStruct1) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[1];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(mat3x2<f32>(val.m_0, val.m_1, val.m_2));
}

fn f() {
  let l = conv_S(a[1u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadStructI) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(mat3x2<f32>(val.m_0, val.m_1, val.m_2));
}

fn f() {
  let I = 1;
  let l = conv_S(a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrix0) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[0].m;
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_0_m() -> mat3x2<f32> {
  let s = &(a[0u]);
  return mat3x2<f32>((*(s)).m_0, (*(s)).m_1, (*(s)).m_2);
}

fn f() {
  let l = load_a_0_m();
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrix1) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[1].m;
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_1_m() -> mat3x2<f32> {
  let s = &(a[1u]);
  return mat3x2<f32>((*(s)).m_0, (*(s)).m_1, (*(s)).m_2);
}

fn f() {
  let l = load_a_1_m();
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrixI) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m;
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_p0_m(p0 : u32) -> mat3x2<f32> {
  let s = &(a[p0]);
  return mat3x2<f32>((*(s)).m_0, (*(s)).m_1, (*(s)).m_2);
}

fn f() {
  let I = 1;
  let l = load_a_p0_m(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrix0Column0) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[0].m[0];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let l = a[0u].m_0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrix1Column0) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[1].m[0];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let l = a[1u].m_0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrixIColumn0) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m[0];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let I = 1;
  let l = a[I].m_0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrix0Column1) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[0].m[1];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let l = a[0u].m_1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrix1Column1) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[1].m[1];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let l = a[1u].m_1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_LoadMatrixIColumnI) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m[I];
}
)";

    auto* expect = R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_p0_m_p1(p0 : u32, p1 : u32) -> vec2<f32> {
  switch(p1) {
    case 0u: {
      return a[p0].m_0;
    }
    case 1u: {
      return a[p0].m_1;
    }
    case 2u: {
      return a[p0].m_2;
    }
    default: {
      return vec2<f32>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0_m_p1(u32(I), u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat4x2Uniform_LoadMatrix) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> m : mat4x2<f32>;

fn f() {
  let l = m;
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> m : mat4x2_f32;

fn conv_mat4x2_f32(val : mat4x2_f32) -> mat4x2<f32> {
  return mat4x2<f32>(val.col0, val.col1, val.col2, val.col3);
}

fn f() {
  let l = conv_mat4x2_f32(m);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat2x2Uniform_LoadColumn0) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : mat2x2<f32>;

fn f() {
  let l = a[0];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : mat2x2_f32;

fn f() {
  let l = a.col0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat4x2Uniform_LoadColumn1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : mat4x2<f32>;

fn f() {
  let l = a[1];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : mat4x2_f32;

fn f() {
  let l = a.col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat2x2Uniform_LoadColumnI) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : mat2x2<f32>;

fn f() {
  let I = 1;

  let l = a[I];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : mat2x2_f32;

fn load_a_p0(p0 : u32) -> vec2<f32> {
  switch(p0) {
    case 0u: {
      return a.col0;
    }
    case 1u: {
      return a.col1;
    }
    default: {
      return vec2<f32>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat2x2Uniform_LoadColumn1Swizzle) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : mat2x2<f32>;

fn f() {
  let l = a[1].yx;
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : mat2x2_f32;

fn f() {
  let l = a.col1.yx;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat4x2Uniform_LoadColumnISwizzle) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : mat4x2<f32>;

fn f() {
  let I = 1;

  let l = a[I].yx;
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : mat4x2_f32;

fn load_a_p0_yx(p0 : u32) -> vec2<f32> {
  switch(p0) {
    case 0u: {
      return a.col0.yx;
    }
    case 1u: {
      return a.col1.yx;
    }
    case 2u: {
      return a.col2.yx;
    }
    case 3u: {
      return a.col3.yx;
    }
    default: {
      return vec2<f32>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0_yx(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat2x2Uniform_LoadColumn1Element1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : mat2x2<f32>;

fn f() {
  let l = a[1][1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : mat2x2_f32;

fn f() {
  let l = a.col1[1u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, Mat4x2Uniform_LoadColumnIElementI) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : mat4x2<f32>;

fn f() {
  let I = 1;

  let l = a[I][I];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : mat4x2_f32;

fn load_a_p0_p1(p0 : u32, p1 : u32) -> f32 {
  switch(p0) {
    case 0u: {
      return a.col0[p1];
    }
    case 1u: {
      return a.col1[p1];
    }
    case 2u: {
      return a.col2[p1];
    }
    case 3u: {
      return a.col3[p1];
    }
    default: {
      return f32();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0_p1(u32(I), u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat2x2Uniform_LoadArray) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat2x2<f32>, 3>;

fn f() {
  let l = a;
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x2_f32, 3u>;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn conv_arr3_mat2x2_f32(val : array<mat2x2_f32, 3u>) -> array<mat2x2<f32>, 3u> {
  var arr : array<mat2x2<f32>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x2_f32(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat2x2_f32(a);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat4x2Uniform_LoadMatrix0) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat4x2<f32>, 3>;

fn f() {
  let l = a[0];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat4x2_f32, 3u>;

fn conv_mat4x2_f32(val : mat4x2_f32) -> mat4x2<f32> {
  return mat4x2<f32>(val.col0, val.col1, val.col2, val.col3);
}

fn f() {
  let l = conv_mat4x2_f32(a[0u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat2x2Uniform_LoadMatrix1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat2x2<f32>, 3>;

fn f() {
  let l = a[1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x2_f32, 3u>;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn f() {
  let l = conv_mat2x2_f32(a[1u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat4x2Uniform_LoadMatrixI) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat4x2<f32>, 3>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat4x2_f32, 3u>;

fn conv_mat4x2_f32(val : mat4x2_f32) -> mat4x2<f32> {
  return mat4x2<f32>(val.col0, val.col1, val.col2, val.col3);
}

fn f() {
  let I = 1;
  let l = conv_mat4x2_f32(a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat2x2Uniform_LoadMatrix1Column0) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat2x2<f32>, 3>;

fn f() {
  let l = a[1][0];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x2_f32, 3u>;

fn f() {
  let l = a[1u].col0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat4x2Uniform_LoadMatrix0Column1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat4x2<f32>, 3>;

fn f() {
  let l = a[0][1];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat4x2_f32, 3u>;

fn f() {
  let l = a[0u].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat2x2Uniform_LoadMatrixIColumn1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat2x2<f32>, 3>;

fn f() {
  let I = 1;
  let l = a[I][1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x2_f32, 3u>;

fn f() {
  let I = 1;
  let l = a[I].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayMat4x2Uniform_LoadMatrix1ColumnI) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<mat4x2<f32>, 3>;

fn f() {
  let I = 1;
  let l = a[1][I];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<mat4x2_f32, 3u>;

fn load_a_1_p0(p0 : u32) -> vec2<f32> {
  switch(p0) {
    case 0u: {
      return a[1u].col0;
    }
    case 1u: {
      return a[1u].col1;
    }
    case 2u: {
      return a[1u].col2;
    }
    case 3u: {
      return a[1u].col3;
    }
    default: {
      return vec2<f32>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_1_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat2x2Uniform_LoadArrays) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat2x2<f32>, 3>, 4>;

fn f() {
  let l = a;
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x2_f32, 3u>, 4u>;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn conv_arr3_mat2x2_f32(val : array<mat2x2_f32, 3u>) -> array<mat2x2<f32>, 3u> {
  var arr : array<mat2x2<f32>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x2_f32(val[i]);
  }
  return arr;
}

fn conv_arr4_arr3_mat2x2_f32(val : array<array<mat2x2_f32, 3u>, 4u>) -> array<array<mat2x2<f32>, 3u>, 4u> {
  var arr : array<array<mat2x2<f32>, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_arr3_mat2x2_f32(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr4_arr3_mat2x2_f32(a);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat4x2Uniform_LoadArray0) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat4x2<f32>, 3>, 4>;

fn f() {
  let l = a[0];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat4x2_f32, 3u>, 4u>;

fn conv_mat4x2_f32(val : mat4x2_f32) -> mat4x2<f32> {
  return mat4x2<f32>(val.col0, val.col1, val.col2, val.col3);
}

fn conv_arr3_mat4x2_f32(val : array<mat4x2_f32, 3u>) -> array<mat4x2<f32>, 3u> {
  var arr : array<mat4x2<f32>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat4x2_f32(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat4x2_f32(a[0u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat2x2Uniform_LoadArray1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat2x2<f32>, 3>,4>;

fn f() {
  let l = a[1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x2_f32, 3u>, 4u>;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn conv_arr3_mat2x2_f32(val : array<mat2x2_f32, 3u>) -> array<mat2x2<f32>, 3u> {
  var arr : array<mat2x2<f32>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x2_f32(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat2x2_f32(a[1u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat2x2Uniform_LoadArrayI) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat2x2<f32>, 3>,4>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x2_f32, 3u>, 4u>;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn conv_arr3_mat2x2_f32(val : array<mat2x2_f32, 3u>) -> array<mat2x2<f32>, 3u> {
  var arr : array<mat2x2<f32>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x2_f32(val[i]);
  }
  return arr;
}

fn f() {
  let I = 1;
  let l = conv_arr3_mat2x2_f32(a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}
TEST_F(Std140Test, ArrayArrayMat2x2Uniform_LoadMatrix12Column0) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat2x2<f32>, 3>, 4>;

fn f() {
  let l = a[1][2][0];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x2_f32, 3u>, 4u>;

fn f() {
  let l = a[1u][2u].col0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat4x2Uniform_LoadMatrix2IColumn1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat4x2<f32>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[2][I][1];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat4x2_f32, 3u>, 4u>;

fn f() {
  let I = 1;
  let l = a[2u][I].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat2x2Uniform_LoadMatrixI2Column1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat2x2<f32>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][2][1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x2_f32, 3u>, 4u>;

fn f() {
  let I = 1;
  let l = a[I][2u].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat2x2Uniform_LoadMatrixIIColumn1) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat2x2<f32>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][I][1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x2_f32, 3u>, 4u>;

fn f() {
  let I = 1;
  let l = a[I][I].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayArrayMat4x2Uniform_LoadMatrix12ColumnI) {
    auto* src = R"(
@group(0) @binding(0) var<uniform> a : array<array<mat4x2<f32>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[1][2][I];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat4x2_f32, 3u>, 4u>;

fn load_a_1_2_p0(p0 : u32) -> vec2<f32> {
  switch(p0) {
    case 0u: {
      return a[1u][2u].col0;
    }
    case 1u: {
      return a[1u][2u].col1;
    }
    case 2u: {
      return a[1u][2u].col2;
    }
    case 3u: {
      return a[1u][2u].col3;
    }
    default: {
      return vec2<f32>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_1_2_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat2x2Uniform_LoadStruct) {
    auto* src = R"(
struct S {
  a : array<mat2x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s;
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

struct S {
  a : array<mat2x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat2x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn conv_arr3_mat2x2_f32(val : array<mat2x2_f32, 3u>) -> array<mat2x2<f32>, 3u> {
  var arr : array<mat2x2<f32>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x2_f32(val[i]);
  }
  return arr;
}

fn conv_S(val : S_std140) -> S {
  return S(conv_arr3_mat2x2_f32(val.a));
}

fn f() {
  let l = conv_S(s);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat2x2Uniform_LoadArray) {
    auto* src = R"(
struct S {
  a : array<mat2x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a;
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

struct S {
  a : array<mat2x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat2x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn conv_arr3_mat2x2_f32(val : array<mat2x2_f32, 3u>) -> array<mat2x2<f32>, 3u> {
  var arr : array<mat2x2<f32>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x2_f32(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat2x2_f32(s.a);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat4x2Uniform_LoadMatrix0) {
    auto* src = R"(
struct S {
  a : array<mat4x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[0];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

struct S {
  a : array<mat4x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat4x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat4x2_f32(val : mat4x2_f32) -> mat4x2<f32> {
  return mat4x2<f32>(val.col0, val.col1, val.col2, val.col3);
}

fn f() {
  let l = conv_mat4x2_f32(s.a[0u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat2x2Uniform_LoadMatrix1) {
    auto* src = R"(
struct S {
  a : array<mat2x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

struct S {
  a : array<mat2x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat2x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat2x2_f32(val : mat2x2_f32) -> mat2x2<f32> {
  return mat2x2<f32>(val.col0, val.col1);
}

fn f() {
  let l = conv_mat2x2_f32(s.a[1u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat4x2Uniform_LoadMatrixI) {
    auto* src = R"(
struct S {
  a : array<mat4x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

struct S {
  a : array<mat4x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat4x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat4x2_f32(val : mat4x2_f32) -> mat4x2<f32> {
  return mat4x2<f32>(val.col0, val.col1, val.col2, val.col3);
}

fn f() {
  let I = 1;
  let l = conv_mat4x2_f32(s.a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat2x2Uniform_LoadMatrix1Column0) {
    auto* src = R"(
struct S {
  a : array<mat2x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[1][0];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

struct S {
  a : array<mat2x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat2x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.a[1u].col0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat4x2Uniform_LoadMatrix0Column1) {
    auto* src = R"(
struct S {
  a : array<mat4x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[0][1];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

struct S {
  a : array<mat4x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat4x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.a[0u].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat2x2Uniform_LoadMatrixIColumn1) {
    auto* src = R"(
struct S {
  a : array<mat2x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I][1];
}
)";

    auto* expect = R"(
struct mat2x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
}

struct S {
  a : array<mat2x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat2x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let I = 1;
  let l = s.a[I].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, StructArrayMat4x2Uniform_LoadMatrix1ColumnI) {
    auto* src = R"(
struct S {
  a : array<mat4x2<f32>, 3>,
};

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[1][I];
}
)";

    auto* expect = R"(
struct mat4x2_f32 {
  col0 : vec2<f32>,
  col1 : vec2<f32>,
  col2 : vec2<f32>,
  col3 : vec2<f32>,
}

struct S {
  a : array<mat4x2<f32>, 3>,
}

struct S_std140 {
  a : array<mat4x2_f32, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_a_1_p0(p0 : u32) -> vec2<f32> {
  switch(p0) {
    case 0u: {
      return s.a[1u].col0;
    }
    case 1u: {
      return s.a[1u].col1;
    }
    case 2u: {
      return s.a[1u].col2;
    }
    case 3u: {
      return s.a[1u].col3;
    }
    default: {
      return vec2<f32>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_s_a_1_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructArrayStructMat4x2Uniform_Loads) {
    auto* src = R"(
struct Inner {
  m : mat4x2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

fn f() {
  let I = 1;

  let l_a             : array<Outer, 4>  = a;
  let l_a_1           : Outer            = a[1];
  let l_a_2_a         : array<Inner, 4>  = a[2].a;
  let l_a_3_a_1       : Inner            = a[3].a[1];
  let l_a_0_a_2_m     : mat4x2<f32>      = a[0].a[2].m;
  let l_a_1_a_3_m_0   : vec2<f32>        = a[1].a[3].m[0];
  let l_a_2_a_0_m_1_0 : f32              = a[2].a[0].m[1][0];
}
)";

    auto* expect = R"(
struct Inner {
  m : mat4x2<f32>,
}

struct Inner_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  m_2 : vec2<f32>,
  m_3 : vec2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

struct Outer_std140 {
  a : array<Inner_std140, 4u>,
}

@group(0) @binding(0) var<uniform> a : array<Outer_std140, 4u>;

fn conv_Inner(val : Inner_std140) -> Inner {
  return Inner(mat4x2<f32>(val.m_0, val.m_1, val.m_2, val.m_3));
}

fn conv_arr4_Inner(val : array<Inner_std140, 4u>) -> array<Inner, 4u> {
  var arr : array<Inner, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Inner(val[i]);
  }
  return arr;
}

fn conv_Outer(val : Outer_std140) -> Outer {
  return Outer(conv_arr4_Inner(val.a));
}

fn conv_arr4_Outer(val : array<Outer_std140, 4u>) -> array<Outer, 4u> {
  var arr : array<Outer, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Outer(val[i]);
  }
  return arr;
}

fn load_a_0_a_2_m() -> mat4x2<f32> {
  let s = &(a[0u].a[2u]);
  return mat4x2<f32>((*(s)).m_0, (*(s)).m_1, (*(s)).m_2, (*(s)).m_3);
}

fn f() {
  let I = 1;
  let l_a : array<Outer, 4> = conv_arr4_Outer(a);
  let l_a_1 : Outer = conv_Outer(a[1u]);
  let l_a_2_a : array<Inner, 4> = conv_arr4_Inner(a[2u].a);
  let l_a_3_a_1 : Inner = conv_Inner(a[3u].a[1u]);
  let l_a_0_a_2_m : mat4x2<f32> = load_a_0_a_2_m();
  let l_a_1_a_3_m_0 : vec2<f32> = a[1u].a[3u].m_0;
  let l_a_2_a_0_m_1_0 : f32 = a[2u].a[0u].m_1[0u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructArrayStructMat4x2Uniform_LoadsViaPtrs) {
    // Note: Std140Test requires the PromoteSideEffectsToDecl transform to have been run first, so
    // side-effects in the let-chain will not be a problem.
    auto* src = R"(
struct Inner {
  m : mat4x2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

fn f() {
  let I = 1;

  let p_a = &a;
  let p_a_3 = &((*p_a)[3]);
  let p_a_3_a = &((*p_a_3).a);
  let p_a_3_a_2 = &((*p_a_3_a)[2]);
  let p_a_3_a_2_m = &((*p_a_3_a_2).m);
  let p_a_3_a_2_m_1 = &((*p_a_3_a_2_m)[1]);


  let l_a             : array<Outer, 4> = *p_a;
  let l_a_3           : Outer           = *p_a_3;
  let l_a_3_a         : array<Inner, 4> = *p_a_3_a;
  let l_a_3_a_2       : Inner           = *p_a_3_a_2;
  let l_a_3_a_2_m     : mat4x2<f32>     = *p_a_3_a_2_m;
  let l_a_3_a_2_m_1   : vec2<f32>       = *p_a_3_a_2_m_1;
  let l_a_2_a_0_m_1_0 : f32             = (*p_a_3_a_2_m_1)[0];
}
)";

    auto* expect = R"(
struct Inner {
  m : mat4x2<f32>,
}

struct Inner_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  m_2 : vec2<f32>,
  m_3 : vec2<f32>,
}

struct Outer {
  a : array<Inner, 4>,
}

struct Outer_std140 {
  a : array<Inner_std140, 4u>,
}

@group(0) @binding(0) var<uniform> a : array<Outer_std140, 4u>;

fn conv_Inner(val : Inner_std140) -> Inner {
  return Inner(mat4x2<f32>(val.m_0, val.m_1, val.m_2, val.m_3));
}

fn conv_arr4_Inner(val : array<Inner_std140, 4u>) -> array<Inner, 4u> {
  var arr : array<Inner, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Inner(val[i]);
  }
  return arr;
}

fn conv_Outer(val : Outer_std140) -> Outer {
  return Outer(conv_arr4_Inner(val.a));
}

fn conv_arr4_Outer(val : array<Outer_std140, 4u>) -> array<Outer, 4u> {
  var arr : array<Outer, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_Outer(val[i]);
  }
  return arr;
}

fn load_a_3_a_2_m() -> mat4x2<f32> {
  let s = &(a[3u].a[2u]);
  return mat4x2<f32>((*(s)).m_0, (*(s)).m_1, (*(s)).m_2, (*(s)).m_3);
}

fn f() {
  let I = 1;
  let p_a = conv_arr4_Outer(a);
  let p_a_3 = conv_Outer(a[3u]);
  let p_a_3_a = conv_arr4_Inner(a[3u].a);
  let p_a_3_a_2 = conv_Inner(a[3u].a[2u]);
  let p_a_3_a_2_m = load_a_3_a_2_m();
  let p_a_3_a_2_m_1 = a[3u].a[2u].m_1;
  let l_a : array<Outer, 4> = conv_arr4_Outer(a);
  let l_a_3 : Outer = conv_Outer(a[3u]);
  let l_a_3_a : array<Inner, 4> = conv_arr4_Inner(a[3u].a);
  let l_a_3_a_2 : Inner = conv_Inner(a[3u].a[2u]);
  let l_a_3_a_2_m : mat4x2<f32> = load_a_3_a_2_m();
  let l_a_3_a_2_m_1 : vec2<f32> = a[3u].a[2u].m_1;
  let l_a_2_a_0_m_1_0 : f32 = a[3u].a[2u].m_1[0u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_CopyArray_UniformToStorage) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
    s = u;
}
)";

    auto* expect =
        R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn conv_S(val : S_std140) -> S {
  return S(mat3x2<f32>(val.m_0, val.m_1, val.m_2));
}

fn conv_arr4_S(val : array<S_std140, 4u>) -> array<S, 4u> {
  var arr : array<S, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_S(val[i]);
  }
  return arr;
}

fn f() {
  s = conv_arr4_S(u);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_CopyStruct_UniformToWorkgroup) {
    auto* src = R"(
struct S {
  v : vec4<i32>,
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
var<workgroup> w : array<S, 4>;

fn f() {
    w[0] = u[1];
}
)";

    auto* expect =
        R"(
struct S {
  v : vec4<i32>,
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  v : vec4<i32>,
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<workgroup> w : array<S, 4>;

fn conv_S(val : S_std140) -> S {
  return S(val.v, mat3x2<f32>(val.m_0, val.m_1, val.m_2));
}

fn f() {
  w[0] = conv_S(u[1u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_CopyMatrix_UniformToPrivate) {
    auto* src = R"(
struct S {
  v : vec4<i32>,
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
var<private> p : array<S, 4>;

fn f() {
    p[2].m = u[1].m;
}
)";

    auto* expect = R"(
struct S {
  v : vec4<i32>,
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  v : vec4<i32>,
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<private> p : array<S, 4>;

fn load_u_1_m() -> mat3x2<f32> {
  let s = &(u[1u]);
  return mat3x2<f32>((*(s)).m_0, (*(s)).m_1, (*(s)).m_2);
}

fn f() {
  p[2].m = load_u_1_m();
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_CopyColumn_UniformToStorage) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
    s[3].m[1] = u[2].m[0];
}
)";

    auto* expect =
        R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
  s[3].m[1] = u[2u].m_0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_CopySwizzle_UniformToWorkgroup) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
var<workgroup> w : array<S, 4>;

fn f() {
    w[3].m[1] = u[2].m[0].yx.xy;
}
)";

    auto* expect =
        R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<workgroup> w : array<S, 4>;

fn f() {
  w[3].m[1] = u[2u].m_0.yx.xy;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, ArrayStructMat3x2Uniform_CopyScalar_UniformToPrivate) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;
var<private> w : array<S, 4>;

fn f() {
    w[3].m[1].x = u[2].m[0].y;
}
)";

    auto* expect =
        R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<private> w : array<S, 4>;

fn f() {
  w[3].m[1].x = u[2u].m_0[1u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test, MatrixUsageInForLoop) {
    auto* src = R"(
struct S {
  @size(64) m : mat3x2<f32>,
}

@group(0) @binding(0) var<uniform> u : S;

fn f() {
    for (var i = u32(u.m[0][0]); i < u32(u.m[i][1]); i += u32(u.m[1][i])) {
    }
}
)";

    auto* expect =
        R"(
struct S {
  @size(64)
  m : mat3x2<f32>,
}

struct S_std140 {
  m_0 : vec2<f32>,
  m_1 : vec2<f32>,
  @size(48)
  m_2 : vec2<f32>,
}

@group(0) @binding(0) var<uniform> u : S_std140;

fn load_u_m_p0_1(p0 : u32) -> f32 {
  switch(p0) {
    case 0u: {
      return u.m_0[1u];
    }
    case 1u: {
      return u.m_1[1u];
    }
    case 2u: {
      return u.m_2[1u];
    }
    default: {
      return f32();
    }
  }
}

fn f() {
  for(var i = u32(u.m_0[0u]); (i < u32(load_u_m_p0_1(u32(i)))); i += u32(u.m_1[i])) {
  }
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
