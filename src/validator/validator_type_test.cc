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
#include "src/ast/type/alias_type.h"
#include "src/ast/type/array_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable_decl_statement.h"
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
      create<ast::Struct>(ast::StructMemberList{Member("vf", ty.f32),
                                                Member("rt", ty.array<f32>())},
                          decos);

  ast::type::Struct struct_type(mod->RegisterSymbol("Foo"), "Foo", st);

  mod->AddConstructedType(&struct_type);
  EXPECT_TRUE(v()->ValidateConstructedTypes(mod->constructed_types()));
}

TEST_F(ValidatorTypeTest, RuntimeArrayIsLastNoBlock_Fail) {
  // struct Foo {
  //   vf: f32;
  //   rt: array<f32>;
  // };

  ast::StructDecorationList decos;
  auto* st =
      create<ast::Struct>(ast::StructMemberList{Member("vf", ty.f32),
                                                Member("rt", ty.array<f32>())},
                          decos);

  ast::type::Struct struct_type(mod->RegisterSymbol("Foo"), "Foo", st);

  mod->AddConstructedType(&struct_type);
  EXPECT_FALSE(v()->ValidateConstructedTypes(mod->constructed_types()));
  EXPECT_EQ(v()->error(),
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
  auto* st =
      create<ast::Struct>(ast::StructMemberList{Member("rt", ty.array<f32>()),
                                                Member("vf", ty.f32)},
                          decos);

  ast::type::Struct struct_type(mod->RegisterSymbol("Foo"), "Foo", st);

  mod->AddConstructedType(&struct_type);
  EXPECT_FALSE(v()->ValidateConstructedTypes(mod->constructed_types()));
  EXPECT_EQ(v()->error(),
            "v-0015: runtime arrays may only appear as the last member "
            "of a struct: 'rt'");
}

TEST_F(ValidatorTypeTest, AliasRuntimeArrayIsNotLast_Fail) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  b: RTArr;
  //  a: u32;
  //}

  ast::type::Alias alias{mod->RegisterSymbol("RTArr"), "RTArr",
                         ty.array<u32>()};

  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("b", &alias), Member("a", ty.u32)}, decos);

  ast::type::Struct struct_type(mod->RegisterSymbol("s"), "s", st);
  mod->AddConstructedType(&struct_type);
  EXPECT_FALSE(v()->ValidateConstructedTypes(mod->constructed_types()));
  EXPECT_EQ(v()->error(),
            "v-0015: runtime arrays may only appear as the last member "
            "of a struct: 'b'");
}

TEST_F(ValidatorTypeTest, AliasRuntimeArrayIsLast_Pass) {
  // [[Block]]
  // type RTArr = array<u32>;
  // struct s {
  //  a: u32;
  //  b: RTArr;
  //}

  ast::type::Alias alias{mod->RegisterSymbol("RTArr"), "RTArr",
                         ty.array<u32>()};

  ast::StructDecorationList decos;
  decos.push_back(create<ast::StructBlockDecoration>());
  auto* st = create<ast::Struct>(
      ast::StructMemberList{Member("a", ty.u32), Member("b", &alias)}, decos);

  ast::type::Struct struct_type(mod->RegisterSymbol("s"), "s", st);
  mod->AddConstructedType(&struct_type);
  EXPECT_TRUE(v()->ValidateConstructedTypes(mod->constructed_types()));
}

TEST_F(ValidatorTypeTest, RuntimeArrayInFunction_Fail) {
  /// [[stage(vertex)]]
  // fn func -> void { var a : array<i32>; }

  auto* var = Var("a", ast::StorageClass::kNone, ty.array<i32>());
  auto* func =
      Func("func", ast::VariableList{}, ty.void_,
           ast::StatementList{
               create<ast::VariableDeclStatement>(
                   Source{Source::Location{12, 34}}, var),
           },
           ast::FunctionDecorationList{
               create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           });
  mod->AddFunction(func);

  EXPECT_TRUE(td()->Determine()) << td()->error();
  EXPECT_FALSE(v()->Validate(mod));
  EXPECT_EQ(v()->error(),
            "12:34 v-0015: runtime arrays may only appear as the last member "
            "of a struct: 'a'");
}

}  // namespace
}  // namespace tint
