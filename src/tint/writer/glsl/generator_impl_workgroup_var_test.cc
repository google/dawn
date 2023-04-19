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

#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/stage_attribute.h"
#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using ::testing::HasSubstr;

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_WorkgroupVar = TestHelper;

TEST_F(GlslGeneratorImplTest_WorkgroupVar, Basic) {
    GlobalVar("wg", ty.f32(), builtin::AddressSpace::kWorkgroup);

    Func("main", utils::Empty, ty.void_(), utils::Vector{Assign("wg", 1.2_f)},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("shared float wg;\n"));
}

TEST_F(GlslGeneratorImplTest_WorkgroupVar, Aliased) {
    auto* alias = Alias("F32", ty.f32());

    GlobalVar("wg", ty.Of(alias), builtin::AddressSpace::kWorkgroup);

    Func("main", utils::Empty, ty.void_(), utils::Vector{Assign("wg", 1.2_f)},
         utils::Vector{
             Stage(ast::PipelineStage::kCompute),
             WorkgroupSize(1_i),
         });
    GeneratorImpl& gen = Build();
    gen.Generate();
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_THAT(gen.result(), HasSubstr("shared float wg;\n"));
}

}  // namespace
}  // namespace tint::writer::glsl
