// Copyright 2023 The Tint Authors.
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

#include "src/tint/sem/test_helper.h"

#include "src/tint/sem/module.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::sem {
namespace {

class DiagnosticSeverityTest : public TestHelper {
  protected:
    /// Create a program with two functions, setting the severity for "chromium.unreachable_code"
    /// using an attribute. Test that we correctly track the severity of the filter for the
    /// functions and the statements with them.
    /// @param global_severity the global severity of the "chromium.unreachable_code" filter
    void Run(builtin::DiagnosticSeverity global_severity) {
        // @diagnostic(off, chromium.unreachable_code)
        // fn foo() {
        //   @diagnostic(info, chromium.unreachable_code) {
        //     @diagnostic(error, chromium.unreachable_code)
        //     if (true) @diagnostic(warning, chromium.unreachable_code) {
        //       return;
        //     } else if (false) {
        //       return;
        //     } else @diagnostic(info, chromium.unreachable_code) {
        //       return;
        //     }
        //     return;
        //
        //     @diagnostic(error, chromium.unreachable_code)
        //     switch (42) @diagnostic(off, chromium.unreachable_code) {
        //       case 0 @diagnostic(warning, chromium.unreachable_code) {
        //         return;
        //       }
        //       default {
        //         return;
        //       }
        //     }
        //
        //     @diagnostic(error, chromium.unreachable_code)
        //     for (var i = 0; false; i++) @diagnostic(warning, chromium.unreachable_code) {
        //       return;
        //     }
        //
        //     @diagnostic(warning, chromium.unreachable_code)
        //     loop @diagnostic(off, chromium.unreachable_code) {
        //       return;
        //       continuing @diagnostic(info, chromium.unreachable_code) {
        //         break if true;
        //       }
        //     }
        //
        //     @diagnostic(error, chromium.unreachable_code)
        //     while (false) @diagnostic(warning, chromium.unreachable_code) {
        //       return;
        //     }
        //   }
        // }
        //
        // fn bar() {
        //   return;
        // }
        auto rule = builtin::ChromiumDiagnosticRule::kUnreachableCode;
        auto func_severity = builtin::DiagnosticSeverity::kOff;
        auto block_severity = builtin::DiagnosticSeverity::kInfo;
        auto if_severity = builtin::DiagnosticSeverity::kError;
        auto if_body_severity = builtin::DiagnosticSeverity::kWarning;
        auto else_body_severity = builtin::DiagnosticSeverity::kInfo;
        auto switch_severity = builtin::DiagnosticSeverity::kError;
        auto switch_body_severity = builtin::DiagnosticSeverity::kOff;
        auto case_severity = builtin::DiagnosticSeverity::kWarning;
        auto for_severity = builtin::DiagnosticSeverity::kError;
        auto for_body_severity = builtin::DiagnosticSeverity::kWarning;
        auto loop_severity = builtin::DiagnosticSeverity::kWarning;
        auto loop_body_severity = builtin::DiagnosticSeverity::kOff;
        auto continuing_severity = builtin::DiagnosticSeverity::kInfo;
        auto while_severity = builtin::DiagnosticSeverity::kError;
        auto while_body_severity = builtin::DiagnosticSeverity::kWarning;
        auto attr = [&](auto severity) {
            return utils::Vector{DiagnosticAttribute(severity, "chromium", "unreachable_code")};
        };

        auto* return_foo_if = Return();
        auto* return_foo_elseif = Return();
        auto* return_foo_else = Return();
        auto* return_foo_block = Return();
        auto* return_foo_case = Return();
        auto* return_foo_default = Return();
        auto* return_foo_for = Return();
        auto* return_foo_loop = Return();
        auto* return_foo_while = Return();
        auto* breakif_foo_continuing = BreakIf(Expr(true));
        auto* else_stmt = Block(utils::Vector{return_foo_else}, attr(else_body_severity));
        auto* elseif = If(Expr(false), Block(return_foo_elseif), Else(else_stmt));
        auto* if_foo = If(Expr(true), Block(utils::Vector{return_foo_if}, attr(if_body_severity)),
                          Else(elseif), attr(if_severity));
        auto* case_stmt =
            Case(CaseSelector(0_a), Block(utils::Vector{return_foo_case}, attr(case_severity)));
        auto* default_stmt = DefaultCase(Block(return_foo_default));
        auto* swtch = Switch(42_a, utils::Vector{case_stmt, default_stmt}, attr(switch_severity),
                             attr(switch_body_severity));
        auto* fl =
            For(Decl(Var("i", ty.i32())), false, Increment("i"),
                Block(utils::Vector{return_foo_for}, attr(for_body_severity)), attr(for_severity));
        auto* l = Loop(Block(utils::Vector{return_foo_loop}, attr(loop_body_severity)),
                       Block(utils::Vector{breakif_foo_continuing}, attr(continuing_severity)),
                       attr(loop_severity));
        auto* wl = While(false, Block(utils::Vector{return_foo_while}, attr(while_body_severity)),
                         attr(while_severity));
        auto* block_1 =
            Block(utils::Vector{if_foo, return_foo_block, swtch, fl, l, wl}, attr(block_severity));
        auto* func_attr = DiagnosticAttribute(func_severity, "chromium", "unreachable_code");
        auto* foo = Func("foo", {}, ty.void_(), utils::Vector{block_1}, utils::Vector{func_attr});

        auto* return_bar = Return();
        auto* bar = Func("bar", {}, ty.void_(), utils::Vector{return_bar});

        auto p = Build();
        EXPECT_TRUE(p.IsValid()) << p.Diagnostics().str();

        EXPECT_EQ(p.Sem().DiagnosticSeverity(foo, rule), func_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(block_1, rule), block_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo->condition, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(if_foo->body, rule), if_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_if, rule), if_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif->condition, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(elseif->body, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_elseif, rule), if_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(else_stmt, rule), else_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_else, rule), else_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(swtch, rule), switch_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(swtch->condition, rule), switch_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(case_stmt, rule), switch_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(case_stmt->body, rule), case_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_case, rule), case_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(default_stmt, rule), switch_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_default, rule), switch_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl, rule), while_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->initializer, rule), for_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->condition, rule), for_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->continuing, rule), for_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(fl->body, rule), for_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_for, rule), for_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(l, rule), loop_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(l->body, rule), loop_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(l->continuing, rule), continuing_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(breakif_foo_continuing, rule), continuing_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_loop, rule), loop_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(wl, rule), while_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(wl->condition, rule), while_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(wl->body, rule), while_body_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_foo_while, rule), while_body_severity);

        EXPECT_EQ(p.Sem().DiagnosticSeverity(bar, rule), global_severity);
        EXPECT_EQ(p.Sem().DiagnosticSeverity(return_bar, rule), global_severity);
    }
};

TEST_F(DiagnosticSeverityTest, WithDirective) {
    DiagnosticDirective(builtin::DiagnosticSeverity::kError, "chromium", "unreachable_code");
    Run(builtin::DiagnosticSeverity::kError);
}

TEST_F(DiagnosticSeverityTest, WithoutDirective) {
    Run(builtin::DiagnosticSeverity::kWarning);
}

}  // namespace
}  // namespace tint::sem
