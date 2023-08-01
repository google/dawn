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

#include "src/tint/lang/core/ir/function.h"
#include "gtest/gtest-spi.h"
#include "src/tint/lang/core/ir/ir_helper_test.h"

namespace tint::ir {
namespace {

using namespace tint::number_suffixes;  // NOLINT
using IR_FunctionTest = IRTestHelper;

TEST_F(IR_FunctionTest, Fail_NullReturnType) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            b.Function("my_func", nullptr);
        },
        "");
}

TEST_F(IR_FunctionTest, Fail_DoubleReturnBuiltin) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* f = b.Function("my_func", mod.Types().void_());
            f->SetReturnBuiltin(Function::ReturnBuiltin::kFragDepth);
            f->SetReturnBuiltin(Function::ReturnBuiltin::kPosition);
        },
        "");
}

TEST_F(IR_FunctionTest, Fail_NullParam) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* f = b.Function("my_func", mod.Types().void_());
            f->SetParams({nullptr});
        },
        "");
}

TEST_F(IR_FunctionTest, Fail_NullBlock) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};
            auto* f = b.Function("my_func", mod.Types().void_());
            f->SetBlock(nullptr);
        },
        "");
}

}  // namespace
}  // namespace tint::ir
