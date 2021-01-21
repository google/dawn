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

#include <memory>
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/struct.h"
#include "src/ast/struct_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/matrix_type.h"
#include "src/type/struct_type.h"
#include "src/type/vector_type.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement) {
  auto* var = Var("a", ast::StorageClass::kNone, ty.f32);
  auto* stmt = create<ast::VariableDeclStatement>(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const) {
  auto* var = Const("a", ast::StorageClass::kNone, ty.f32);
  auto* stmt = create<ast::VariableDeclStatement>(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  const float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Array) {
  type::Array ary(ty.f32, 5, ast::ArrayDecorationList{});

  auto* var = Var("a", ast::StorageClass::kNone, &ary);
  auto* stmt = create<ast::VariableDeclStatement>(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float a[5] = {0.0f};\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Struct) {
  auto* str = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.f32),
                            Member("b", ty.f32, {MemberOffset(4)})},
      ast::StructDecorationList{});

  auto* s = ty.struct_("S", str);
  auto* var = Var("a", ast::StorageClass::kNone, s);
  auto* stmt = create<ast::VariableDeclStatement>(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  S a = {};
)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Vector) {
  auto* var = Var("a", ast::StorageClass::kFunction, ty.vec2<f32>());
  auto* stmt = create<ast::VariableDeclStatement>(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float2 a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Matrix) {
  auto* var = Var("a", ast::StorageClass::kFunction, ty.mat3x2<f32>());

  auto* stmt = create<ast::VariableDeclStatement>(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float3x2 a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Private) {
  auto* var = Var("a", ast::StorageClass::kPrivate, ty.f32);
  auto* stmt = create<ast::VariableDeclStatement>(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_Private) {
  auto* var = Var("a", ast::StorageClass::kNone, ty.f32, Expr("initializer"),
                  ast::VariableDecorationList{});
  auto* stmt = create<ast::VariableDeclStatement>(var);

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), R"(float a = initializer;
)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_ZeroVec) {
  auto* zero_vec = vec3<f32>();

  auto* var = Var("a", ast::StorageClass::kNone, ty.vec3<f32>(), zero_vec,
                  ast::VariableDecorationList{});
  auto* stmt = create<ast::VariableDeclStatement>(var);

  ASSERT_TRUE(gen.EmitStatement(stmt)) << gen.error();
  EXPECT_EQ(gen.result(), R"(float3 a = float3(0.0f);
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
