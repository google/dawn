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

#include "gmock/gmock.h"
#include "src/ast/override_decoration.h"
#include "src/ast/stage_decoration.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {
using ::testing::HasSubstr;

using HlslGeneratorImplTest_WorkgroupVar = TestHelper;

TEST_F(HlslGeneratorImplTest_WorkgroupVar, Basic) {
  Global("wg", ty.f32(), ast::StorageClass::kWorkgroup);

  Func("main", {}, ty.void_(), {Assign("wg", 1.2f)},
       {Stage(ast::PipelineStage::kCompute)});
  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("groupshared float wg;\n"));

  Validate();
}

TEST_F(HlslGeneratorImplTest_WorkgroupVar, Aliased) {
  auto* alias = ty.alias("F32", ty.f32());
  AST().AddConstructedType(alias);

  Global("wg", alias, ast::StorageClass::kWorkgroup);

  Func("main", {}, ty.void_(), {Assign("wg", 1.2f)},
       {Stage(ast::PipelineStage::kCompute)});
  GeneratorImpl& gen = Build();

  ASSERT_TRUE(gen.Generate(out)) << gen.error();
  EXPECT_THAT(result(), HasSubstr("groupshared float wg;\n"));

  Validate();
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
