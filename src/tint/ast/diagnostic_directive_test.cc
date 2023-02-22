// Copyright 2022 The Tint Authors.
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

#include "src/tint/ast/diagnostic_directive.h"

#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using DiagnosticDirectiveTest = TestHelper;

TEST_F(DiagnosticDirectiveTest, Creation) {
    auto* diag = DiagnosticDirective(Source{{{10, 5}, {10, 15}}},
                                     builtin::DiagnosticSeverity::kWarning, "foo");
    EXPECT_EQ(diag->source.range.begin.line, 10u);
    EXPECT_EQ(diag->source.range.begin.column, 5u);
    EXPECT_EQ(diag->source.range.end.line, 10u);
    EXPECT_EQ(diag->source.range.end.column, 15u);
    EXPECT_EQ(diag->control.severity, builtin::DiagnosticSeverity::kWarning);
    CheckIdentifier(Symbols(), diag->control.rule_name, "foo");
}

}  // namespace
}  // namespace tint::ast
