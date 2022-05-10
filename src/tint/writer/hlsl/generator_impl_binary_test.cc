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
    out << data.op;
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

    Global("left", ty.f32(), ast::StorageClass::kPrivate);
    Global("right", ty.f32(), ast::StorageClass::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), params.result);
}
TEST_P(HlslBinaryTest, Emit_u32) {
    auto params = GetParam();

    if ((params.valid_for & BinaryData::Types::Integer) == 0) {
        return;
    }

    Global("left", ty.u32(), ast::StorageClass::kPrivate);
    Global("right", ty.u32(), ast::StorageClass::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
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

    Global("left", ty.i32(), ast::StorageClass::kPrivate);
    Global("right", ty.i32(), ast::StorageClass::kPrivate);

    auto* left = Expr("left");
    auto* right = Expr("right");

    auto* expr = create<ast::BinaryExpression>(params.op, left, right);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
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

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorScalar) {
    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(),
              "(float3(1.0f, 1.0f, 1.0f) * "
              "1.0f)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarVector) {
    auto* lhs = Expr(1_f);
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(),
              "(1.0f * float3(1.0f, 1.0f, "
              "1.0f))");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixScalar) {
    Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = Expr(1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "(mat * 1.0f)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarMatrix) {
    Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
    auto* lhs = Expr(1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "(1.0f * mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixVector) {
    Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
    auto* lhs = Expr("mat");
    auto* rhs = vec3<f32>(1_f, 1_f, 1_f);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "mul(float3(1.0f, 1.0f, 1.0f), mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorMatrix) {
    Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
    auto* lhs = vec3<f32>(1_f, 1_f, 1_f);
    auto* rhs = Expr("mat");

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "mul(mat, float3(1.0f, 1.0f, 1.0f))");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixMatrix) {
    Global("lhs", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
    Global("rhs", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, Expr("lhs"), Expr("rhs"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "mul(rhs, lhs)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_And) {
    Global("a", ty.bool_(), ast::StorageClass::kPrivate);
    Global("b", ty.bool_(), ast::StorageClass::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
    EXPECT_EQ(out.str(), "(tint_tmp)");
    EXPECT_EQ(gen.result(), R"(bool tint_tmp = a;
if (tint_tmp) {
  tint_tmp = b;
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_Multi) {
    // (a && b) || (c || d)
    Global("a", ty.bool_(), ast::StorageClass::kPrivate);
    Global("b", ty.bool_(), ast::StorageClass::kPrivate);
    Global("c", ty.bool_(), ast::StorageClass::kPrivate);
    Global("d", ty.bool_(), ast::StorageClass::kPrivate);

    auto* expr = create<ast::BinaryExpression>(
        ast::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("c"), Expr("d")));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
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
    Global("a", ty.bool_(), ast::StorageClass::kPrivate);
    Global("b", ty.bool_(), ast::StorageClass::kPrivate);

    auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("b"));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    std::stringstream out;
    ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
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

    Global("a", ty.bool_(), ast::StorageClass::kPrivate);
    Global("b", ty.bool_(), ast::StorageClass::kPrivate);
    Global("c", ty.bool_(), ast::StorageClass::kPrivate);

    auto* expr =
        If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
           Block(Return(1_i)),
           Else(If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Block(Return(2_i)), Else(Block(Return(3_i))))));
    Func("func", {}, ty.i32(), {WrapInStatement(expr)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.error();
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

    Global("a", ty.bool_(), ast::StorageClass::kPrivate);
    Global("b", ty.bool_(), ast::StorageClass::kPrivate);
    Global("c", ty.bool_(), ast::StorageClass::kPrivate);

    auto* expr = Return(create<ast::BinaryExpression>(
        ast::BinaryOp::kLogicalOr,
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")),
        Expr("c")));
    Func("func", {}, ty.bool_(), {WrapInStatement(expr)});

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.error();
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

    Global("a", ty.bool_(), ast::StorageClass::kPrivate);
    Global("b", ty.bool_(), ast::StorageClass::kPrivate);
    Global("c", ty.bool_(), ast::StorageClass::kPrivate);
    Global("d", ty.bool_(), ast::StorageClass::kPrivate);

    auto* expr =
        Assign(Expr("a"),
               create<ast::BinaryExpression>(
                   ast::BinaryOp::kLogicalAnd,
                   create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("c")),
                   Expr("d")));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.error();
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

    Global("b", ty.bool_(), ast::StorageClass::kPrivate);
    Global("c", ty.bool_(), ast::StorageClass::kPrivate);
    Global("d", ty.bool_(), ast::StorageClass::kPrivate);

    auto* var =
        Var("a", ty.bool_(), ast::StorageClass::kNone,
            create<ast::BinaryExpression>(
                ast::BinaryOp::kLogicalOr,
                create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("b"), Expr("c")),
                Expr("d")));

    auto* decl = Decl(var);
    WrapInFunction(decl);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(decl)) << gen.error();
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
         {
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
             Param(Sym(), ty.bool_()),
         },
         ty.void_(), ast::StatementList{}, ast::AttributeList{});
    Global("a", ty.bool_(), ast::StorageClass::kPrivate);
    Global("b", ty.bool_(), ast::StorageClass::kPrivate);
    Global("c", ty.bool_(), ast::StorageClass::kPrivate);
    Global("d", ty.bool_(), ast::StorageClass::kPrivate);

    ast::ExpressionList params;
    params.push_back(
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"), Expr("b")));
    params.push_back(
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("c"), Expr("d")));
    params.push_back(create<ast::BinaryExpression>(
        ast::BinaryOp::kLogicalAnd,
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"), Expr("c")),
        create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"), Expr("d"))));

    auto* expr = CallStmt(Call("foo", params));
    WrapInFunction(expr);

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.EmitStatement(expr)) << gen.error();
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

namespace HlslGeneratorDivMod {

struct Params {
    enum class Type { Div, Mod };
    Type type;
};

struct HlslGeneratorDivModTest : TestParamHelper<Params> {
    std::string Token() { return GetParam().type == Params::Type::Div ? "/" : "%"; }

    template <typename... Args>
    auto Op(Args... args) {
        return GetParam().type == Params::Type::Div ? Div(std::forward<Args>(args)...)
                                                    : Mod(std::forward<Args>(args)...);
    }
};

INSTANTIATE_TEST_SUITE_P(HlslGeneratorImplTest,
                         HlslGeneratorDivModTest,
                         testing::Values(Params{Params::Type::Div}, Params{Params::Type::Mod}));

TEST_P(HlslGeneratorDivModTest, DivOrModByLiteralZero_i32) {
    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", ty.i32())),
             Decl(Let("r", nullptr, Op("a", 0_i))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn() {
  int a = 0;
  const int r = (a )" + Token() +
                                R"( 1);
}
)");
}

TEST_P(HlslGeneratorDivModTest, DivOrModByLiteralZero_u32) {
    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", ty.u32())),
             Decl(Let("r", nullptr, Op("a", 0_u))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn() {
  uint a = 0u;
  const uint r = (a )" + Token() +
                                R"( 1u);
}
)");
}  // namespace HlslGeneratorDivMod

TEST_P(HlslGeneratorDivModTest, DivOrModByLiteralZero_vec_by_vec_i32) {
    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", nullptr, vec4<i32>(100_i, 100_i, 100_i, 100_i))),
             Decl(Let("r", nullptr, Op("a", vec4<i32>(50_i, 0_i, 25_i, 0_i)))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn() {
  int4 a = int4(100, 100, 100, 100);
  const int4 r = (a )" + Token() +
                                R"( int4(50, 1, 25, 1));
}
)");
}  // namespace

TEST_P(HlslGeneratorDivModTest, DivOrModByLiteralZero_vec_by_scalar_i32) {
    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", nullptr, vec4<i32>(100_i, 100_i, 100_i, 100_i))),
             Decl(Let("r", nullptr, Op("a", 0_i))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn() {
  int4 a = int4(100, 100, 100, 100);
  const int4 r = (a )" + Token() +
                                R"( 1);
}
)");
}  // namespace hlsl

TEST_P(HlslGeneratorDivModTest, DivOrModByIdentifier_i32) {
    Func("fn", {Param("b", ty.i32())}, ty.void_(),
         {
             Decl(Var("a", ty.i32())),
             Decl(Let("r", nullptr, Op("a", "b"))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn(int b) {
  int a = 0;
  const int r = (a )" + Token() +
                                R"( (b == 0 ? 1 : b));
}
)");
}  // namespace writer

TEST_P(HlslGeneratorDivModTest, DivOrModByIdentifier_u32) {
    Func("fn", {Param("b", ty.u32())}, ty.void_(),
         {
             Decl(Var("a", ty.u32())),
             Decl(Let("r", nullptr, Op("a", "b"))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn(uint b) {
  uint a = 0u;
  const uint r = (a )" + Token() +
                                R"( (b == 0u ? 1u : b));
}
)");
}  // namespace tint

TEST_P(HlslGeneratorDivModTest, DivOrModByIdentifier_vec_by_vec_i32) {
    Func("fn", {Param("b", ty.vec3<i32>())}, ty.void_(),
         {
             Decl(Var("a", ty.vec3<i32>())),
             Decl(Let("r", nullptr, Op("a", "b"))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn(int3 b) {
  int3 a = int3(0, 0, 0);
  const int3 r = (a )" + Token() +
                                R"( (b == int3(0, 0, 0) ? int3(1, 1, 1) : b));
}
)");
}

TEST_P(HlslGeneratorDivModTest, DivOrModByIdentifier_vec_by_scalar_i32) {
    Func("fn", {Param("b", ty.i32())}, ty.void_(),
         {
             Decl(Var("a", ty.vec3<i32>())),
             Decl(Let("r", nullptr, Op("a", "b"))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(void fn(int b) {
  int3 a = int3(0, 0, 0);
  const int3 r = (a )" + Token() +
                                R"( (b == 0 ? 1 : b));
}
)");
}

TEST_P(HlslGeneratorDivModTest, DivOrModByExpression_i32) {
    Func("zero", {}, ty.i32(),
         {
             Return(Expr(0_i)),
         });

    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", ty.i32())),
             Decl(Let("r", nullptr, Op("a", Call("zero")))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(int value_or_one_if_zero_int(int value) {
  return value == 0 ? 1 : value;
}

int zero() {
  return 0;
}

void fn() {
  int a = 0;
  const int r = (a )" + Token() +
                                R"( value_or_one_if_zero_int(zero()));
}
)");
}

TEST_P(HlslGeneratorDivModTest, DivOrModByExpression_u32) {
    Func("zero", {}, ty.u32(),
         {
             Return(Expr(0_u)),
         });

    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", ty.u32())),
             Decl(Let("r", nullptr, Op("a", Call("zero")))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(uint value_or_one_if_zero_uint(uint value) {
  return value == 0u ? 1u : value;
}

uint zero() {
  return 0u;
}

void fn() {
  uint a = 0u;
  const uint r = (a )" + Token() +
                                R"( value_or_one_if_zero_uint(zero()));
}
)");
}

TEST_P(HlslGeneratorDivModTest, DivOrModByExpression_vec_by_vec_i32) {
    Func("zero", {}, ty.vec3<i32>(),
         {
             Return(vec3<i32>(0_i, 0_i, 0_i)),
         });

    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", ty.vec3<i32>())),
             Decl(Let("r", nullptr, Op("a", Call("zero")))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(int3 value_or_one_if_zero_int3(int3 value) {
  return value == int3(0, 0, 0) ? int3(1, 1, 1) : value;
}

int3 zero() {
  return int3(0, 0, 0);
}

void fn() {
  int3 a = int3(0, 0, 0);
  const int3 r = (a )" + Token() +
                                R"( value_or_one_if_zero_int3(zero()));
}
)");
}

TEST_P(HlslGeneratorDivModTest, DivOrModByExpression_vec_by_scalar_i32) {
    Func("zero", {}, ty.i32(),
         {
             Return(0_i),
         });

    Func("fn", {}, ty.void_(),
         {
             Decl(Var("a", ty.vec3<i32>())),
             Decl(Let("r", nullptr, Op("a", Call("zero")))),
         });

    GeneratorImpl& gen = Build();

    ASSERT_TRUE(gen.Generate());
    EXPECT_EQ(gen.result(), R"(int value_or_one_if_zero_int(int value) {
  return value == 0 ? 1 : value;
}

int zero() {
  return 0;
}

void fn() {
  int3 a = int3(0, 0, 0);
  const int3 r = (a )" + Token() +
                                R"( value_or_one_if_zero_int(zero()));
}
)");
}
}  // namespace HlslGeneratorDivMod

}  // namespace
}  // namespace tint::writer::hlsl
