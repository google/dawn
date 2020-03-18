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
#include "src/ast/array_accessor_expression.h"
#include "src/ast/binding_decoration.h"
#include "src/ast/bool_literal.h"
#include "src/ast/builtin.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/decorated_variable.h"
#include "src/ast/float_literal.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/int_literal.h"
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
#include "src/ast/uint_literal.h"
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
  ASSERT_TRUE(g.EmitAliasType(&alias)) << g.error();
  EXPECT_EQ(g.result(), R"(type a = f32;
)");
}

TEST_F(GeneratorImplTest, EmitAliasType_Struct) {
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
  ASSERT_TRUE(g.EmitAliasType(&alias)) << g.error();
  EXPECT_EQ(g.result(), R"(type a = struct {
  a : f32;
  [[offset 4]] b : i32;
};
)");
}

TEST_F(GeneratorImplTest, EmitEntryPoint_NoName) {
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "", "frag_main");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitEntryPoint(&ep)) << g.error();
  EXPECT_EQ(g.result(), R"(entry_point fragment = frag_main;
)");
}

TEST_F(GeneratorImplTest, EmitEntryPoint_WithName) {
  ast::EntryPoint ep(ast::PipelineStage::kFragment, "main", "frag_main");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitEntryPoint(&ep)) << g.error();
  EXPECT_EQ(g.result(), R"(entry_point fragment as "main" = frag_main;
)");
}

TEST_F(GeneratorImplTest, EmitImport) {
  ast::Import import("GLSL.std.450", "std::glsl");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitImport(&import)) << g.error();
  EXPECT_EQ(g.result(), R"(import "GLSL.std.450" as std::glsl;
)");
}

TEST_F(GeneratorImplTest, EmitType_Alias) {
  ast::type::F32Type f32;
  ast::type::AliasType alias("alias", &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&alias)) << g.error();
  EXPECT_EQ(g.result(), "alias");
}

TEST_F(GeneratorImplTest, EmitType_Array) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b, 4);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "array<bool, 4>");
}

TEST_F(GeneratorImplTest, EmitType_RuntimeArray) {
  ast::type::BoolType b;
  ast::type::ArrayType a(&b);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&a)) << g.error();
  EXPECT_EQ(g.result(), "array<bool>");
}

TEST_F(GeneratorImplTest, EmitType_Bool) {
  ast::type::BoolType b;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&b)) << g.error();
  EXPECT_EQ(g.result(), "bool");
}

TEST_F(GeneratorImplTest, EmitType_F32) {
  ast::type::F32Type f32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&f32)) << g.error();
  EXPECT_EQ(g.result(), "f32");
}

TEST_F(GeneratorImplTest, EmitType_I32) {
  ast::type::I32Type i32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&i32)) << g.error();
  EXPECT_EQ(g.result(), "i32");
}

TEST_F(GeneratorImplTest, EmitType_Matrix) {
  ast::type::F32Type f32;
  ast::type::MatrixType m(&f32, 3, 2);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&m)) << g.error();
  EXPECT_EQ(g.result(), "mat2x3<f32>");
}

TEST_F(GeneratorImplTest, EmitType_Pointer) {
  ast::type::F32Type f32;
  ast::type::PointerType p(&f32, ast::StorageClass::kWorkgroup);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&p)) << g.error();
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
  ASSERT_TRUE(g.EmitType(&s)) << g.error();
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
  ASSERT_TRUE(g.EmitType(&s)) << g.error();
  EXPECT_EQ(g.result(), R"([[block]] struct {
  a : i32;
  [[offset 4]] b : f32;
})");
}

TEST_F(GeneratorImplTest, EmitType_U32) {
  ast::type::U32Type u32;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&u32)) << g.error();
  EXPECT_EQ(g.result(), "u32");
}

