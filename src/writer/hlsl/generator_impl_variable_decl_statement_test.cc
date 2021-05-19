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

#include "gmock/gmock.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using ::testing::HasSubstr;

using HlslGeneratorImplTest_VariableDecl = TestHelper;

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement) {
  auto* var = Var("a", ty.f32());
  auto* stmt = Decl(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), "  float a = 0.0f;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Const) {
  auto* var = Const("a", ty.f32(), Construct(ty.f32()));
  auto* stmt = Decl(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), "  const float a = 0.0f;\n");
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Array) {
  auto* var = Var("a", ty.array<f32, 5>());

  WrapInFunction(var, Expr("a"));

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(),
              HasSubstr("  float a[5] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};\n"));
}

TEST_F(HlslGeneratorImplTest_VariableDecl, Emit_VariableDeclStatement_Private) {
  Global("a", ty.f32(), ast::StorageClass::kPrivate);

  WrapInFunction(Expr("a"));

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("  static float a;\n"));
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_Private) {
  Global("initializer", ty.f32(), ast::StorageClass::kPrivate);
  Global("a", ty.f32(), ast::StorageClass::kPrivate, Expr("initializer"));

  WrapInFunction(Expr("a"));

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr(R"(float a = initializer;
)"));
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_ZeroVec) {
  auto* var = Var("a", ty.vec3<f32>(), ast::StorageClass::kNone, vec3<f32>());

  auto* stmt = Decl(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(), R"(float3 a = float3(0.0f, 0.0f, 0.0f);
)");
}

TEST_F(HlslGeneratorImplTest_VariableDecl,
       Emit_VariableDeclStatement_Initializer_ZeroMat) {
  auto* var =
      Var("a", ty.mat2x3<f32>(), ast::StorageClass::kNone, mat2x3<f32>());

  auto* stmt = Decl(var);
  WrapInFunction(stmt);

  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.EmitStatement(out, stmt)) << gen.error();
  EXPECT_EQ(result(),
            R"(float2x3 a = float2x3(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
