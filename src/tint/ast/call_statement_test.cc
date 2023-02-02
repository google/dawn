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

#include "src/tint/ast/call_statement.h"

#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"

namespace tint::ast {
namespace {

using CallStatementTest = TestHelper;

TEST_F(CallStatementTest, Creation) {
    auto* expr = Call("func");

    auto* c = CallStmt(expr);
    EXPECT_EQ(c->expr, expr);
}

TEST_F(CallStatementTest, IsCall) {
    auto* c = CallStmt(Call("f"));
    EXPECT_TRUE(c->Is<CallStatement>());
}

TEST_F(CallStatementTest, Assert_Null_Call) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.CallStmt(nullptr);
        },
        "internal compiler error");
}

TEST_F(CallStatementTest, Assert_DifferentProgramID_Call) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.CallStmt(b2.Call("func"));
        },
        "internal compiler error");
}

}  // namespace
}  // namespace tint::ast
