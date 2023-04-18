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
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied->
// See the License for the specific language governing permissions and
// limitations under the License.

#include "gtest/gtest-spi.h"

#include "src/tint/ast/test_helper.h"
#include "src/tint/builtin/builtin_value.h"

namespace tint::ast {
namespace {

using BuiltinAttributeTest = TestHelper;

TEST_F(BuiltinAttributeTest, Creation) {
    auto* d = Builtin(builtin::BuiltinValue::kFragDepth);
    CheckIdentifier(d->builtin, "frag_depth");
}

TEST_F(BuiltinAttributeTest, Assert_Null_Builtin) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Builtin(nullptr);
        },
        "internal compiler error");
}

TEST_F(BuiltinAttributeTest, Assert_DifferentProgramID_Builtin) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Builtin(b2.Expr("bang"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
