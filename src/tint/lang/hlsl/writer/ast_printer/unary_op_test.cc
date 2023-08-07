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

#include "src/tint/lang/hlsl/writer/ast_printer/helper_test.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::hlsl::writer {
namespace {

using HlslUnaryOpTest = TestHelper;

TEST_F(HlslUnaryOpTest, AddressOf) {
    GlobalVar("expr", ty.f32(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kAddressOf, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "expr");
}

TEST_F(HlslUnaryOpTest, Complement) {
    GlobalVar("expr", ty.u32(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kComplement, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "~(expr)");
}

TEST_F(HlslUnaryOpTest, Indirection) {
    GlobalVar("G", ty.f32(), core::AddressSpace::kPrivate);
    auto* p = Let("expr", create<ast::UnaryOpExpression>(ast::UnaryOp::kAddressOf, Expr("G")));
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kIndirection, Expr("expr"));
    WrapInFunction(p, op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "expr");
}

TEST_F(HlslUnaryOpTest, Not) {
    GlobalVar("expr", ty.bool_(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kNot, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "!(expr)");
}

TEST_F(HlslUnaryOpTest, Negation) {
    GlobalVar("expr", ty.i32(), core::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation, Expr("expr"));
    WrapInFunction(op);

    ASTPrinter& gen = Build();

    StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "-(expr)");
}
}  // namespace
}  // namespace tint::hlsl::writer
