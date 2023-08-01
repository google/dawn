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

#include "src/tint/lang/wgsl/writer/ast_printer/helper_test.h"

#include "gmock/gmock.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::wgsl::writer {
namespace {

using WgslASTPrinterTest = TestHelper;

TEST_F(WgslASTPrinterTest, Emit_Switch) {
    GlobalVar("cond", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* def_body = Block(create<ast::BreakStatement>());
    auto* def = Case(DefaultCaseSelector(), def_body);

    auto* case_body = Block(create<ast::BreakStatement>());
    auto* case_stmt = Case(Vector{CaseSelector(5_i)}, case_body);

    Vector body{
        case_stmt,
        def,
    };

    auto* cond = Expr("cond");
    auto* s = Switch(cond, body);
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();

    gen.EmitStatement(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5i: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

TEST_F(WgslASTPrinterTest, Emit_Switch_MixedDefault) {
    GlobalVar("cond", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* def_body = Block(create<ast::BreakStatement>());
    auto* def = Case(Vector{CaseSelector(5_i), DefaultCaseSelector()}, def_body);

    auto* cond = Expr("cond");
    auto* s = Switch(cond, Vector{def});
    WrapInFunction(s);

    ASTPrinter& gen = Build();

    gen.IncrementIndent();
    gen.EmitStatement(s);
    EXPECT_THAT(gen.Diagnostics(), testing::IsEmpty());
    EXPECT_EQ(gen.Result(), R"(  switch(cond) {
    case 5i, default: {
      break;
    }
  }
)");
}

}  // namespace
}  // namespace tint::wgsl::writer
