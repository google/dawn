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

using Std140Test_F16 = TransformTest;

TEST_F(Std140Test_F16, StructMatricesUniform) {
    auto* src = R"(
enable f16;

struct S2x2F16 {
  m : mat2x2<f16>,
}
struct S3x2F16 {
  m : mat3x2<f16>,
}
struct S4x2F16 {
  m : mat4x2<f16>,
}
struct S2x3F16 {
  m : mat2x3<f16>,
}
struct S3x3F16 {
  m : mat3x3<f16>,
}
struct S4x3F16 {
  m : mat4x3<f16>,
}
struct S2x4F16 {
  m : mat2x4<f16>,
}
struct S3x4F16 {
  m : mat3x4<f16>,
}
struct S4x4F16 {
  m : mat4x4<f16>,
}

@group(2) @binding(2) var<uniform> s2x2f16 : S2x2F16;
@group(3) @binding(2) var<uniform> s3x2f16 : S3x2F16;
@group(4) @binding(2) var<uniform> s4x2f16 : S4x2F16;
@group(2) @binding(3) var<uniform> s2x3f16 : S2x3F16;
@group(3) @binding(3) var<uniform> s3x3f16 : S3x3F16;
@group(4) @binding(3) var<uniform> s4x3f16 : S4x3F16;
@group(2) @binding(4) var<uniform> s2x4f16 : S2x4F16;
@group(3) @binding(4) var<uniform> s3x4f16 : S3x4F16;
@group(4) @binding(4) var<uniform> s4x4f16 : S4x4F16;
)";

    auto* expect = R"(
enable f16;

struct S2x2F16 {
  m : mat2x2<f16>,
}

struct S2x2F16_std140 {
  m_0 : vec2<f16>,
  m_1 : vec2<f16>,
}

struct S3x2F16 {
  m : mat3x2<f16>,
}

struct S3x2F16_std140 {
  m_0 : vec2<f16>,
  m_1 : vec2<f16>,
  m_2 : vec2<f16>,
}

struct S4x2F16 {
  m : mat4x2<f16>,
}

struct S4x2F16_std140 {
  m_0 : vec2<f16>,
  m_1 : vec2<f16>,
  m_2 : vec2<f16>,
  m_3 : vec2<f16>,
}

struct S2x3F16 {
  m : mat2x3<f16>,
}

struct S2x3F16_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

struct S3x3F16 {
  m : mat3x3<f16>,
}

struct S3x3F16_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
  m_2 : vec3<f16>,
}

struct S4x3F16 {
  m : mat4x3<f16>,
}

struct S4x3F16_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
  m_2 : vec3<f16>,
  m_3 : vec3<f16>,
}

struct S2x4F16 {
  m : mat2x4<f16>,
}

struct S2x4F16_std140 {
  m_0 : vec4<f16>,
  m_1 : vec4<f16>,
}

struct S3x4F16 {
  m : mat3x4<f16>,
}

struct S3x4F16_std140 {
  m_0 : vec4<f16>,
  m_1 : vec4<f16>,
  m_2 : vec4<f16>,
}

struct S4x4F16 {
  m : mat4x4<f16>,
}

struct S4x4F16_std140 {
  m_0 : vec4<f16>,
  m_1 : vec4<f16>,
  m_2 : vec4<f16>,
  m_3 : vec4<f16>,
}

@group(2) @binding(2) var<uniform> s2x2f16 : S2x2F16_std140;

@group(3) @binding(2) var<uniform> s3x2f16 : S3x2F16_std140;

@group(4) @binding(2) var<uniform> s4x2f16 : S4x2F16_std140;

@group(2) @binding(3) var<uniform> s2x3f16 : S2x3F16_std140;

@group(3) @binding(3) var<uniform> s3x3f16 : S3x3F16_std140;

@group(4) @binding(3) var<uniform> s4x3f16 : S4x3F16_std140;

@group(2) @binding(4) var<uniform> s2x4f16 : S2x4F16_std140;

@group(3) @binding(4) var<uniform> s3x4f16 : S3x4F16_std140;

@group(4) @binding(4) var<uniform> s4x4f16 : S4x4F16_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

// In the following tests we only test `mat2x3<f16>`, and set all constant column index to 1, row
// index 0, inner array index 2, and outer array index 3. For exhaustive tests, i.e. tests on all
// matrix shape and different valid constant index, please refer to std140_exhaustive_test.cc

