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

#include "gmock/gmock.h"
#include "gtest/gtest-spi.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/clone_context.h"

namespace tint::ast {
namespace {

using ModuleTest = TestHelper;

TEST_F(ModuleTest, Creation) {
    EXPECT_EQ(Program(std::move(*this)).AST().Functions().Length(), 0u);
}

TEST_F(ModuleTest, LookupFunction) {
    auto* func = Func("main", {}, ty.f32(), {});

    Program program(std::move(*this));
    EXPECT_EQ(func, program.AST().Functions().Find(program.Symbols().Get("main")));
}

TEST_F(ModuleTest, LookupFunctionMissing) {
    Program program(std::move(*this));
    EXPECT_EQ(nullptr, program.AST().Functions().Find(program.Symbols().Get("Missing")));
}

TEST_F(ModuleTest, Assert_Null_GlobalVariable) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder builder;
            builder.AST().AddGlobalVariable(nullptr);
        },
        "internal compiler error");
}

TEST_F(ModuleTest, Assert_Null_TypeDecl) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder builder;
            builder.AST().AddTypeDecl(nullptr);
        },
        "internal compiler error");
}

TEST_F(ModuleTest, Assert_DifferentProgramID_Function) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.AST().AddFunction(b2.create<Function>(b2.Ident("func"), utils::Empty, b2.ty.f32(),
                                                     b2.Block(), utils::Empty, utils::Empty));
        },
        "internal compiler error");
}

TEST_F(ModuleTest, Assert_DifferentProgramID_GlobalVariable) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.AST().AddGlobalVariable(b2.Var("var", b2.ty.i32(), builtin::AddressSpace::kPrivate));
        },
        "internal compiler error");
}

TEST_F(ModuleTest, Assert_Null_Function) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder builder;
            builder.AST().AddFunction(nullptr);
        },
        "internal compiler error");
}

TEST_F(ModuleTest, CloneOrder) {
    // Create a program with a function, alias decl and var decl.
    Program p = [] {
        ProgramBuilder b;
        b.Func("F", {}, b.ty.void_(), {});
        b.Alias("A", b.ty.u32());
        b.GlobalVar("V", b.ty.i32(), builtin::AddressSpace::kPrivate);
        return Program(std::move(b));
    }();

    // Clone the program, using ReplaceAll() to create new module-scope
    // declarations. We want to test that these are added just before the
    // declaration that triggered the ReplaceAll().
    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &p);
    ctx.ReplaceAll([&](const Function*) -> const Function* {
        ctx.dst->Alias("inserted_before_F", cloned.ty.u32());
        return nullptr;
    });
    ctx.ReplaceAll([&](const ast::Alias*) -> const ast::Alias* {
        ctx.dst->Alias("inserted_before_A", cloned.ty.u32());
        return nullptr;
    });
    ctx.ReplaceAll([&](const Variable*) -> const Variable* {
        ctx.dst->Alias("inserted_before_V", cloned.ty.u32());
        return nullptr;
    });
    ctx.Clone();

    auto& decls = cloned.AST().GlobalDeclarations();
    ASSERT_EQ(decls.Length(), 6u);
    EXPECT_TRUE(decls[1]->Is<Function>());
    EXPECT_TRUE(decls[3]->Is<ast::Alias>());
    EXPECT_TRUE(decls[5]->Is<Variable>());

    ASSERT_TRUE(decls[0]->Is<ast::Alias>());
    ASSERT_TRUE(decls[2]->Is<ast::Alias>());
    ASSERT_TRUE(decls[4]->Is<ast::Alias>());

    ASSERT_EQ(decls[0]->As<ast::Alias>()->name->symbol.Name(), "inserted_before_F");
    ASSERT_EQ(decls[2]->As<ast::Alias>()->name->symbol.Name(), "inserted_before_A");
    ASSERT_EQ(decls[4]->As<ast::Alias>()->name->symbol.Name(), "inserted_before_V");
}

TEST_F(ModuleTest, Directives) {
    auto* enable_1 = Enable(builtin::Extension::kF16);
    auto* diagnostic_1 = DiagnosticDirective(builtin::DiagnosticSeverity::kWarning, "foo");
    auto* enable_2 = Enable(builtin::Extension::kChromiumExperimentalFullPtrParameters);
    auto* diagnostic_2 = DiagnosticDirective(builtin::DiagnosticSeverity::kOff, "bar");

    this->SetResolveOnBuild(false);
    Program program(std::move(*this));
    EXPECT_THAT(program.AST().GlobalDeclarations(), ::testing::ContainerEq(utils::Vector{
                                                        enable_1,
                                                        diagnostic_1,
                                                        enable_2,
                                                        diagnostic_2,
                                                    }));
    EXPECT_THAT(program.AST().Enables(), ::testing::ContainerEq(utils::Vector{
                                             enable_1,
                                             enable_2,
                                         }));
    EXPECT_THAT(program.AST().DiagnosticDirectives(), ::testing::ContainerEq(utils::Vector{
                                                          diagnostic_1,
                                                          diagnostic_2,
                                                      }));
}

}  // namespace
}  // namespace tint::ast
