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

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Assign = TestHelper;

TEST_F(HlslGeneratorImplTest_Assign, Emit_Assign) {
  Func("fn", {}, ty.void_(),
       {
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

TEST_F(HlslGeneratorImplTest_Assign, Emit_Vector_Assign_ConstantIndex) {
  Func("fn", {}, ty.void_(),
       {
           Decl(Var("lhs", ty.vec3<f32>())),
           Decl(Var("rhs", ty.f32())),
           Decl(Let("index", ty.u32(), Expr(0u))),
           Assign(IndexAccessor("lhs", "index"), "rhs"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate());
  EXPECT_EQ(gen.result(),
            R"(void fn() {
  float3 lhs = float3(0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  const uint index = 0u;
  lhs[index] = rhs;
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Vector_Assign_DynamicIndex) {
  Func("fn", {}, ty.void_(),
       {
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

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Vector_ConstantIndex) {
  Func("fn", {}, ty.void_(),
       {
           Decl(Var("lhs", ty.mat4x2<f32>())),
           Decl(Var("rhs", ty.vec2<f32>())),
           Decl(Let("index", ty.u32(), Expr(0u))),
           Assign(IndexAccessor("lhs", "index"), "rhs"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate());
  EXPECT_EQ(gen.result(),
            R"(void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float2 rhs = float2(0.0f, 0.0f);
  const uint index = 0u;
  lhs[index] = rhs;
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Vector_DynamicIndex) {
  Func("fn", {}, ty.void_(),
       {
           Decl(Var("lhs", ty.mat4x2<f32>())),
           Decl(Var("rhs", ty.vec2<f32>())),
           Decl(Var("index", ty.u32())),
           Assign(IndexAccessor("lhs", "index"), "rhs"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate());
  EXPECT_EQ(
      gen.result(),
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

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Scalar_ConstantIndex) {
  Func("fn", {}, ty.void_(),
       {
           Decl(Var("lhs", ty.mat4x2<f32>())),
           Decl(Var("rhs", ty.f32())),
           Decl(Let("index", ty.u32(), Expr(0u))),
           Assign(IndexAccessor(IndexAccessor("lhs", "index"), "index"), "rhs"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate());
  EXPECT_EQ(gen.result(),
            R"(void fn() {
  float4x2 lhs = float4x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
  float rhs = 0.0f;
  const uint index = 0u;
  lhs[index][index] = rhs;
}
)");
}

TEST_F(HlslGeneratorImplTest_Assign, Emit_Matrix_Assign_Scalar_DynamicIndex) {
  Func("fn", {}, ty.void_(),
       {
           Decl(Var("lhs", ty.mat4x2<f32>())),
           Decl(Var("rhs", ty.f32())),
           Decl(Var("index", ty.u32())),
           Assign(IndexAccessor(IndexAccessor("lhs", "index"), "index"), "rhs"),
       });

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate());
  EXPECT_EQ(
      gen.result(),
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
  uint index = 0u;
  set_scalar_float4x2(lhs, index, index, rhs);
}
)");
}

}  // namespace
}  // namespace tint::writer::hlsl
