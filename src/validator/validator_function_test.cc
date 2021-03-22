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

#include "src/ast/stage_decoration.h"
#include "src/validator/validator_test_helper.h"

namespace tint {
namespace {

class ValidateFunctionTest : public ValidatorTestHelper,
                             public testing::Test {};

TEST_F(ValidateFunctionTest, VoidFunctionEndWithoutReturnStatement_Pass) {
  // [[stage(vertex)]]
  // fn func -> void { var a:i32 = 2; }
  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{},
       ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate());
}

TEST_F(ValidateFunctionTest,
       VoidFunctionEndWithoutReturnStatementEmptyBody_Pass) {
  // [[stage(vertex)]]
  // fn func -> void {}

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{},
       ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate());
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_Pass) {
  // [[stage(vertex)]]
  // fn func -> void { return; }

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_fail) {
  // fn func -> void { return 2; }
  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2)),
       },
       ast::DecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v.error(),
            "12:34 v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__void'");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementTypeF32_pass) {
  // fn func -> f32 { return 2.0; }
  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2.f)),
       },
       ast::DecorationList{});
  Func("main", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate());
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementTypeF32_fail) {
  // fn func -> f32 { return 2; }
  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2)),
       },
       ast::DecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v.error(),
            "12:34 v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__f32'");
}

TEST_F(ValidateFunctionTest,
       FunctionTypeMustMatchReturnStatementTypeF32Alias_pass) {
  // type myf32 = f32;
  // fn func -> myf32 { return 2.0; }
  auto* myf32 = ty.alias("myf32", ty.f32());
  Func("func", ast::VariableList{}, myf32,
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2.f)),
       },
       ast::DecorationList{});
  Func("main", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate());
}

TEST_F(ValidateFunctionTest,
       FunctionTypeMustMatchReturnStatementTypeF32Alias_fail) {
  // type myf32 = f32;
  // fn func -> myf32 { return 2; }
  auto* myf32 = ty.alias("myf32", ty.f32());
  Func("func", ast::VariableList{}, myf32,
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2u)),
       },
       ast::DecorationList{});
  Func("main", ast::VariableList{}, ty.void_(), ast::StatementList{},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(
      v.error(),
      "12:34 v-000y: return statement type must match its function "
      "return type, returned '__u32', expected '__alias_tint_symbol_1__f32'");
}

TEST_F(ValidateFunctionTest, PipelineStage_MustBeUnique_Fail) {
  // [[stage(fragment)]]
  // [[stage(vertex)]]
  // fn main() -> void { return; }
  Func(Source{Source::Location{12, 34}}, "main", ast::VariableList{},
       ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
           create<ast::StageDecoration>(ast::PipelineStage::kFragment),
       });

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(
      v.error(),
      "12:34 v-0020: only one stage decoration permitted per entry point");
}

TEST_F(ValidateFunctionTest, OnePipelineStageFunctionMustBePresent_Pass) {
  // [[stage(vertex)]]
  // fn vtx_func() -> void { return; }

  Func("vtx_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

TEST_F(ValidateFunctionTest, OnePipelineStageFunctionMustBePresent_Fail) {
  // fn vtx_func() -> void { return; }
  Func("vtx_func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::DecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "v-0003: At least one of vertex, fragment or compute shader must "
            "be present");
}

TEST_F(ValidateFunctionTest, FunctionVarInitWithParam) {
  // fn foo(bar : f32) -> void{
  //   var baz : f32 = bar;
  // }

  auto* bar = Var("bar", ty.f32(), ast::StorageClass::kFunction);
  auto* baz = Var("baz", ty.f32(), ast::StorageClass::kFunction, Expr("bar"));

  Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

TEST_F(ValidateFunctionTest, FunctionConstInitWithParam) {
  // fn foo(bar : f32) -> void{
  //   const baz : f32 = bar;
  // }

  auto* bar = Var("bar", ty.f32(), ast::StorageClass::kFunction);
  auto* baz = Const("baz", ty.f32(), Expr("bar"));

  Func("foo", ast::VariableList{bar}, ty.void_(), ast::StatementList{Decl(baz)},
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate()) << v.error();
}

}  // namespace
}  // namespace tint
