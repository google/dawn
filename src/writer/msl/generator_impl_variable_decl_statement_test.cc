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
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement) {
  ast::type::F32 f32;
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Const) {
  ast::type::F32 f32;
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            true,                            // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  const float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Array) {
  ast::type::F32 f32;
  ast::type::Array ary(&f32, 5, ast::ArrayDecorationList{});

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &ary,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float a[5] = {0.0f};\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Struct) {
  ast::type::F32 f32;

  ast::StructMemberList members;
  members.push_back(
      create<ast::StructMember>("a", &f32, ast::StructMemberDecorationList{}));

  ast::StructMemberDecorationList b_deco;
  b_deco.push_back(create<ast::StructMemberOffsetDecoration>(4, Source{}));
  members.push_back(create<ast::StructMember>("b", &f32, b_deco));

  auto* str = create<ast::Struct>(members);

  ast::type::Struct s("S", str);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &s,                              // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  S a = {};
)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Vector) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 2);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kFunction,    // storage_class
                            &vec,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float2 a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Matrix) {
  ast::type::F32 f32;
  ast::type::Matrix mat(&f32, 2, 3);
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float3x2 a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Private) {
  ast::type::F32 f32;
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kPrivate,     // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), "  float a = 0.0f;\n");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_Private) {
  auto* ident = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("initializer"), "initializer");

  ast::type::F32 f32;
  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            ident,                           // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), R"(float a = initializer;
)");
}

TEST_F(MslGeneratorImplTest, Emit_VariableDeclStatement_Initializer_ZeroVec) {
  ast::type::F32 f32;
  ast::type::Vector vec(&f32, 3);

  ast::ExpressionList values;
  auto* zero_vec =
      create<ast::TypeConstructorExpression>(Source{}, &vec, values);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "a",                             // name
                            ast::StorageClass::kNone,        // storage_class
                            &vec,                            // type
                            false,                           // is_const
                            zero_vec,                        // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement stmt(var);

  ASSERT_TRUE(gen.EmitStatement(&stmt)) << gen.error();
  EXPECT_EQ(gen.result(), R"(float3 a = float3(0.0f);
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
