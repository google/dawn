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

#include "src/tint/writer/hlsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Return = TestHelper;

TEST_F(HlslGeneratorImplTest_Return, Emit_Return) {
    auto* r = Return();
    WrapInFunction(r);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(r)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), "  return;\n");
}

TEST_F(HlslGeneratorImplTest_Return, Emit_ReturnWithValue) {
    auto* r = Return(123_i);
    Func("f", utils::Empty, ty.i32(), utils::Vector{r});

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(r)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), "  return 123;\n");
}

}  // namespace
}  // namespace tint::writer::hlsl