TEST_F(Std140Test_F16, SingleStructMatUniform_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, CustomAlign_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  before : i32,
  @align(128)
  m : mat2x3<f16>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
enable f16;

struct S {
  before : i32,
  @align(128)
  m : mat2x3<f16>,
  after : i32,
}

struct S_std140 {
  before : i32,
  @align(128i)
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, CustomSizeMat_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  before : i32,
  @size(128)
  m : mat2x3<f16>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
enable f16;

struct S {
  before : i32,
  @size(128)
  m : mat2x3<f16>,
  after : i32,
}

struct S_std140 {
  before : i32,
  m_0 : vec3<f16>,
  @size(120)
  m_1 : vec3<f16>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, CustomAlignAndSize_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  before : i32,
  @align(128) @size(128)
  m : mat2x3<f16>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
enable f16;

struct S {
  before : i32,
  @align(128) @size(128)
  m : mat2x3<f16>,
  after : i32,
}

struct S_std140 {
  before : i32,
  @align(128i)
  m_0 : vec3<f16>,
  @size(120)
  m_1 : vec3<f16>,
  after : i32,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatrixUsageInForLoop_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  for(var i = u32(s.m[0][0]); (i < u32(s.m[i][1])); i += u32(s.m[1][i])) {
  }
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_1(p0 : u32) -> f16 {
  switch(p0) {
    case 0u: {
      return s.m_0[1u];
    }
    case 1u: {
      return s.m_1[1u];
    }
    default: {
      return f16();
    }
  }
}

fn f() {
  for(var i = u32(s.m_0[0u]); (i < u32(load_s_m_p0_1(u32(i)))); i += u32(s.m_1[i])) {
  }
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadMatrix_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> m : mat2x3<f16>;

fn f() {
  let l = m;
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> m : mat2x3_f16;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let l = conv_mat2x3_f16(m);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadColumn_ConstIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let l = a[1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn f() {
  let l = a.col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadColumn_VariableIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn load_a_p0(p0 : u32) -> vec3<f16> {
  switch(p0) {
    case 0u: {
      return a.col0;
    }
    case 1u: {
      return a.col1;
    }
    default: {
      return vec3<f16>();
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

TEST_F(Std140Test_F16, MatUniform_LoadColumnSwizzle_ConstIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let l = a[1].yzx;
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn f() {
  let l = a.col1.yzx;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadColumnSwizzle_VariableIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let I = 1;
  let l = a[I].yzx;
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn load_a_p0_yzx(p0 : u32) -> vec3<f16> {
  switch(p0) {
    case 0u: {
      return a.col0.yzx;
    }
    case 1u: {
      return a.col1.yzx;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_p0_yzx(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadScalar_ConstColumnIndex_ConstRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let l = a[1][0];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn f() {
  let l = a.col1[0u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadScalar_VariableColumnIndex_ConstRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let I = 0;
  let l = a[I][0];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn load_a_p0_0(p0 : u32) -> f16 {
  switch(p0) {
    case 0u: {
      return a.col0[0u];
    }
    case 1u: {
      return a.col1[0u];
    }
    default: {
      return f16();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_a_p0_0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadScalar_ConstColumnIndex_VariableRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let I = 0;
  let l = a[1][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn f() {
  let I = 0;
  let l = a.col1[I];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, MatUniform_LoadScalar_VariableColumnIndex_VariableRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : mat2x3<f16>;

fn f() {
  let I = 0;
  let l = a[I][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : mat2x3_f16;

fn load_a_p0_p1(p0 : u32, p1 : u32) -> f16 {
  switch(p0) {
    case 0u: {
      return a.col0[p1];
    }
    case 1u: {
      return a.col1[p1];
    }
    default: {
      return f16();
    }
  }
}

fn f() {
  let I = 0;
  let l = load_a_p0_p1(u32(I), u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructMatUniform_NameCollision_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m_1 : i32,
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;
)";

    auto* expect = R"(
enable f16;

struct S {
  m_1 : i32,
  m : mat2x3<f16>,
}

struct S_std140 {
  m_1 : i32,
  m__0 : vec3<f16>,
  m__1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructMatUniform_LoadStruct_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_S(val : S_std140) -> S {
  return S(mat2x3<f16>(val.m_0, val.m_1));
}

fn f() {
  let l = conv_S(s);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructMatUniform_LoadMatrix_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m() -> mat2x3<f16> {
  let s = &(s);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn f() {
  let l = load_s_m();
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructMatUniform_LoadColumn_ConstIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[1];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructMatUniform_LoadColumn_VariableIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0(p0 : u32) -> vec3<f16> {
  switch(p0) {
    case 0u: {
      return s.m_0;
    }
    case 1u: {
      return s.m_1;
    }
    default: {
      return vec3<f16>();
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

TEST_F(Std140Test_F16, StructMatUniform_LoadScalar_ConstColumnIndex_ConstRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.m[1][0];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.m_1[0u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructMatUniform_LoadScalar_VariableColumnIndex_ConstRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I][0];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_0(p0 : u32) -> f16 {
  switch(p0) {
    case 0u: {
      return s.m_0[0u];
    }
    case 1u: {
      return s.m_1[0u];
    }
    default: {
      return f16();
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

TEST_F(Std140Test_F16, StructMatUniform_LoadScalar_ConstColumnIndex_VariableRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[1][I];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
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

TEST_F(Std140Test_F16, StructMatUniform_LoadScalar_VariableColumnIndex_VariableRowIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 0;
  let l = s.m[I][I];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_m_p0_p1(p0 : u32, p1 : u32) -> f16 {
  switch(p0) {
    case 0u: {
      return s.m_0[p1];
    }
    case 1u: {
      return s.m_1[p1];
    }
    default: {
      return f16();
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

TEST_F(Std140Test_F16, ArrayStructMatUniform_LoadArray_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(mat2x3<f16>(val.m_0, val.m_1));
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

TEST_F(Std140Test_F16, ArrayStructMatUniform_LoadStruct_ConstIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[2];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(mat2x3<f16>(val.m_0, val.m_1));
}

fn f() {
  let l = conv_S(a[2u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_LoadStruct_VariableIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn conv_S(val : S_std140) -> S {
  return S(mat2x3<f16>(val.m_0, val.m_1));
}

fn f() {
  let I = 1;
  let l = conv_S(a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_LoadMatrix_ConstArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[2].m;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_2_m() -> mat2x3<f16> {
  let s = &(a[2u]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn f() {
  let l = load_a_2_m();
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_LoadMatrix_VariableArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_p0_m(p0 : u32) -> mat2x3<f16> {
  let s = &(a[p0]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn f() {
  let I = 1;
  let l = load_a_p0_m(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayStructMatUniform_LoadColumn_ConstArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let l = a[2].m[1];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let l = a[2u].m_1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayStructMatUniform_LoadColumn_VariableArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m[1];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn f() {
  let I = 1;
  let l = a[I].m_1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayStructMatUniform_LoadColumn_ConstArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[2].m[I];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_2_m_p0(p0 : u32) -> vec3<f16> {
  switch(p0) {
    case 0u: {
      return a[2u].m_0;
    }
    case 1u: {
      return a[2u].m_1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_2_m_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayStructMatUniform_LoadColumn_VariableArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S, 3>;

fn f() {
  let I = 1;
  let l = a[I].m[I];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<S_std140, 3u>;

fn load_a_p0_m_p1(p0 : u32, p1 : u32) -> vec3<f16> {
  switch(p1) {
    case 0u: {
      return a[p0].m_0;
    }
    case 1u: {
      return a[p0].m_1;
    }
    default: {
      return vec3<f16>();
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

TEST_F(Std140Test_F16, ArrayStructArrayStructMatUniform_Loads_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct Inner {
  @size(64)
  m : mat2x3<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let l_a : array<Outer, 4> = a;
  let l_a_1 : Outer = a[1];
  let l_a_I : Outer = a[I];
  let l_a_2_a : array<Inner, 4> = a[2].a;
  let l_a_I_a : array<Inner, 4> = a[I].a;
  let l_a_3_a_1 : Inner = a[3].a[1];
  let l_a_3_a_I : Inner = a[3].a[I];
  let l_a_I_a_1 : Inner = a[I].a[1];
  let l_a_I_a_J : Inner = a[I].a[J];
  let l_a_0_a_2_m : mat2x3<f16> = a[0].a[2].m;
  let l_a_0_a_I_m : mat2x3<f16> = a[0].a[I].m;
  let l_a_I_a_2_m : mat2x3<f16> = a[I].a[2].m;
  let l_a_I_a_J_m : mat2x3<f16> = a[I].a[J].m;
  let l_a_1_a_3_m_0 : vec3<f16> = a[1].a[3].m[0];
  let l_a_I_a_J_m_K : vec3<f16> = a[I].a[J].m[K];
  let l_a_2_a_0_m_1_0 : f16 = a[2].a[0].m[1][0];
  let l_a_I_a_J_m_K_I : f16 = a[I].a[J].m[K][I];
}
)";

    auto* expect = R"(
enable f16;

struct Inner {
  @size(64)
  m : mat2x3<f16>,
}

struct Inner_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

struct Outer_std140 {
  a : array<Inner_std140, 4u>,
}

@group(0) @binding(0) var<uniform> a : array<Outer_std140, 4u>;

fn conv_Inner(val : Inner_std140) -> Inner {
  return Inner(mat2x3<f16>(val.m_0, val.m_1));
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

fn load_a_0_a_2_m() -> mat2x3<f16> {
  let s = &(a[0u].a[2u]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_0_a_p0_m(p0 : u32) -> mat2x3<f16> {
  let s = &(a[0u].a[p0]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_p0_a_2_m(p0 : u32) -> mat2x3<f16> {
  let s = &(a[p0].a[2u]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_p0_a_p1_m(p0 : u32, p1 : u32) -> mat2x3<f16> {
  let s = &(a[p0].a[p1]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_p0_a_p1_m_p2(p0 : u32, p1 : u32, p2 : u32) -> vec3<f16> {
  switch(p2) {
    case 0u: {
      return a[p0].a[p1].m_0;
    }
    case 1u: {
      return a[p0].a[p1].m_1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn load_a_p0_a_p1_m_p2_p3(p0 : u32, p1 : u32, p2 : u32, p3 : u32) -> f16 {
  switch(p2) {
    case 0u: {
      return a[p0].a[p1].m_0[p3];
    }
    case 1u: {
      return a[p0].a[p1].m_1[p3];
    }
    default: {
      return f16();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let l_a : array<Outer, 4> = conv_arr4_Outer(a);
  let l_a_1 : Outer = conv_Outer(a[1u]);
  let l_a_I : Outer = conv_Outer(a[I]);
  let l_a_2_a : array<Inner, 4> = conv_arr4_Inner(a[2u].a);
  let l_a_I_a : array<Inner, 4> = conv_arr4_Inner(a[I].a);
  let l_a_3_a_1 : Inner = conv_Inner(a[3u].a[1u]);
  let l_a_3_a_I : Inner = conv_Inner(a[3u].a[I]);
  let l_a_I_a_1 : Inner = conv_Inner(a[I].a[1u]);
  let l_a_I_a_J : Inner = conv_Inner(a[I].a[J]);
  let l_a_0_a_2_m : mat2x3<f16> = load_a_0_a_2_m();
  let l_a_0_a_I_m : mat2x3<f16> = load_a_0_a_p0_m(u32(I));
  let l_a_I_a_2_m : mat2x3<f16> = load_a_p0_a_2_m(u32(I));
  let l_a_I_a_J_m : mat2x3<f16> = load_a_p0_a_p1_m(u32(I), u32(J));
  let l_a_1_a_3_m_0 : vec3<f16> = a[1u].a[3u].m_0;
  let l_a_I_a_J_m_K : vec3<f16> = load_a_p0_a_p1_m_p2(u32(I), u32(J), u32(K));
  let l_a_2_a_0_m_1_0 : f16 = a[2u].a[0u].m_1[0u];
  let l_a_I_a_J_m_K_I : f16 = load_a_p0_a_p1_m_p2_p3(u32(I), u32(J), u32(K), u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructArrayStructMatUniform_LoadsViaPtrs_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct Inner {
  @size(64)
  m : mat2x3<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

@group(0) @binding(0) var<uniform> a : array<Outer, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let p_a = &(a);
  let p_a_3 = &((*(p_a))[3]);
  let p_a_I = &((*(p_a))[I]);
  let p_a_3_a = &((*(p_a_3)).a);
  let p_a_I_a = &((*(p_a_I)).a);
  let p_a_3_a_2 = &((*(p_a_3_a))[2]);
  let p_a_3_a_I = &((*(p_a_3_a))[I]);
  let p_a_I_a_2 = &((*(p_a_I_a))[2]);
  let p_a_I_a_J = &((*(p_a_I_a))[J]);
  let p_a_3_a_2_m = &((*(p_a_3_a_2)).m);
  let p_a_3_a_I_m = &((*(p_a_3_a_I)).m);
  let p_a_I_a_2_m = &((*(p_a_I_a_2)).m);
  let p_a_I_a_J_m = &((*(p_a_I_a_J)).m);
  let p_a_3_a_2_m_1 = &((*(p_a_3_a_2_m))[1]);
  let p_a_I_a_J_m_K = &((*(p_a_I_a_J_m))[K]);
  let l_a : array<Outer, 4> = *(p_a);
  let l_a_3 : Outer = *(p_a_3);
  let l_a_I : Outer = *(p_a_I);
  let l_a_3_a : array<Inner, 4> = *(p_a_3_a);
  let l_a_I_a : array<Inner, 4> = *(p_a_I_a);
  let l_a_3_a_2 : Inner = *(p_a_3_a_2);
  let l_a_3_a_I : Inner = *(p_a_3_a_I);
  let l_a_I_a_2 : Inner = *(p_a_I_a_2);
  let l_a_I_a_J : Inner = *(p_a_I_a_J);
  let l_a_3_a_2_m : mat2x3<f16> = *(p_a_3_a_2_m);
  let l_a_3_a_I_m : mat2x3<f16> = *(p_a_3_a_I_m);
  let l_a_I_a_2_m : mat2x3<f16> = *(p_a_I_a_2_m);
  let l_a_I_a_J_m : mat2x3<f16> = *(p_a_I_a_J_m);
  let l_a_3_a_2_m_1 : vec3<f16> = *(p_a_3_a_2_m_1);
  let l_a_I_a_J_m_K : vec3<f16> = *(p_a_I_a_J_m_K);
  let l_a_2_a_0_m_1_0 : f16 = (*(p_a_3_a_2_m_1))[0];
  let l_a_I_a_J_m_K_I : f16 = (*(p_a_I_a_J_m_K))[I];
}
)";

    auto* expect = R"(
enable f16;

struct Inner {
  @size(64)
  m : mat2x3<f16>,
}

struct Inner_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

struct Outer {
  a : array<Inner, 4>,
}

struct Outer_std140 {
  a : array<Inner_std140, 4u>,
}

@group(0) @binding(0) var<uniform> a : array<Outer_std140, 4u>;

fn conv_Inner(val : Inner_std140) -> Inner {
  return Inner(mat2x3<f16>(val.m_0, val.m_1));
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

fn load_a_3_a_2_m() -> mat2x3<f16> {
  let s = &(a[3u].a[2u]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_3_a_p0_m(p0 : u32) -> mat2x3<f16> {
  let s = &(a[3u].a[p0]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_p0_a_2_m(p0 : u32) -> mat2x3<f16> {
  let s = &(a[p0].a[2u]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_p0_a_p1_m(p0 : u32, p1 : u32) -> mat2x3<f16> {
  let s = &(a[p0].a[p1]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn load_a_p0_a_p1_m_p2(p0 : u32, p1 : u32, p2 : u32) -> vec3<f16> {
  switch(p2) {
    case 0u: {
      return a[p0].a[p1].m_0;
    }
    case 1u: {
      return a[p0].a[p1].m_1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn load_a_p0_a_p1_m_p2_p3(p0 : u32, p1 : u32, p2 : u32, p3 : u32) -> f16 {
  switch(p2) {
    case 0u: {
      return a[p0].a[p1].m_0[p3];
    }
    case 1u: {
      return a[p0].a[p1].m_1[p3];
    }
    default: {
      return f16();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let K = 0;
  let p_a = conv_arr4_Outer(a);
  let p_a_3 = conv_Outer(a[3u]);
  let p_a_I = conv_Outer(a[I]);
  let p_a_3_a = conv_arr4_Inner(a[3u].a);
  let p_a_I_a = conv_arr4_Inner(a[I].a);
  let p_a_3_a_2 = conv_Inner(a[3u].a[2u]);
  let p_a_3_a_I = conv_Inner(a[3u].a[I]);
  let p_a_I_a_2 = conv_Inner(a[I].a[2u]);
  let p_a_I_a_J = conv_Inner(a[I].a[J]);
  let p_a_3_a_2_m = load_a_3_a_2_m();
  let p_a_3_a_I_m = load_a_3_a_p0_m(u32(I));
  let p_a_I_a_2_m = load_a_p0_a_2_m(u32(I));
  let p_a_I_a_J_m = load_a_p0_a_p1_m(u32(I), u32(J));
  let p_a_3_a_2_m_1 = a[3u].a[2u].m_1;
  let p_a_I_a_J_m_K = load_a_p0_a_p1_m_p2(u32(I), u32(J), u32(K));
  let l_a : array<Outer, 4> = conv_arr4_Outer(a);
  let l_a_3 : Outer = conv_Outer(a[3u]);
  let l_a_I : Outer = conv_Outer(a[I]);
  let l_a_3_a : array<Inner, 4> = conv_arr4_Inner(a[3u].a);
  let l_a_I_a : array<Inner, 4> = conv_arr4_Inner(a[I].a);
  let l_a_3_a_2 : Inner = conv_Inner(a[3u].a[2u]);
  let l_a_3_a_I : Inner = conv_Inner(a[3u].a[I]);
  let l_a_I_a_2 : Inner = conv_Inner(a[I].a[2u]);
  let l_a_I_a_J : Inner = conv_Inner(a[I].a[J]);
  let l_a_3_a_2_m : mat2x3<f16> = load_a_3_a_2_m();
  let l_a_3_a_I_m : mat2x3<f16> = load_a_3_a_p0_m(u32(I));
  let l_a_I_a_2_m : mat2x3<f16> = load_a_p0_a_2_m(u32(I));
  let l_a_I_a_J_m : mat2x3<f16> = load_a_p0_a_p1_m(u32(I), u32(J));
  let l_a_3_a_2_m_1 : vec3<f16> = a[3u].a[2u].m_1;
  let l_a_I_a_J_m_K : vec3<f16> = load_a_p0_a_p1_m_p2(u32(I), u32(J), u32(K));
  let l_a_2_a_0_m_1_0 : f16 = a[3u].a[2u].m_1[0u];
  let l_a_I_a_J_m_K_I : f16 = load_a_p0_a_p1_m_p2_p3(u32(I), u32(J), u32(K), u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_CopyArray_UniformToStorage_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
  s = u;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn conv_S(val : S_std140) -> S {
  return S(mat2x3<f16>(val.m_0, val.m_1));
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

TEST_F(Std140Test_F16, ArrayStructMatUniform_CopyStruct_UniformToWorkgroup_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

var<workgroup> w : array<S, 4>;

fn f() {
  w[0] = u[1];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  v : vec4<i32>,
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<workgroup> w : array<S, 4>;

fn conv_S(val : S_std140) -> S {
  return S(val.v, mat2x3<f16>(val.m_0, val.m_1));
}

fn f() {
  w[0] = conv_S(u[1u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_CopyMatrix_UniformToPrivate_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S, 3>;

var<private> p : array<S, 4>;

fn f() {
  p[2].m = u[1].m;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  v : vec4<i32>,
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 3u>;

var<private> p : array<S, 4>;

fn load_u_1_m() -> mat2x3<f16> {
  let s = &(u[1u]);
  return mat2x3<f16>((*(s)).m_0, (*(s)).m_1);
}

fn f() {
  p[2].m = load_u_1_m();
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_CopyColumn_UniformToStorage_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S, 3>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
  s[3].m[1] = u[2].m[0];
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 3u>;

@group(0) @binding(1) var<storage, read_write> s : array<S, 4>;

fn f() {
  s[3].m[1] = u[2u].m_0;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_CopyColumnSwizzle_UniformToWorkgroup_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S, 4>;

var<workgroup> w : array<S, 4>;

fn f() {
  w[3].m[1] = u[2].m[0].yzx.yzx;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 4u>;

var<workgroup> w : array<S, 4>;

fn f() {
  w[3].m[1] = u[2u].m_0.yzx.yzx;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayStructMatUniform_CopyScalar_UniformToPrivate_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : mat2x3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S, 3>;

var<private> p : array<S, 4>;

fn f() {
  p[3].m[1].x = u[2].m[0].y;
}
)";

    auto* expect = R"(
enable f16;

struct S {
  v : vec4<i32>,
  @size(64)
  m : mat2x3<f16>,
}

struct S_std140 {
  v : vec4<i32>,
  m_0 : vec3<f16>,
  @size(56)
  m_1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> u : array<S_std140, 3u>;

var<private> p : array<S, 4>;

fn f() {
  p[3].m[1].x = u[2u].m_0[1u];
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayMatUniform_LoadArray_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<mat2x3<f16>, 3>;

fn f() {
  let l = a;
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x3_f16, 3u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn conv_arr3_mat2x3_f16(val : array<mat2x3_f16, 3u>) -> array<mat2x3<f16>, 3u> {
  var arr : array<mat2x3<f16>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x3_f16(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat2x3_f16(a);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayMatUniform_LoadMatrix_ConstArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<mat2x3<f16>, 3>;

fn f() {
  let l = a[2];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x3_f16, 3u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let l = conv_mat2x3_f16(a[2u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayMatUniform_LoadMatrix_VariableArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<mat2x3<f16>, 3>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x3_f16, 3u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let I = 1;
  let l = conv_mat2x3_f16(a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayMatUniform_LoadColumn_ConstArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<mat2x3<f16>, 3>;

fn f() {
  let l = a[2][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x3_f16, 3u>;

fn f() {
  let l = a[2u].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayMatUniform_LoadColumn_VariableArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<mat2x3<f16>, 3>;

fn f() {
  let I = 1;
  let l = a[I][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x3_f16, 3u>;

fn f() {
  let I = 1;
  let l = a[I].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayMatUniform_LoadColumn_ConstArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<mat2x3<f16>, 3>;

fn f() {
  let I = 1;
  let l = a[2][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x3_f16, 3u>;

fn load_a_2_p0(p0 : u32) -> vec3<f16> {
  switch(p0) {
    case 0u: {
      return a[2u].col0;
    }
    case 1u: {
      return a[2u].col1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_2_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayMatUniform_LoadColumn_VariableArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<mat2x3<f16>, 3>;

fn f() {
  let I = 1;
  let l = a[I][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<mat2x3_f16, 3u>;

fn load_a_p0_p1(p0 : u32, p1 : u32) -> vec3<f16> {
  switch(p1) {
    case 0u: {
      return a[p0].col0;
    }
    case 1u: {
      return a[p0].col1;
    }
    default: {
      return vec3<f16>();
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

TEST_F(Std140Test_F16, StructArrayMatUniform_LoadStruct_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s;
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn conv_arr3_mat2x3_f16(val : array<mat2x3_f16, 3u>) -> array<mat2x3<f16>, 3u> {
  var arr : array<mat2x3<f16>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x3_f16(val[i]);
  }
  return arr;
}

fn conv_S(val : S_std140) -> S {
  return S(conv_arr3_mat2x3_f16(val.a));
}

fn f() {
  let l = conv_S(s);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructArrayMatUniform_LoadArray_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a;
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn conv_arr3_mat2x3_f16(val : array<mat2x3_f16, 3u>) -> array<mat2x3<f16>, 3u> {
  var arr : array<mat2x3<f16>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x3_f16(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat2x3_f16(s.a);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructArrayMatUniform_LoadMatrix_ConstArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[2];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let l = conv_mat2x3_f16(s.a[2u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, StructArrayMatUniform_LoadMatrix_VariableArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let I = 1;
  let l = conv_mat2x3_f16(s.a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       StructArrayMatUniform_LoadColumn_ConstArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let l = s.a[2][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn f() {
  let l = s.a[2u].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       StructArrayMatUniform_LoadColumn_VariableArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
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

TEST_F(Std140Test_F16,
       StructArrayMatUniform_LoadColumn_ConstArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[2][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_a_2_p0(p0 : u32) -> vec3<f16> {
  switch(p0) {
    case 0u: {
      return s.a[2u].col0;
    }
    case 1u: {
      return s.a[2u].col1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_s_a_2_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       StructArrayMatUniform_LoadColumn_VariableArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

struct S {
  a : array<mat2x3<f16>, 3>,
}

@group(0) @binding(0) var<uniform> s : S;

fn f() {
  let I = 1;
  let l = s.a[I][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

struct S {
  a : array<mat2x3<f16>, 3>,
}

struct S_std140 {
  a : array<mat2x3_f16, 3u>,
}

@group(0) @binding(0) var<uniform> s : S_std140;

fn load_s_a_p0_p1(p0 : u32, p1 : u32) -> vec3<f16> {
  switch(p1) {
    case 0u: {
      return s.a[p0].col0;
    }
    case 1u: {
      return s.a[p0].col1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_s_a_p0_p1(u32(I), u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayArrayMatUniform_LoadArrays_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let l = a;
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn conv_arr3_mat2x3_f16(val : array<mat2x3_f16, 3u>) -> array<mat2x3<f16>, 3u> {
  var arr : array<mat2x3<f16>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x3_f16(val[i]);
  }
  return arr;
}

fn conv_arr4_arr3_mat2x3_f16(val : array<array<mat2x3_f16, 3u>, 4u>) -> array<array<mat2x3<f16>, 3u>, 4u> {
  var arr : array<array<mat2x3<f16>, 3u>, 4u>;
  for(var i : u32; (i < 4u); i = (i + 1)) {
    arr[i] = conv_arr3_mat2x3_f16(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr4_arr3_mat2x3_f16(a);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayArrayMatUniform_LoadArray_ConstOuterArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let l = a[3];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn conv_arr3_mat2x3_f16(val : array<mat2x3_f16, 3u>) -> array<mat2x3<f16>, 3u> {
  var arr : array<mat2x3<f16>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x3_f16(val[i]);
  }
  return arr;
}

fn f() {
  let l = conv_arr3_mat2x3_f16(a[3u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16, ArrayArrayMatUniform_LoadArray_VariableOuterArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn conv_arr3_mat2x3_f16(val : array<mat2x3_f16, 3u>) -> array<mat2x3<f16>, 3u> {
  var arr : array<mat2x3<f16>, 3u>;
  for(var i : u32; (i < 3u); i = (i + 1)) {
    arr[i] = conv_mat2x3_f16(val[i]);
  }
  return arr;
}

fn f() {
  let I = 1;
  let l = conv_arr3_mat2x3_f16(a[I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayArrayMatUniform_LoadMatrix_ConstOuterArrayIndex_ConstInnerArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let l = a[3][2];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let l = conv_mat2x3_f16(a[3u][2u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayArrayMatUniform_LoadMatrix_ConstOuterArrayIndex_VariableInnerArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[3][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let I = 1;
  let l = conv_mat2x3_f16(a[3u][I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayArrayMatUniform_LoadMatrix_VariableOuterArrayIndex_ConstInnerArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][2];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let I = 1;
  let l = conv_mat2x3_f16(a[I][2u]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(Std140Test_F16,
       ArrayArrayMatUniform_LoadMatrix_VariableOuterArrayIndex_VariableInnerArrayIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn conv_mat2x3_f16(val : mat2x3_f16) -> mat2x3<f16> {
  return mat2x3<f16>(val.col0, val.col1);
}

fn f() {
  let I = 1;
  let l = conv_mat2x3_f16(a[I][I]);
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_ConstInnerArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let l = a[3][2][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn f() {
  let l = a[3u][2u].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_ConstInnerArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[3][2][I];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn load_a_3_2_p0(p0 : u32) -> vec3<f16> {
  switch(p0) {
    case 0u: {
      return a[3u][2u].col0;
    }
    case 1u: {
      return a[3u][2u].col1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let l = load_a_3_2_p0(u32(I));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_VariableInnerArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[3][I][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn f() {
  let I = 1;
  let l = a[3u][I].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_ConstOuterArrayIndex_VariableInnerArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[3][I][J];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn load_a_3_p0_p1(p0 : u32, p1 : u32) -> vec3<f16> {
  switch(p1) {
    case 0u: {
      return a[3u][p0].col0;
    }
    case 1u: {
      return a[3u][p0].col1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let l = load_a_3_p0_p1(u32(I), u32(J));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_ConstInnerArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let l = a[I][2][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn f() {
  let I = 1;
  let l = a[I][2u].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_ConstInnerArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[I][2][J];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn load_a_p0_2_p1(p0 : u32, p1 : u32) -> vec3<f16> {
  switch(p1) {
    case 0u: {
      return a[p0][2u].col0;
    }
    case 1u: {
      return a[p0][2u].col1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 1;
  let J = 2;
  let l = load_a_p0_2_p1(u32(I), u32(J));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_VariableInnerArrayIndex_ConstColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[I][J][1];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn f() {
  let I = 1;
  let J = 2;
  let l = a[I][J].col1;
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

TEST_F(
    Std140Test_F16,
    ArrayArrayMatUniform_LoadColumn_VariableOuterArrayIndex_VariableInnerArrayIndex_VariableColumnIndex_Mat2x3F16) {
    auto* src = R"(
enable f16;

@group(0) @binding(0) var<uniform> a : array<array<mat2x3<f16>, 3>, 4>;

fn f() {
  let I = 0;
  let J = 1;
  let K = 2;
  let l = a[I][J][K];
}
)";

    auto* expect = R"(
enable f16;

struct mat2x3_f16 {
  col0 : vec3<f16>,
  col1 : vec3<f16>,
}

@group(0) @binding(0) var<uniform> a : array<array<mat2x3_f16, 3u>, 4u>;

fn load_a_p0_p1_p2(p0 : u32, p1 : u32, p2 : u32) -> vec3<f16> {
  switch(p2) {
    case 0u: {
      return a[p0][p1].col0;
    }
    case 1u: {
      return a[p0][p1].col1;
    }
    default: {
      return vec3<f16>();
    }
  }
}

fn f() {
  let I = 0;
  let J = 1;
  let K = 2;
  let l = load_a_p0_p1_p2(u32(I), u32(J), u32(K));
}
)";

    auto got = Run<Std140>(src);

    EXPECT_EQ(expect, str(got));
}

}  // namespace
}  // namespace tint::transform
