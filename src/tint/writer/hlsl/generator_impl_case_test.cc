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

using HlslGeneratorImplTest_Case = TestHelper;

TEST_F(HlslGeneratorImplTest_Case, Emit_Case) {
    auto* s =
        Switch(1_i, Case(CaseSelector(5_i), Block(create<ast::BreakStatement>())), DefaultCase());
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitCase(s, 0)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  case 5: {
    break;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Case, Emit_Case_BreaksByDefault) {
    auto* s = Switch(1_i, Case(CaseSelector(5_i), Block()), DefaultCase());
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitCase(s, 0)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  case 5: {
    break;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Case, Emit_Case_MultipleSelectors) {
    auto* s = Switch(1_i,
                     Case(utils::Vector{CaseSelector(5_i), CaseSelector(6_i)},
                          Block(create<ast::BreakStatement>())),
                     DefaultCase());
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitCase(s, 0)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  case 5:
  case 6: {
    break;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Case, Emit_Case_Default) {
    auto* s = Switch(1_i, DefaultCase(Block(create<ast::BreakStatement>())));
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitCase(s, 0u)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(  default: {
    break;
  }
)");
}

}  // namespace
}  // namespace tint::writer::hlsl
