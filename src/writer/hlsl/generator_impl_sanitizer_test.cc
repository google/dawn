// Copyright 2021 The Tint Authors.
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
#include "src/ast/variable_decl_statement.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslSanitizerTest = TestHelper;

TEST_F(HlslSanitizerTest, PromoteArrayInitializerToConstVar) {
  auto* array_init = array<i32, 4>(1, 2, 3, 4);
  auto* array_index = IndexAccessor(array_init, 3);
  auto* pos = Var("pos", ty.i32(), ast::StorageClass::kFunction, array_index);

  Func("main", ast::VariableList{}, ty.void_(),
       ast::StatementList{
           create<ast::VariableDeclStatement>(pos),
       },
       ast::DecorationList{
           create<ast::StageDecoration>(ast::PipelineStage::kVertex),
       });

  GeneratorImpl& gen = SanitizeAndBuild();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();

  auto got = result();
  auto* expect = R"(void main() {
  const int tint_symbol_1[4] = {1, 2, 3, 4};
  int pos = tint_symbol_1[3];
  return;
}

)";
  EXPECT_EQ(expect, got);
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
