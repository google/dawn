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

#include "src/tint/writer/wgsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Case) {
    auto* s =
        Switch(1_i, Case(CaseSelector(5_i), Block(create<ast::BreakStatement>())), DefaultCase());
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    gen.EmitCase(s->body[0]);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  case 5i: {
    break;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Case_MultipleSelectors) {
    auto* s = Switch(1_i,
                     Case(
                         utils::Vector{
                             CaseSelector(5_i),
                             CaseSelector(6_i),
                         },
                         Block(create<ast::BreakStatement>())),
                     DefaultCase());
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    gen.EmitCase(s->body[0]);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  case 5i, 6i: {
    break;
  }
)");
}

TEST_F(WgslGeneratorImplTest, Emit_Case_Default) {
    auto* s = Switch(1_i, DefaultCase(Block(create<ast::BreakStatement>())));
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    gen.EmitCase(s->body[0]);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  default: {
    break;
  }
)");
}

}  // namespace
}  // namespace tint::writer::wgsl
