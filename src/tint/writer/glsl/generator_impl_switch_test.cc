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

#include "src/tint/writer/glsl/test_helper.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Switch = TestHelper;

TEST_F(GlslGeneratorImplTest_Switch, Emit_Switch) {
    GlobalVar("cond", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* def_body = Block(create<ast::BreakStatement>());
    auto* def = create<ast::CaseStatement>(utils::Vector{DefaultCaseSelector()}, def_body);

    auto* case_body = Block(create<ast::BreakStatement>());
    auto* case_stmt = create<ast::CaseStatement>(utils::Vector{CaseSelector(5_i)}, case_body);

    auto* cond = Expr("cond");
    auto* s = Switch(cond, utils::Vector{case_stmt, def});
    WrapInFunction(s);

    GeneratorImpl& gen = Build();
    gen.increment_indent();
    gen.EmitStatement(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  switch(cond) {
    case 5: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

TEST_F(GlslGeneratorImplTest_Switch, Emit_Switch_MixedDefault) {
    GlobalVar("cond", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* def_body = Block(create<ast::BreakStatement>());
    auto* def = create<ast::CaseStatement>(utils::Vector{CaseSelector(5_i), DefaultCaseSelector()},
                                           def_body);

    auto* cond = Expr("cond");
    auto* s = Switch(cond, utils::Vector{def});
    WrapInFunction(s);

    GeneratorImpl& gen = Build();
    gen.increment_indent();
    gen.EmitStatement(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.result(), R"(  switch(cond) {
    case 5:
    default: {
      break;
    }
  }
)");
}

}  // namespace
}  // namespace tint::writer::glsl
