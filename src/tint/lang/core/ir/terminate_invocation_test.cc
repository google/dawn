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

#include "src/tint/lang/core/ir/terminate_invocation.h"
#include "gtest/gtest.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::core::ir {
namespace {

using IR_TerminateInvocationTest = IRTestHelper;

TEST_F(IR_TerminateInvocationTest, Clone) {
    auto* ti = b.TerminateInvocation();
    auto* new_ti = clone_ctx.Clone(ti);

    EXPECT_NE(ti, new_ti);
    EXPECT_NE(nullptr, new_ti);
    EXPECT_NE(ti, new_ti);
}

}  // namespace
}  // namespace tint::core::ir
