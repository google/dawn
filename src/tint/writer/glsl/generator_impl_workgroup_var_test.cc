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
#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/glsl/test_helper.h"

namespace tint::writer::glsl {
namespace {
using ::testing::HasSubstr;

using GlslGeneratorImplTest_WorkgroupVar = TestHelper;

TEST_F(GlslGeneratorImplTest_WorkgroupVar, Basic) {
    Global("wg", ty.f32(), ast::StorageClass::kWorkgroup);

    Func("main", {}, ty.void_(), {Assign("wg", 1.2f)},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1),
         });
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("shared float wg;\n"));
}

TEST_F(GlslGeneratorImplTest_WorkgroupVar, Aliased) {
    auto* alias = Alias("F32", ty.f32());

    Global("wg", ty.Of(alias), ast::StorageClass::kWorkgroup);

    Func("main", {}, ty.void_(), {Assign("wg", 1.2f)},
         {
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1),
         });
    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate()) << gen.error();
    EXPECT_THAT(gen.result(), HasSubstr("shared float wg;\n"));
}

}  // namespace
}  // namespace tint::writer::glsl
