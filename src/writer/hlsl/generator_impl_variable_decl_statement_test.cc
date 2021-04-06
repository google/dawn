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

#include "src/ast/variable_decl_statement.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_VariableDecl = TestHelper;

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement) {
  auto* var = Global("a", ty.f32(), ast::StorageClass::kInput);

  auto* stmt = create<ast::VariableDeclStatement>(var);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const) {
  auto* var = Const("a", ty.f32());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), "  const float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Array) {
  auto* var = Global("a", ty.array<f32, 5>(), ast::StorageClass::kInput);

  auto* stmt = create<ast::VariableDeclStatement>(var);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), "  float a[5];\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Function) {
  auto* var = Global("a", ty.f32(), ast::StorageClass::kFunction);

  auto* stmt = create<ast::VariableDeclStatement>(var);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Private) {
  auto* var = Global("a", ty.f32(), ast::StorageClass::kPrivate);

  auto* stmt = create<ast::VariableDeclStatement>(var);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), "  float a;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_Private) {
  Global("initializer", ty.f32(), ast::StorageClass::kInput);
  auto* var =
      Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr("initializer"));

  auto* stmt = create<ast::VariableDeclStatement>(var);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), R"(float a = initializer;
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_ZeroVec) {
  auto* var =
      Var("a", ty.vec3<f32>(), ast::StorageClass::kFunction, vec3<f32>());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), R"(float3 a = float3(0.0f);
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_ZeroMat) {
  auto* var =
      Var("a", ty.mat2x3<f32>(), ast::StorageClass::kFunction, mat2x3<f32>());

  auto* stmt = create<ast::VariableDeclStatement>(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(),
            R"(float3x2 a = float3x2(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
