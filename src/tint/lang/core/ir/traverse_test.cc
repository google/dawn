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

#include "src/tint/lang/core/ir/traverse.h"

#include "gmock/gmock.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::core::ir {
namespace {

using IR_TraverseTest = IRTestHelper;

TEST_F(IR_TraverseTest, Blocks) {
    Vector<Instruction*, 8> expect;

    auto fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        expect.Push(b.Var<function, i32>());

        auto* if_ = b.If(true);
        expect.Push(if_);
        b.Append(if_->True(), [&] {
            auto* if2_ = b.If(true);
            expect.Push(if2_);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            expect.Push(b.ExitIf(if_));
        });

        b.Append(if_->False(), [&] { expect.Push(b.ExitIf(if_)); });

        auto* loop_ = b.Loop();
        expect.Push(loop_);
        b.Append(loop_->Initializer(), [&] { expect.Push(b.NextIteration(loop_)); });
        b.Append(loop_->Body(), [&] {
            auto* if2_ = b.If(true);
            expect.Push(if2_);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            expect.Push(b.Continue(loop_));
        });
        b.Append(loop_->Continuing(), [&] { expect.Push(b.NextIteration(loop_)); });

        auto* switch_ = b.Switch(1_i);
        expect.Push(switch_);

        auto* case_0 = b.Case(switch_, {Switch::CaseSelector{b.Constant(0_i)}});
        b.Append(case_0, [&] { expect.Push(b.Var<function, i32>()); });

        auto* case_1 = b.Case(switch_, {Switch::CaseSelector{b.Constant(1_i)}});
        b.Append(case_1, [&] { expect.Push(b.Var<function, i32>()); });

        expect.Push(b.Var<function, i32>());
    });

    Vector<Instruction*, 8> got;
    Traverse(fn->Block(), [&](Instruction* inst) { got.Push(inst); });

    EXPECT_THAT(got, testing::ContainerEq(expect));
}

TEST_F(IR_TraverseTest, Filtered) {
    Vector<ExitIf*, 8> expect;

    auto fn = b.Function("f", ty.void_());
    b.Append(fn->Block(), [&] {
        b.Var<function, i32>();

        auto* if_ = b.If(true);
        b.Append(if_->True(), [&] {
            auto* if2_ = b.If(true);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            expect.Push(b.ExitIf(if_));
        });

        b.Append(if_->False(), [&] { expect.Push(b.ExitIf(if_)); });

        auto* loop_ = b.Loop();
        b.Append(loop_->Initializer(), [&] { b.NextIteration(loop_); });
        b.Append(loop_->Body(), [&] {
            auto* if2_ = b.If(true);
            b.Append(if2_->True(), [&] { expect.Push(b.ExitIf(if2_)); });
            b.Continue(loop_);
        });
        b.Append(loop_->Continuing(), [&] { b.NextIteration(loop_); });

        auto* switch_ = b.Switch(1_i);

        auto* case_0 = b.Case(switch_, {Switch::CaseSelector{b.Constant(0_i)}});
        b.Append(case_0, [&] { b.Var<function, i32>(); });

        auto* case_1 = b.Case(switch_, {Switch::CaseSelector{b.Constant(1_i)}});
        b.Append(case_1, [&] { b.Var<function, i32>(); });

        b.Var<function, i32>();
    });

    Vector<ExitIf*, 8> got;
    Traverse(fn->Block(), [&](ExitIf* inst) { got.Push(inst); });

    EXPECT_THAT(got, testing::ContainerEq(expect));
}

}  // namespace
}  // namespace tint::core::ir
