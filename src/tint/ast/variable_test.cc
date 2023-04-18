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

#include "src/tint/ast/id_attribute.h"
#include "src/tint/ast/test_helper.h"
#include "src/tint/builtin/builtin_value.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::ast {
namespace {

using VariableTest = TestHelper;

TEST_F(VariableTest, Creation) {
    auto* v = Var("my_var", ty.i32(), builtin::AddressSpace::kFunction);

    CheckIdentifier(v->name, "my_var");
    CheckIdentifier(v->declared_address_space, "function");
    EXPECT_EQ(v->declared_access, nullptr);
    CheckIdentifier(v->type, "i32");
    EXPECT_EQ(v->source.range.begin.line, 0u);
    EXPECT_EQ(v->source.range.begin.column, 0u);
    EXPECT_EQ(v->source.range.end.line, 0u);
    EXPECT_EQ(v->source.range.end.column, 0u);
}

TEST_F(VariableTest, CreationWithSource) {
    auto* v = Var(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 5}}}, "i",
                  ty.f32(), builtin::AddressSpace::kPrivate, utils::Empty);

    CheckIdentifier(v->name, "i");
    CheckIdentifier(v->declared_address_space, "private");
    CheckIdentifier(v->type, "f32");
    EXPECT_EQ(v->source.range.begin.line, 27u);
    EXPECT_EQ(v->source.range.begin.column, 4u);
    EXPECT_EQ(v->source.range.end.line, 27u);
    EXPECT_EQ(v->source.range.end.column, 5u);
}

TEST_F(VariableTest, CreationEmpty) {
    auto* v = Var(Source{Source::Range{Source::Location{27, 4}, Source::Location{27, 7}}}, "a_var",
                  ty.i32(), builtin::AddressSpace::kWorkgroup, utils::Empty);

    CheckIdentifier(v->name, "a_var");
    CheckIdentifier(v->declared_address_space, "workgroup");
    CheckIdentifier(v->type, "i32");
    EXPECT_EQ(v->source.range.begin.line, 27u);
    EXPECT_EQ(v->source.range.begin.column, 4u);
    EXPECT_EQ(v->source.range.end.line, 27u);
    EXPECT_EQ(v->source.range.end.column, 7u);
}

TEST_F(VariableTest, Assert_Null_Name) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b;
            b.Var(static_cast<Identifier*>(nullptr), b.ty.i32());
        },
        "internal compiler error");
}

TEST_F(VariableTest, Assert_DifferentProgramID_Symbol) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Var(b2.Sym("x"), b1.ty.f32());
        },
        "internal compiler error");
}

TEST_F(VariableTest, Assert_DifferentProgramID_Initializer) {
    EXPECT_FATAL_FAILURE(
        {
            ProgramBuilder b1;
            ProgramBuilder b2;
            b1.Var("x", b1.ty.f32(), b2.Expr(1.2_f));
        },
        "internal compiler error");
}

TEST_F(VariableTest, WithAttributes) {
    auto* var = Var("my_var", ty.i32(), builtin::AddressSpace::kFunction, Location(1_u),
                    Builtin(builtin::BuiltinValue::kPosition), Id(1200_u));

    auto& attributes = var->attributes;
    EXPECT_TRUE(ast::HasAttribute<LocationAttribute>(attributes));
    EXPECT_TRUE(ast::HasAttribute<BuiltinAttribute>(attributes));
    EXPECT_TRUE(ast::HasAttribute<IdAttribute>(attributes));

    auto* location = GetAttribute<LocationAttribute>(attributes);
    ASSERT_NE(nullptr, location);
    ASSERT_NE(nullptr, location->expr);
    EXPECT_TRUE(location->expr->Is<IntLiteralExpression>());
}

TEST_F(VariableTest, HasBindingPoint_BothProvided) {
    auto* var = Var("my_var", ty.i32(), builtin::AddressSpace::kFunction, Binding(2_a), Group(1_a));
    EXPECT_TRUE(var->HasBindingPoint());
}

TEST_F(VariableTest, HasBindingPoint_NeitherProvided) {
    auto* var = Var("my_var", ty.i32(), builtin::AddressSpace::kFunction, utils::Empty);
    EXPECT_FALSE(var->HasBindingPoint());
}

TEST_F(VariableTest, HasBindingPoint_MissingGroupAttribute) {
    auto* var = Var("my_var", ty.i32(), builtin::AddressSpace::kFunction, Binding(2_a));
    EXPECT_FALSE(var->HasBindingPoint());
}

TEST_F(VariableTest, HasBindingPoint_MissingBindingAttribute) {
    auto* var = Var("my_var", ty.i32(), builtin::AddressSpace::kFunction, Group(1_a));
    EXPECT_FALSE(var->HasBindingPoint());
}

}  // namespace
}  // namespace tint::ast
