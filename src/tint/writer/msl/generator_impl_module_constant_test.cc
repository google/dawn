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

#include "src/tint/ast/id_attribute.h"
#include "src/tint/writer/msl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_ModuleConstant) {
    auto* var = GlobalConst("pos", ty.array<f32, 3>(), array<f32, 3>(1_f, 2_f, 3_f));

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitProgramConstVariable(var)) << gen.error();
    EXPECT_EQ(gen.result(), "constant float pos[3] = {1.0f, 2.0f, 3.0f};\n");
}

TEST_F(MslGeneratorImplTest, Emit_SpecConstant) {
    auto* var = Override("pos", ty.f32(), Expr(3_f),
                         ast::AttributeList{
                             Id(23),
                         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitProgramConstVariable(var)) << gen.error();
    EXPECT_EQ(gen.result(), "constant float pos [[function_constant(23)]];\n");
}

TEST_F(MslGeneratorImplTest, Emit_SpecConstant_NoId) {
    auto* var_a = Override("a", ty.f32(), nullptr,
                           ast::AttributeList{
                               Id(0),
                           });
    auto* var_b = Override("b", ty.f32(), nullptr);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitProgramConstVariable(var_a)) << gen.error();
    ASSERT_TRUE(gen.EmitProgramConstVariable(var_b)) << gen.error();
    EXPECT_EQ(gen.result(), R"(constant float a [[function_constant(0)]];
constant float b [[function_constant(1)]];
)");
}

}  // namespace
}  // namespace tint::writer::msl
