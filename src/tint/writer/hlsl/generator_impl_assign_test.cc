// Copyright 2020 The Tint Authors.
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

#include "src/tint/writer/hlsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Assign = TestHelper;

TEST_F(HlslGeneratorImplTest_Assign, Emit_Assign) {
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.i32())),
             Decl(Var("rhs", ty.i32())),
             Assign("lhs", "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void fn() {
  int lhs = 0;
  int rhs = 0;
  lhs = rhs;
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Vector_Assign_LetIndex) {
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.vec3<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Let("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void fn() {
  float3 lhs = float3(0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  const uint index = 0u;
  set_float3(lhs, index, rhs);
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Vector_Assign_ConstIndex) {
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.vec3<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Const("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void fn() {
  float3 lhs = float3(0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  lhs[0u] = rhs;
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Vector_Assign_DynamicIndex) {
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.vec3<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Var("index", ty.u32())),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void set_float3(inout float3 vec, int idx, float val) {
  vec = (idx.xxx == int3(0, 1, 2)) ? val.xxx : vec;
}

void fn() {
  float3 lhs = float3(0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  uint index = 0u;
  set_float3(lhs, index, rhs);
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Vector_LetIndex) {
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.vec2<f32>())),
             Decl(Let("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void set_vector_float4x2(inout float4x2 mat, int col, float2 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
    case 2: mat[2] = val; break;
    case 3: mat[3] = val; break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2 rhs = float2(0.0f, 0.0f);
  const uint index = 0u;
  set_vector_float4x2(lhs, index, rhs);
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Vector_ConstIndex) {
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.vec2<f32>())),
             Decl(Const("index", ty.u32(), Expr(0_u))),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2 rhs = float2(0.0f, 0.0f);
  lhs[0u] = rhs;
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Vector_DynamicIndex) {
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.vec2<f32>())),
             Decl(Var("index", ty.u32())),
             Assign(IndexAccessor("lhs", "index"), "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void set_vector_float4x2(inout float4x2 mat, int col, float2 val) {
  switch (col) {
    case 0: mat[0] = val; break;
    case 1: mat[1] = val; break;
    case 2: mat[2] = val; break;
    case 3: mat[3] = val; break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2 rhs = float2(0.0f, 0.0f);
  uint index = 0u;
  set_vector_float4x2(lhs, index, rhs);
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Scalar_LetIndices) {
    auto* col = IndexAccessor("lhs", "col");
    auto* el = IndexAccessor(col, "row");
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Let("col", ty.u32(), Expr(0_u))),
             Decl(Let("row", ty.u32(), Expr(1_u))),
             Assign(el, "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void set_scalar_float4x2(inout float4x2 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xx == int2(0, 1)) ? val.xx : mat[0];
      break;
    case 1:
      mat[1] = (row.xx == int2(0, 1)) ? val.xx : mat[1];
      break;
    case 2:
      mat[2] = (row.xx == int2(0, 1)) ? val.xx : mat[2];
      break;
    case 3:
      mat[3] = (row.xx == int2(0, 1)) ? val.xx : mat[3];
      break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  const uint col = 0u;
  const uint row = 1u;
  set_scalar_float4x2(lhs, col, row, rhs);
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Scalar_ConstIndices) {
    auto* col = IndexAccessor("lhs", "col");
    auto* el = IndexAccessor(col, "row");
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Const("col", ty.u32(), Expr(0_u))),
             Decl(Const("row", ty.u32(), Expr(1_u))),
             Assign(el, "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  lhs[0u][1u] = rhs;
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Scalar_DynamicIndices) {
    auto* col = IndexAccessor("lhs", "col");
    auto* el = IndexAccessor(col, "row");
    Func("fn", utils::Empty, ty.void_(),
         utils::Vector{
             Decl(Var("lhs", ty.mat4x2<f32>())),
             Decl(Var("rhs", ty.f32())),
             Decl(Var("col", ty.u32())),
             Decl(Var("row", ty.u32())),
             Assign(el, "rhs"),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(),
              R"(void set_scalar_float4x2(inout float4x2 mat, int col, int row, float val) {
  switch (col) {
    case 0:
      mat[0] = (row.xx == int2(0, 1)) ? val.xx : mat[0];
      break;
    case 1:
      mat[1] = (row.xx == int2(0, 1)) ? val.xx : mat[1];
      break;
    case 2:
      mat[2] = (row.xx == int2(0, 1)) ? val.xx : mat[2];
      break;
    case 3:
      mat[3] = (row.xx == int2(0, 1)) ? val.xx : mat[3];
      break;
  }
}

void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  uint col = 0u;
  uint row = 0u;
  set_scalar_float4x2(lhs, col, row, rhs);
}
)");
}

}  // namespace
}  // namespace tint::writer::hlsl
