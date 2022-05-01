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
#include "src/tint/ast/test_helper.h"
#include "src/tint/clone_context.h"

namespace tint::ast {
namespace {

using ModuleTest = TestHelper;

TEST_F(ModuleTest, Creation) {
    EXPECT_EQ(Program(std::move(*this)).AST().Functions().size(), 0u);
}

TEST_F(ModuleTest, LookupFunction) {
    auto* func = Func("main", VariableList{}, ty.f32(), StatementList{}, ast::AttributeList{});

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
            b1.AST().AddFunction(b2.create<ast::Function>(b2.Symbols().Register("func"),
                                                          VariableList{}, b2.ty.f32(), b2.Block(),
                                                          AttributeList{}, AttributeList{}));
        },
        "internal compiler error");
}

TEST_F(ModuleTest, Assert_DifferentProgramID_GlobalVariable) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.AST().AddGlobalVariable(b2.Var("var", b2.ty.i32(), ast::StorageClass::kPrivate));
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
        b.Global("V", b.ty.i32(), ast::StorageClass::kPrivate);
        return Program(std::move(b));
    }();

    // Clone the program, using ReplaceAll() to create new module-scope
    // declarations. We want to test that these are added just before the
    // declaration that triggered the ReplaceAll().
    ProgramBuilder cloned;
    CloneContext ctx(&cloned, &p);
    ctx.ReplaceAll([&](const ast::Function*) -> const ast::Function* {
        ctx.dst->Alias("inserted_before_F", cloned.ty.u32());
        return nullptr;
    });
    ctx.ReplaceAll([&](const ast::Alias*) -> const ast::Alias* {
        ctx.dst->Alias("inserted_before_A", cloned.ty.u32());
        return nullptr;
    });
    ctx.ReplaceAll([&](const ast::Variable*) -> const ast::Variable* {
        ctx.dst->Alias("inserted_before_V", cloned.ty.u32());
        return nullptr;
    });
    ctx.Clone();

    auto& decls = cloned.AST().GlobalDeclarations();
    ASSERT_EQ(decls.size(), 6u);
    EXPECT_TRUE(decls[1]->Is<ast::Function>());
    EXPECT_TRUE(decls[3]->Is<ast::Alias>());
    EXPECT_TRUE(decls[5]->Is<ast::Variable>());

    ASSERT_TRUE(decls[0]->Is<ast::Alias>());
    ASSERT_TRUE(decls[2]->Is<ast::Alias>());
    ASSERT_TRUE(decls[4]->Is<ast::Alias>());

    ASSERT_EQ(cloned.Symbols().NameFor(decls[0]->As<ast::Alias>()->name), "inserted_before_F");
    ASSERT_EQ(cloned.Symbols().NameFor(decls[2]->As<ast::Alias>()->name), "inserted_before_A");
    ASSERT_EQ(cloned.Symbols().NameFor(decls[4]->As<ast::Alias>()->name), "inserted_before_V");
}

}  // namespace
}  // namespace tint::ast