TEST_F(GeneratorImplTest, EmitType_Vector) {
  ast::type::F32Type f32;
  ast::type::VectorType v(&f32, 3);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v)) << g.error();
  EXPECT_EQ(g.result(), "vec3<f32>");
}

TEST_F(GeneratorImplTest, EmitType_Void) {
  ast::type::VoidType v;

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitType(&v)) << g.error();
  EXPECT_EQ(g.result(), "void");
}

TEST_F(GeneratorImplTest, EmitVariable) {
  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kNone, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
  EXPECT_EQ(g.result(), R"(var a : f32;
)");
}

TEST_F(GeneratorImplTest, EmitVariable_StorageClass) {
  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kInput, &f32);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
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
  ASSERT_TRUE(g.EmitVariable(&dv)) << g.error();
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
  ASSERT_TRUE(g.EmitVariable(&dv)) << g.error();
  EXPECT_EQ(g.result(),
            R"([[builtin position, binding 0, set 1, location 2]] var a : f32;
)");
}

TEST_F(GeneratorImplTest, EmitVariable_Initializer) {
  auto ident = std::make_unique<ast::IdentifierExpression>("initializer");

  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kNone, &f32);
  v.set_initializer(std::move(ident));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
  EXPECT_EQ(g.result(), R"(var a : f32 = initializer;
)");
}

TEST_F(GeneratorImplTest, EmitVariable_Const) {
  auto ident = std::make_unique<ast::IdentifierExpression>("initializer");

  ast::type::F32Type f32;
  ast::Variable v("a", ast::StorageClass::kNone, &f32);
  v.set_initializer(std::move(ident));
  v.set_is_const(true);

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitVariable(&v)) << g.error();
  EXPECT_EQ(g.result(), R"(const a : f32 = initializer;
)");
}

TEST_F(GeneratorImplTest, EmitExpression_ArrayAccessor) {
  auto lit = std::make_unique<ast::IntLiteral>(5);
  auto idx = std::make_unique<ast::ConstInitializerExpression>(std::move(lit));
  auto ary = std::make_unique<ast::IdentifierExpression>("ary");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&expr)) << g.error();
  EXPECT_EQ(g.result(), "ary[5]");
}

TEST_F(GeneratorImplTest, EmitExpression_Identifier) {
  ast::IdentifierExpression i(std::vector<std::string>{"std", "glsl"});

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "std::glsl");
}

TEST_F(GeneratorImplTest, EmitArrayAccessor) {
  auto ary = std::make_unique<ast::IdentifierExpression>("ary");
  auto idx = std::make_unique<ast::IdentifierExpression>("idx");

  ast::ArrayAccessorExpression expr(std::move(ary), std::move(idx));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitArrayAccessor(&expr)) << g.error();
  EXPECT_EQ(g.result(), "ary[idx]");
}

TEST_F(GeneratorImplTest, EmitIdentifierExpression_Single) {
  ast::IdentifierExpression i("glsl");

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "glsl");
}

TEST_F(GeneratorImplTest, EmitIdentifierExpression_MultipleNames) {
  ast::IdentifierExpression i({"std", "glsl", "init"});

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitExpression(&i)) << g.error();
  EXPECT_EQ(g.result(), "std::glsl::init");
}

TEST_F(GeneratorImplTest, EmitInitializer_Bool) {
  auto lit = std::make_unique<ast::BoolLiteral>(false);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "false");
}

TEST_F(GeneratorImplTest, EmitInitializer_Int) {
  auto lit = std::make_unique<ast::IntLiteral>(-12345);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "-12345");
}

TEST_F(GeneratorImplTest, EmitInitializer_UInt) {
  auto lit = std::make_unique<ast::UintLiteral>(56779);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "56779u");
}

