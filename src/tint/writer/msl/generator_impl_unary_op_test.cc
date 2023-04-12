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

#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/msl/test_helper.h"

namespace tint::writer::msl {
namespace {

using MslUnaryOpTest = TestHelper;

TEST_F(MslUnaryOpTest, AddressOf) {
    GlobalVar("expr", ty.f32(), builtin::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kAddressOf, Expr("expr"));
    WrapInFunction(op);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "&(expr)");
}

TEST_F(MslUnaryOpTest, Complement) {
    GlobalVar("expr", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kComplement, Expr("expr"));
    WrapInFunction(op);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "~(expr)");
}

TEST_F(MslUnaryOpTest, Indirection) {
    GlobalVar("G", ty.f32(), builtin::AddressSpace::kPrivate);
    auto* p = Let("expr", create<ast::UnaryOpExpression>(ast::UnaryOp::kAddressOf, Expr("G")));
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kIndirection, Expr("expr"));
    WrapInFunction(p, op);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "*(expr)");
}

TEST_F(MslUnaryOpTest, Not) {
    GlobalVar("expr", ty.bool_(), builtin::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kNot, Expr("expr"));
    WrapInFunction(op);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "!(expr)");
}

TEST_F(MslUnaryOpTest, Negation) {
    GlobalVar("expr", ty.i32(), builtin::AddressSpace::kPrivate);
    auto* op = create<ast::UnaryOpExpression>(ast::UnaryOp::kNegation, Expr("expr"));
    WrapInFunction(op);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "tint_unary_minus(expr)");
}

TEST_F(MslUnaryOpTest, IntMin) {
    auto* op = Expr(i32(std::numeric_limits<int32_t>::min()));
    WrapInFunction(op);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, op)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(-2147483647 - 1)");
}

}  // namespace
}  // namespace tint::writer::msl
