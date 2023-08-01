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

#include "src/tint/lang/msl/writer/ast_printer/helper_test.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::msl::writer {
namespace {

using MslASTPrinterTest = TestHelper;

TEST_F(MslASTPrinterTest, Emit_Switch) {
    auto* cond = Var("cond", ty.i32());

    auto* def_body = Block(create<ast::BreakStatement>());
    auto* def = Case(DefaultCaseSelector(), def_body);

    auto* case_body = Block(create<ast::BreakStatement>());
    auto* case_stmt = Case(CaseSelector(5_i), case_body);

    Vector body{case_stmt, def};
    auto* s = Switch(cond, body);
    WrapInFunction(cond, s);
    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

TEST_F(MslASTPrinterTest, Emit_Switch_MixedDefault) {
    auto* cond = Var("cond", ty.i32());

    auto* def_body = Block(create<ast::BreakStatement>());
    auto* def = Case(Vector{CaseSelector(5_i), DefaultCaseSelector()}, def_body);

    auto* s = Switch(cond, def);
    WrapInFunction(cond, s);
    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    ASSERT_TRUE(gen.EmitStatement(s)) << gen.Diagnostics();
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5:
    default: {
      break;
    }
  }
)");
}

}  // namespace
}  // namespace tint::msl::writer