TEST_F(GeneratorImplTest, EmitInitializer_Float) {
  auto lit = std::make_unique<ast::FloatLiteral>(1.5e27);
  ast::ConstInitializerExpression expr(std::move(lit));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "1.49999995e+27");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Float) {
  ast::type::F32Type f32;

  auto lit = std::make_unique<ast::FloatLiteral>(-1.2e-5);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&f32, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "f32(-1.20000004e-05)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Bool) {
  ast::type::BoolType b;

  auto lit = std::make_unique<ast::BoolLiteral>(true);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&b, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "bool(true)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Int) {
  ast::type::I32Type i32;

  auto lit = std::make_unique<ast::IntLiteral>(-12345);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&i32, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "i32(-12345)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Uint) {
  ast::type::U32Type u32;

  auto lit = std::make_unique<ast::UintLiteral>(12345);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit)));

  ast::TypeInitializerExpression expr(&u32, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "u32(12345u)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Vec) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);

  auto lit1 = std::make_unique<ast::FloatLiteral>(1.f);
  auto lit2 = std::make_unique<ast::FloatLiteral>(2.f);
  auto lit3 = std::make_unique<ast::FloatLiteral>(3.f);
  std::vector<std::unique_ptr<ast::Expression>> values;
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit1)));
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit2)));
  values.push_back(
      std::make_unique<ast::ConstInitializerExpression>(std::move(lit3)));

  ast::TypeInitializerExpression expr(&vec, std::move(values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), "vec3<f32>(1.00000000, 2.00000000, 3.00000000)");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Mat) {
  ast::type::F32Type f32;
  ast::type::MatrixType mat(&f32, 3, 2);

  ast::type::VectorType vec(&f32, 2);

  std::vector<std::unique_ptr<ast::Expression>> mat_values;

  for (size_t i = 0; i < 3; i++) {
    auto lit1 = std::make_unique<ast::FloatLiteral>(1.f + (i * 2));
    auto lit2 = std::make_unique<ast::FloatLiteral>(2.f + (i * 2));

    std::vector<std::unique_ptr<ast::Expression>> values;
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit1)));
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit2)));

    mat_values.push_back(std::make_unique<ast::TypeInitializerExpression>(
        &vec, std::move(values)));
  }

  ast::TypeInitializerExpression expr(&mat, std::move(mat_values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(),
            std::string("mat2x3<f32>(vec2<f32>(1.00000000, 2.00000000), ") +
                "vec2<f32>(3.00000000, 4.00000000), " +
                "vec2<f32>(5.00000000, 6.00000000))");
}

TEST_F(GeneratorImplTest, EmitInitializer_Type_Array) {
  ast::type::F32Type f32;
  ast::type::VectorType vec(&f32, 3);
  ast::type::ArrayType ary(&vec, 3);

  std::vector<std::unique_ptr<ast::Expression>> ary_values;

  for (size_t i = 0; i < 3; i++) {
    auto lit1 = std::make_unique<ast::FloatLiteral>(1.f + (i * 3));
    auto lit2 = std::make_unique<ast::FloatLiteral>(2.f + (i * 3));
    auto lit3 = std::make_unique<ast::FloatLiteral>(3.f + (i * 3));

    std::vector<std::unique_ptr<ast::Expression>> values;
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit1)));
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit2)));
    values.push_back(
        std::make_unique<ast::ConstInitializerExpression>(std::move(lit3)));

    ary_values.push_back(std::make_unique<ast::TypeInitializerExpression>(
        &vec, std::move(values)));
  }

  ast::TypeInitializerExpression expr(&ary, std::move(ary_values));

  GeneratorImpl g;
  ASSERT_TRUE(g.EmitInitializer(&expr)) << g.error();
  EXPECT_EQ(g.result(), std::string("array<vec3<f32>, 3>(") +
                            "vec3<f32>(1.00000000, 2.00000000, 3.00000000), " +
                            "vec3<f32>(4.00000000, 5.00000000, 6.00000000), " +
                            "vec3<f32>(7.00000000, 8.00000000, 9.00000000))");
}

}  // namespace
}  // namespace wgsl
}  // namespace writer
}  // namespace tint
