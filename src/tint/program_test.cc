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

#include "gtest/gtest-spi.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/test_helper.h"

namespace tint {
namespace {

using ProgramTest = ast::TestHelper;

TEST_F(ProgramTest, Unbuilt) {
    Program program;
    EXPECT_FALSE(program.IsValid());
}

TEST_F(ProgramTest, Creation) {
    Program program(std::move(*this));
    EXPECT_EQ(program.AST().Functions().Length(), 0u);
}

TEST_F(ProgramTest, EmptyIsValid) {
    Program program(std::move(*this));
    EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, IDsAreUnique) {
    Program program_a(ProgramBuilder{});
    Program program_b(ProgramBuilder{});
    Program program_c(ProgramBuilder{});
    EXPECT_NE(program_a.ID(), program_b.ID());
    EXPECT_NE(program_b.ID(), program_c.ID());
    EXPECT_NE(program_c.ID(), program_a.ID());
}

TEST_F(ProgramTest, Assert_GlobalVariable) {
    GlobalVar("var", ty.f32(), builtin::AddressSpace::kPrivate);

    Program program(std::move(*this));
    EXPECT_TRUE(program.IsValid());
}

TEST_F(ProgramTest, Assert_NullGlobalVariable) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.AST().AddGlobalVariable(nullptr);
        },
        "internal compiler error");
}

TEST_F(ProgramTest, Assert_NullTypeDecl) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.AST().AddTypeDecl(nullptr);
        },
        "internal compiler error");
}

TEST_F(ProgramTest, Assert_Null_Function) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.AST().AddFunction(nullptr);
        },
        "internal compiler error");
}

TEST_F(ProgramTest, DiagnosticsMove) {
    Diagnostics().add_error(diag::System::Program, "an error message");

    Program program_a(std::move(*this));
    EXPECT_FALSE(program_a.IsValid());
    EXPECT_EQ(program_a.Diagnostics().count(), 1u);
    EXPECT_EQ(program_a.Diagnostics().error_count(), 1u);
    EXPECT_EQ(program_a.Diagnostics().begin()->message, "an error message");

    Program program_b(std::move(program_a));
    EXPECT_FALSE(program_b.IsValid());
    EXPECT_EQ(program_b.Diagnostics().count(), 1u);
    EXPECT_EQ(program_b.Diagnostics().error_count(), 1u);
    EXPECT_EQ(program_b.Diagnostics().begin()->message, "an error message");
}

TEST_F(ProgramTest, ReuseMovedFromVariable) {
    Program a(std::move(*this));
    EXPECT_TRUE(a.IsValid());

    Program b = std::move(a);
    EXPECT_TRUE(b.IsValid());

    a = std::move(b);
    EXPECT_TRUE(a.IsValid());
}

}  // namespace
}  // namespace tint
