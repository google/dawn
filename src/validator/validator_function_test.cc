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
       ast::FunctionDecorationList{
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
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_TRUE(v.Validate());
}

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatement_Fail) {
  // fn func -> int { var a:i32 = 2; }

  auto* var = Var("a", ty.i32(), ast::StorageClass::kNone, Expr(2));

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(var),
       },
       ast::FunctionDecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0002: non-void function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionEndWithoutReturnStatementEmptyBody_Fail) {
  // fn func -> int {}

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{}, ast::FunctionDecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0002: non-void function must end with a return statement");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementType_Pass) {
  // [[stage(vertex)]]
  // fn func -> void { return; }

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{
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
       ast::FunctionDecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v.error(),
            "12:34 v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__void'");
}

TEST_F(ValidateFunctionTest, FunctionTypeMustMatchReturnStatementTypeF32_fail) {
  // fn func -> f32 { return 2; }
  Func("func", ast::VariableList{}, ty.f32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Source{Source::Location{12, 34}},
                                        Expr(2)),
       },
       ast::FunctionDecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  // TODO(sarahM0): replace 000y with a rule number
  EXPECT_EQ(v.error(),
            "12:34 v-000y: return statement type must match its function "
            "return type, returned '__i32', expected '__f32'");
}

TEST_F(ValidateFunctionTest, FunctionNamesMustBeUnique_fail) {
  // fn func -> i32 { return 2; }
  // fn func -> i32 { return 2; }
  Func("func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr(2)),
       },
       ast::FunctionDecorationList{});

  Func(Source{Source::Location{12, 34}}, "func", ast::VariableList{}, ty.i32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr(2)),
       },
       ast::FunctionDecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(), "12:34 v-0016: function names must be unique 'func'");
}

TEST_F(ValidateFunctionTest, Function_WithPipelineStage_NotVoid_Fail) {
  // [[stage(vertex)]]
  // fn vtx_main() -> i32 { return 0; }

  Func(Source{Source::Location{12, 34}}, "vtx_main", ast::VariableList{},
       ty.i32(),
       ast::StatementList{
           create<ast::ReturnStatement>(Expr(0)),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0024: Entry point function must return void: 'vtx_main'");
}

TEST_F(ValidateFunctionTest, Function_WithPipelineStage_WithParams_Fail) {
  // [[stage(vertex)]]
  // fn vtx_func(a : i32) -> void { return; }

  Func(Source{Source::Location{12, 34}}, "vtx_func",
       ast::VariableList{Var("a", ty.i32(), ast::StorageClass::kNone)},
       ty.void_(),
       ast::StatementList{
           create<ast::ReturnStatement>(),
       },
       ast::FunctionDecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0023: Entry point function must accept no parameters: "
            "'vtx_func'");
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
       ast::FunctionDecorationList{
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
       ast::FunctionDecorationList{
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
       ast::FunctionDecorationList{});

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "v-0003: At least one of vertex, fragment or compute shader must "
            "be present");
}

}  // namespace
}  // namespace tint
