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

#include "gtest/gtest.h"
#include "src/ast/array_accessor_expression.h"
#include "src/ast/struct.h"
#include "src/ast/struct_block_decoration.h"
#include "src/ast/struct_member.h"
#include "src/ast/struct_member_decoration.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable_decl_statement.h"
#include "src/type/alias_type.h"
#include "src/type/array_type.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/struct_type.h"
#include "src/validator/validator_impl.h"
#include "src/validator/validator_test_helper.h"

#include "src/ast/pipeline_stage.h"
#include "src/ast/stage_decoration.h"
namespace tint {
namespace {

class ValidatorTypeTest : public ValidatorTestHelper, public testing::Test {};

TEST_F(ValidatorTypeTest, RuntimeArrayIsLast_Pass) {
  // [[Block]]
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st =
      create<ast::Struct>(ast::StructMemberList{Member("vf", ty.f32()),
                                                Member("rt", ty.array<f32>())},
                          decos);

  auto* struct_type = ty.struct_("Foo", st);

  AST().AddConstructedType(struct_type);

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_TRUE(v.ValidateConstructedTypes(program->AST().ConstructedTypes()));
}

TEST_F(ValidatorTypeTest, RuntimeArrayIsLastNoBlock_Fail) {
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  ast::StructDecorationList decos;
  auto* st =
      create<ast::Struct>(ast::StructMemberList{Member("vf", ty.f32()),
                                                Member("rt", ty.array<f32>())},
                          decos);

  auto* struct_type = ty.struct_("Foo", st);
  AST().AddConstructedType(struct_type);

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_FALSE(v.ValidateConstructedTypes(program->AST().ConstructedTypes()));
  EXPECT_EQ(v.error(),
            "v-0031: a struct containing a runtime-sized array must be "
            "in the 'storage' storage class: 'Foo'");
}

TEST_F(ValidatorTypeTest, RuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // struct Foo {
  //   rt: array<f32>;
  //   vf: f32;
  // };

  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());

  SetSource(Source::Location{12, 34});
  auto* rt = Member("rt", ty.array<f32>());
  SetSource(Source{});
  auto* st = create<ast::Struct>(
      ast::StructMemberList{rt, Member("vf", ty.f32())}, decos);

  auto* struct_type = ty.struct_("Foo", st);

  AST().AddConstructedType(struct_type);

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_FALSE(v.ValidateConstructedTypes(program->AST().ConstructedTypes()));
  EXPECT_EQ(v.error(),
            "12:34 v-0015: runtime arrays may only appear as the last member "
            "of a struct");
}

TEST_F(ValidatorTypeTest, AliasRuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  b: RTArr;
  //  a: u32;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());

  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("b", alias), Member("a", ty.u32())}, decos);

  auto* struct_type = ty.struct_("s", st);
  AST().AddConstructedType(struct_type);

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_FALSE(v.ValidateConstructedTypes(program->AST().ConstructedTypes()));
  EXPECT_EQ(v.error(),
            "v-0015: runtime arrays may only appear as the last member "
            "of a struct");
}

TEST_F(ValidatorTypeTest, AliasRuntimeArrayIsLast_Pass) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  a: u32;
  //  b: RTArr;
  //}

  auto* alias = ty.alias("RTArr", ty.array<u32>());

  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.u32()), Member("b", alias)}, decos);

  auto* struct_type = ty.struct_("s", st);
  AST().AddConstructedType(struct_type);

  ValidatorImpl& v = Build();
  const Program* program = v.program();

  EXPECT_TRUE(v.ValidateConstructedTypes(program->AST().ConstructedTypes()));
}

TEST_F(ValidatorTypeTest, RuntimeArrayInFunction_Fail) {
  /// [[stage(vertex)]]
  // fn func -> void { var a : array<i32>; }

  auto* var = Var("a", ast::StorageClass::kNone, ty.array<i32>());
  auto* func =
      Func("func", ast::VariableList{}, ty.void_(),
           ast::StatementList{
               create<ast::VariableDeclStatement>(
                   Source{Source::Location{12, 34}}, var),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });
  AST().Functions().Add(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0015: runtime arrays may only appear as the last member "
            "of a struct");
}

TEST_F(ValidatorTypeTest, RuntimeArrayAsParameter_Fail) {
  // fn func(a : array<u32>) {}
  // [[stage(vertex)]] fn main() {}

  auto* param =
      Var(Source{Source::Location{12, 34}}, "a", ast::StorageClass::kNone,
          ty.array<i32>(), nullptr, ast::VariableDecorationList{});

  auto* func = Func("func", ast::VariableList{param}, ty.void_(),
                    ast::StatementList{
                        create<ast::ReturnStatement>(),
                    },
                    ast::FunctionDecorationList{});
  AST().Functions().Add(func);

  auto* main =
      Func("main", ast::VariableList{}, ty.void_(),
           ast::StatementList{
               create<ast::ReturnStatement>(),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });
  AST().Functions().Add(main);

  EXPECT_TRUE(td()->Determine()) << td()->error();

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0015: runtime arrays may only appear as the last member "
            "of a struct");
}
}  // namespace
}  // namespace tint
