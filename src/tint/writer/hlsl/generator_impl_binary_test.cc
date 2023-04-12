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
#include "src/tint/ast/variable_decl_statement.h"
#include "src/tint/utils/string_stream.h"
#include "src/tint/writer/hlsl/test_helper.h"

using namespace tint::number_suffixes;  // NOLINT

namespace tint::writer::hlsl {
namespace {

using HlslGeneratorImplTest_Binary = TestHelper;

struct BinaryData {
    const char* result;
    ast::BinaryOp op;

    enum Types { All = 0b11, Integer = 0b10, Float = 0b01 };
    Types valid_for = Types::All;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
    utils::StringStream str;
    str << data.op;
    out << str.str();
    return out;
}

using HlslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(HlslBinaryTest, Emit_f32) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Float) == 0) {
        return;
    }

    // Skip ops that are illegal for this type
    if (params.op == ast::BinaryOp::kAnd || params.op == ast::BinaryOp::kOr ||
        params.op == ast::BinaryOp::kXor || params.op == ast::BinaryOp::kShiftLeft ||
        params.op == ast::BinaryOp::kShiftRight) {
        return;
    }

    GlobalVar("left", ty.f32(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.f32(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(HlslBinaryTest, Emit_f16) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Float) == 0) {
        return;
    }

    // Skip ops that are illegal for this type
    if (params.op == ast::BinaryOp::kAnd || params.op == ast::BinaryOp::kOr ||
        params.op == ast::BinaryOp::kXor || params.op == ast::BinaryOp::kShiftLeft ||
        params.op == ast::BinaryOp::kShiftRight) {
        return;
    }

    Enable(builtin::Extension::kF16);

    GlobalVar("left", ty.f16(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.f16(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(HlslBinaryTest, Emit_u32) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Integer) == 0) {
        return;
    }

    GlobalVar("left", ty.u32(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.u32(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(HlslBinaryTest, Emit_i32) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Integer) == 0) {
        return;
    }

    // Skip ops that are illegal for this type
    if (params.op == ast::BinaryOp::kShiftLeft || params.op == ast::BinaryOp::kShiftRight) {
        return;
    }

    GlobalVar("left", ty.i32(), builtin::AddressSpace::kPrivate);
    GlobalVar("right", ty.i32(), builtin::AddressSpace::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest,
    HlslBinaryTest,
    testing::Values(BinaryData{"(left & right)", ast::BinaryOp::kAnd},
                    BinaryData{"(left | right)", ast::BinaryOp::kOr},
                    BinaryData{"(left ^ right)", ast::BinaryOp::kXor},
                    BinaryData{"(left == right)", ast::BinaryOp::kEqual},
                    BinaryData{"(left != right)", ast::BinaryOp::kNotEqual},
                    BinaryData{"(left < right)", ast::BinaryOp::kLessThan},
                    BinaryData{"(left > right)", ast::BinaryOp::kGreaterThan},
                    BinaryData{"(left <= right)", ast::BinaryOp::kLessThanEqual},
                    BinaryData{"(left >= right)", ast::BinaryOp::kGreaterThanEqual},
                    BinaryData{"(left << right)", ast::BinaryOp::kShiftLeft},
                    BinaryData{"(left >> right)", ast::BinaryOp::kShiftRight},
                    BinaryData{"(left + right)", ast::BinaryOp::kAdd},
                    BinaryData{"(left - right)", ast::BinaryOp::kSubtract},
                    BinaryData{"(left * right)", ast::BinaryOp::kMultiply},
                    // NOTE: Integer divide covered by DivOrModBy* tests below
                    BinaryData{"(left / right)", ast::BinaryOp::kDivide, BinaryData::Types::Float},
                    // NOTE: Integer modulo covered by DivOrModBy* tests below
                    BinaryData{"(left % right)", ast::BinaryOp::kModulo,
                               BinaryData::Types::Float}));

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorScalar_f32) {
    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(1.0f).xxx");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorScalar_f16) {
    Enable(builtin::Extension::kF16);

    auto* lhs = vec3<f16>(1_h, 1_h, 1_h);
    auto* rhs = Expr(1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(float16_t(1.0h)).xxx");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarVector_f32) {
    auto* lhs = Expr(1_f);
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(1.0f).xxx");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarVector_f16) {
    Enable(builtin::Extension::kF16);

    auto* lhs = Expr(1_h);
    auto* rhs = vec3<f16>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(float16_t(1.0h)).xxx");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixScalar_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(mat * 1.0f)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixScalar_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Expr(1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(mat * float16_t(1.0h))");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarMatrix_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr(1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(1.0f * mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarMatrix_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr(1_h);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(float16_t(1.0h) * mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixVector_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul((1.0f).xxx, mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixVector_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = vec3<f16>(1_h, 1_h, 1_h);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul((float16_t(1.0h)).xxx, mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorMatrix_f32) {
    GlobalVar("mat", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(mat, (1.0f).xxx)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorMatrix_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("mat", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    auto* lhs = vec3<f16>(1_h, 1_h, 1_h);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(mat, (float16_t(1.0h)).xxx)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixMatrix_f32) {
    GlobalVar("lhs", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.mat3x3<f32>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(rhs, lhs)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixMatrix_f16) {
    Enable(builtin::Extension::kF16);

    GlobalVar("lhs", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);
    GlobalVar("rhs", ty.mat3x3<f16>(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "mul(rhs, lhs)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_And) {
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_Multi) {
    // (a && b) || (c || d)
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(
        ast::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("c"), Expr("d")));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = a;
if (tint_tmp_1) {
  tint_tmp_1 = b;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  bool tint_tmp_2 = c;
  if (!tint_tmp_2) {
    tint_tmp_2 = d;
  }
  tint_tmp = (tint_tmp_2);
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_Or) {
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    utils::StringStream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.Diagnostics();
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (!tint_tmp) {
  tint_tmp = b;
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, If_WithLogical) {
    // if (a && b) {
    //   return 1i;
    // } else if (b || c) {
    //   return 2i;
    // } else {
    //   return 3i;
    // }

    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr =
        If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
           Block(Return(1_i)),
           Else(If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Block(Return(2_i)), Else(Block(Return(3_i))))));
    Func("func", utils::Empty, ty.i32(), utils::Vector{WrapInStatement(expr)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
if ((tint_tmp)) {
  return 1;
} else {
  bool tint_tmp_1 = b;
  if (!tint_tmp_1) {
    tint_tmp_1 = c;
  }
  if ((tint_tmp_1)) {
    return 2;
  } else {
    return 3;
  }
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Return_WithLogical) {
    // return (a && b) || c;

    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr = Return(create<ast::BinaryExpression>(
        ast::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        Expr("c")));
    Func("func", utils::Empty, ty.bool_(), utils::Vector{WrapInStatement(expr)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = a;
if (tint_tmp_1) {
  tint_tmp_1 = b;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  tint_tmp = c;
}
return (tint_tmp);
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Assign_WithLogical) {
    // a = (b || c) && d;

    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* expr =
        Assign(Expr("a"),
               create<ast::BinaryExpression>(
                   ast::BinaryOp::kLogicalAnd,
                   create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Expr("d")));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = b;
if (!tint_tmp_1) {
  tint_tmp_1 = c;
}
bool tint_tmp = (tint_tmp_1);
if (tint_tmp) {
  tint_tmp = d;
}
a = (tint_tmp);
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Decl_WithLogical) {
    // var a : bool = (b && c) || d;

    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    auto* var =
        Var("a", ty.bool_(), builtin::AddressSpace::kUndefined,
            create<ast::BinaryExpression>(
                ast::BinaryOp::kLogicalOr,
                create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("b"), Expr("c")),
                Expr("d")));

    auto* decl = Decl(var);
    WrapInFunction(decl);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(decl)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(bool tint_tmp_1 = b;
if (tint_tmp_1) {
  tint_tmp_1 = c;
}
bool tint_tmp = (tint_tmp_1);
if (!tint_tmp) {
  tint_tmp = d;
}
bool a = (tint_tmp);
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Call_WithLogical) {
    // foo(a && b, c || d, (a || c) && (b || d))

    Func("foo",
         utils::Vector{
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
         },
         ty.void_(), utils::Empty, utils::Empty);
    GlobalVar("a", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("b", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("c", ty.bool_(), builtin::AddressSpace::kPrivate);
    GlobalVar("d", ty.bool_(), builtin::AddressSpace::kPrivate);

    utils::Vector params{
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("c"), Expr("d")),
        create<ast::BinaryExpression>(
            ast::BinaryOp::kLogicalAnd,
            create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("c")),
            create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("d"))),
    };

    auto* expr = CallStmt(Call("foo", params));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.Diagnostics();
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
bool tint_tmp_1 = c;
if (!tint_tmp_1) {
  tint_tmp_1 = d;
}
bool tint_tmp_3 = a;
if (!tint_tmp_3) {
  tint_tmp_3 = c;
}
bool tint_tmp_2 = (tint_tmp_3);
if (tint_tmp_2) {
  bool tint_tmp_4 = b;
  if (!tint_tmp_4) {
    tint_tmp_4 = d;
  }
  tint_tmp_2 = (tint_tmp_4);
}
foo((tint_tmp), (tint_tmp_1), (tint_tmp_2));
)");
}

}  // namespace
}  // namespace tint::writer::hlsl
