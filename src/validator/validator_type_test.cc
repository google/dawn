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

#include "src/ast/struct_block_decoration.h"
#include "src/validator/validator_test_helper.h"

#include "src/ast/stage_decoration.h"
namespace tint {
namespace {

class ValidatorTypeTest : public ValidatorTestHelper, public testing::Test {};

TEST_F(ValidatorTypeTest, RuntimeArrayInFunction_Fail) {
  /// [[stage(vertex)]]
  // fn func -> void { var a : array<i32>; }

  auto* var = Var("a", ty.array<i32>(), ast::StorageClass::kNone);

  Func("func", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(Source{Source::Location{12, 34}},
                                              var),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  ValidatorImpl& v = Build();

  EXPECT_FALSE(v.Validate());
  EXPECT_EQ(v.error(),
            "12:34 v-0015: runtime arrays may only appear as the last member "
            "of a struct");
}

}  // namespace
}  // namespace tint
