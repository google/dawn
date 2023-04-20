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

TEST_F(DiagnosticDirectiveTest, Name) {
    auto* d = DiagnosticDirective(Source{{{10, 5}, {10, 15}}},
                                  builtin::DiagnosticSeverity::kWarning, "foo");
    EXPECT_EQ(d->source.range.begin.line, 10u);
    EXPECT_EQ(d->source.range.begin.column, 5u);
    EXPECT_EQ(d->source.range.end.line, 10u);
    EXPECT_EQ(d->source.range.end.column, 15u);
    EXPECT_EQ(d->control.severity, builtin::DiagnosticSeverity::kWarning);
    EXPECT_EQ(d->control.rule_name->category, nullptr);
    CheckIdentifier(d->control.rule_name->name, "foo");
}

TEST_F(DiagnosticDirectiveTest, CategoryAndName) {
    auto* d = DiagnosticDirective(Source{{{10, 5}, {10, 15}}},
                                  builtin::DiagnosticSeverity::kWarning, "foo", "bar");
    EXPECT_EQ(d->source.range.begin.line, 10u);
    EXPECT_EQ(d->source.range.begin.column, 5u);
    EXPECT_EQ(d->source.range.end.line, 10u);
    EXPECT_EQ(d->source.range.end.column, 15u);
    EXPECT_EQ(d->control.severity, builtin::DiagnosticSeverity::kWarning);
    CheckIdentifier(d->control.rule_name->category, "foo");
    CheckIdentifier(d->control.rule_name->name, "bar");
}

}  // namespace
}  // namespace tint::ast
