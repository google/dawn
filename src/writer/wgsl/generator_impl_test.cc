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

#include "src/writer/wgsl/generator_impl.h"

#include <memory>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/location_decoration.h"
#include "src/ast/set_decoration.h"
#include "src/ast/struct.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/pointer_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/variable.h"

namespace tint {
namespace writer {
namespace wgsl {
namespace {

using GeneratorImplTest = testing::Test;

TEST_F(GeneratorImplTest, DISABLED_Generate) {}

TEST_F(GeneratorImplTest, EmitAliasType_F32) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("a", &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitAliasType(&alias));
  EXPECT_EQ(g.result(), R"(type a = f32;
)");
}

TEST_F(GeneratorImplTest, DISABLED_EmitAliasType_Struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::StructMember>> members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &f32, std::vector<std::unique_ptr<ast::StructMemberDecoration>>{}));

  std::vector<std::unique_ptr<ast::StructMemberDecoration>> b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &i32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));
  ast::type::AliasType alias("a", &s);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitAliasType(&alias));
  EXPECT_EQ(g.result(), R"(type a = struct {
  a: f32;
  [[offset 4]] b : i32;
}
)");
}

TEST_F(GeneratorImplTest, EmitEntryPoint_NoName) {
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "", "frag_main");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitEntryPoint(&ep));
  EXPECT_EQ(g.result(), R"(entry_point fragment = frag_main;
)");
}

TEST_F(GeneratorImplTest, EmitEntryPoint_WithName) {
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "main", "frag_main");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitEntryPoint(&ep));
  EXPECT_EQ(g.result(), R"(entry_point fragment as "main" = frag_main;
)");
}

TEST_F(GeneratorImplTest, EmitImport) {
  ast::Import import("GLSL.std.450", "std::glsl");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitImport(&import));
  EXPECT_EQ(g.result(), R"(import "GLSL.std.450" as std::glsl;
)");
}

TEST_F(GeneratorImplTest, EmitType_Alias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("alias", &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&alias));
  EXPECT_EQ(g.result(), "alias");
}

TEST_F(GeneratorImplTest, EmitType_Array) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a));
  EXPECT_EQ(g.result(), "array<bool, 4>");
}

TEST_F(GeneratorImplTest, EmitType_RuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a));
  EXPECT_EQ(g.result(), "array<bool>");
}

TEST_F(GeneratorImplTest, EmitType_Bool) {
  ast::type::BoolType b;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&b));
  EXPECT_EQ(g.result(), "bool");
}

TEST_F(GeneratorImplTest, EmitType_F32) {
  ast::type::F32Type f32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&f32));
  EXPECT_EQ(g.result(), "f32");
}

TEST_F(GeneratorImplTest, EmitType_I32) {
  ast::type::I32Type i32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&i32));
  EXPECT_EQ(g.result(), "i32");
}

TEST_F(GeneratorImplTest, EmitType_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType m(&f32, 3, 2);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&m));
  EXPECT_EQ(g.result(), "mat2x3<f32>");
}

TEST_F(GeneratorImplTest, EmitType_Pointer) {
  ast::type::F32Type f32;
  ast::type::PointerType p(&f32, ast::StorageClass::kWorkgroup);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&p));
  EXPECT_EQ(g.result(), "ptr<workgroup, f32>");
}

TEST_F(GeneratorImplTest, EmitType_Struct) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::StructMember>> members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &i32, std::vector<std::unique_ptr<ast::StructMemberDecoration>>{}));

  std::vector<std::unique_ptr<ast::StructMemberDecoration>> b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));

  ast::type::StructType s(std::move(str));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&s));
  EXPECT_EQ(g.result(), R"(struct {
  a : i32;
  [[offset 4]] b : f32;
})");
}

TEST_F(GeneratorImplTest, EmitType_Struct_WithDecoration) {
  ast::type::I32Type i32;
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::StructMember>> members;
  members.push_back(std::make_unique<ast::StructMember>(
      "a", &i32, std::vector<std::unique_ptr<ast::StructMemberDecoration>>{}));

  std::vector<std::unique_ptr<ast::StructMemberDecoration>> b_deco;
  b_deco.push_back(std::make_unique<ast::StructMemberOffsetDecoration>(4));
  members.push_back(
      std::make_unique<ast::StructMember>("b", &f32, std::move(b_deco)));

  auto str = std::make_unique<ast::Struct>();
  str->set_members(std::move(members));
  str->set_decoration(ast::StructDecoration::kBlock);

  ast::type::StructType s(std::move(str));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&s));
  EXPECT_EQ(g.result(), R"([[block]] struct {
  a : i32;
  [[offset 4]] b : f32;
})");
}

TEST_F(GeneratorImplTest, EmitType_U32) {
  ast::type::U32Type u32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&u32));
  EXPECT_EQ(g.result(), "u32");
}

TEST_F(GeneratorImplTest, EmitType_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType v(&f32, 3);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v));
  EXPECT_EQ(g.result(), "vec3<f32>");
}

TEST_F(GeneratorImplTest, EmitType_Void) {
  ast::type::VoidType v;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v));
  EXPECT_EQ(g.result(), "void");
}

TEST_F(GeneratorImplTest, EmitVariable) {
  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kNone, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v));
  EXPECT_EQ(g.result(), R"(var a : f32;
)");
}

TEST_F(GeneratorImplTest, EmitVariable_StorageClass) {
  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kInput, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v));
  EXPECT_EQ(g.result(), R"(var<in> a : f32;
)");
}

TEST_F(GeneratorImplTest, EmitVariable_Decorated) {
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::VariableDecoration>> decos;
  decos.push_back(std::make_unique<ast::LocationDecoration>(2));

  ast::DecoratedVariable dv;
  dv.set_name("a");
  dv.set_type(&f32);
  dv.set_decorations(std::move(decos));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&dv));
  EXPECT_EQ(g.result(), R"([[location 2]] var a : f32;
)");
}

TEST_F(GeneratorImplTest, EmitVariable_Decorated_Multiple) {
  ast::type::F32Type f32;

  std::vector<std::unique_ptr<ast::VariableDecoration>> decos;
  decos.push_back(
      std::make_unique<ast::BuiltinDecoration>(ast::Builtin::kPosition));
  decos.push_back(std::make_unique<ast::BindingDecoration>(0));
  decos.push_back(std::make_unique<ast::SetDecoration>(1));
  decos.push_back(std::make_unique<ast::LocationDecoration>(2));

  ast::DecoratedVariable dv;
  dv.set_name("a");
  dv.set_type(&f32);
  dv.set_decorations(std::move(decos));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&dv));
  EXPECT_EQ(g.result(),
            R"([[builtin position, binding 0, set 1, location 2]] var a : f32;
)");
}

TEST_F(GeneratorImplTest, DISABLED_EmitVariable_Initializer) {}

TEST_F(GeneratorImplTest, DISABLED_EmitVariable_Const) {}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
