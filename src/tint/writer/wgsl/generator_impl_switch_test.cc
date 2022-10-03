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

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::wgsl {
namespace {

using WgslGeneratorImplTest = TestHelper;

TEST_F(WgslGeneratorImplTest, Emit_Switch) {
    GlobalVar("cond", ty.i32(), ast::AddressSpace::kPrivate);

    auto* def_body = Block(create<ast::BreakStatement>());
    auto* def = create<ast::CaseStatement>(utils::Empty, def_body);

    utils::Vector case_val{Expr(5_i)};

    auto* case_body = Block(create<ast::BreakStatement>());

    auto* case_stmt = create<ast::CaseStatement>(case_val, case_body);

    utils::Vector body{
        case_stmt,
        def,
    };

    auto* cond = Expr("cond");
    auto* s = create<ast::SwitchStatement>(cond, body);
    WrapInFunction(s);

    GeneratorImpl& gen = Build();

    gen.increment_indent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.error();
    EXPECT_EQ(gen.result(), R"(  switch(cond) {
    case 5i: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

}  // namespace
}  // namespace tint::writer::wgsl
