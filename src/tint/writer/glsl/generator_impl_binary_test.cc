// Copyright 2021 The Tint Authors.
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
#include "src/tint/writer/glsl/test_helper.h"

namespace tint::writer::glsl {
namespace {

using GlslGeneratorImplTest_Binary = TestHelper;

struct BinaryData {
  const char* result;
  ast::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
  out << data.op;
  return out;
}

using GlslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(GlslBinaryTest, Emit_f32) {
  auto params = GetParam();

  // Skip ops that are illegal for this type
  if (params.op == ast::BinaryOp::kAnd || params.op == ast::BinaryOp::kOr ||
      params.op == ast::BinaryOp::kXor ||
      params.op == ast::BinaryOp::kShiftLeft ||
      params.op == ast::BinaryOp::kShiftRight ||
      params.op == ast::BinaryOp::kModulo) {
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
TEST_P(GlslBinaryTest, Emit_u32) {
  auto params = GetParam();

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
TEST_P(GlslBinaryTest, Emit_i32) {
  auto params = GetParam();

  // Skip ops that are illegal for this type
  if (params.op == ast::BinaryOp::kShiftLeft ||
      params.op == ast::BinaryOp::kShiftRight) {
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
    GlslGeneratorImplTest,
    GlslBinaryTest,
    testing::Values(
        BinaryData{"(left & right)", ast::BinaryOp::kAnd},
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
        BinaryData{"(left / right)", ast::BinaryOp::kDivide},
        BinaryData{"(left % right)", ast::BinaryOp::kModulo}));

TEST_F(GlslGeneratorImplTest_Binary, Multiply_VectorScalar) {
  auto* lhs = vec3<f32>(1.f, 1.f, 1.f);
  auto* rhs = Expr(1.f);

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(),
            "(vec3(1.0f, 1.0f, 1.0f) * "
            "1.0f)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_ScalarVector) {
  auto* lhs = Expr(1.f);
  auto* rhs = vec3<f32>(1.f, 1.f, 1.f);

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);

  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(),
            "(1.0f * vec3(1.0f, 1.0f, "
            "1.0f))");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixScalar) {
  Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
  auto* lhs = Expr("mat");
  auto* rhs = Expr(1.f);

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(), "(mat * 1.0f)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_ScalarMatrix) {
  Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
  auto* lhs = Expr(1.f);
  auto* rhs = Expr("mat");

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(), "(1.0f * mat)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixVector) {
  Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
  auto* lhs = Expr("mat");
  auto* rhs = vec3<f32>(1.f, 1.f, 1.f);

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(), "(mat * vec3(1.0f, 1.0f, 1.0f))");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_VectorMatrix) {
  Global("mat", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
  auto* lhs = vec3<f32>(1.f, 1.f, 1.f);
  auto* rhs = Expr("mat");

  auto* expr =
      create<ast::BinaryExpression>(ast::BinaryOp::kMultiply, lhs, rhs);
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(), "(vec3(1.0f, 1.0f, 1.0f) * mat)");
}

TEST_F(GlslGeneratorImplTest_Binary, Multiply_MatrixMatrix) {
  Global("lhs", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);
  Global("rhs", ty.mat3x3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kMultiply,
                                             Expr("lhs"), Expr("rhs"));
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  EXPECT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(), "(lhs * rhs)");
}

TEST_F(GlslGeneratorImplTest_Binary, Logical_And) {
  Global("a", ty.bool_(), ast::StorageClass::kPrivate);
  Global("b", ty.bool_(), ast::StorageClass::kPrivate);

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                             Expr("a"), Expr("b"));
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

TEST_F(GlslGeneratorImplTest_Binary, ModF32) {
  Global("a", ty.f32(), ast::StorageClass::kPrivate);
  Global("b", ty.f32(), ast::StorageClass::kPrivate);

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"),
                                             Expr("b"));
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, ModVec3F32) {
  Global("a", ty.vec3<f32>(), ast::StorageClass::kPrivate);
  Global("b", ty.vec3<f32>(), ast::StorageClass::kPrivate);

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kModulo, Expr("a"),
                                             Expr("b"));
  WrapInFunction(expr);

  GeneratorImpl& gen = Build();

  std::stringstream out;
  ASSERT_TRUE(gen.EmitExpression(out, expr)) << gen.error();
  EXPECT_EQ(out.str(), "tint_float_modulo(a, b)");
}

TEST_F(GlslGeneratorImplTest_Binary, Logical_Multi) {
  // (a && b) || (c || d)
  Global("a", ty.bool_(), ast::StorageClass::kPrivate);
  Global("b", ty.bool_(), ast::StorageClass::kPrivate);
  Global("c", ty.bool_(), ast::StorageClass::kPrivate);
  Global("d", ty.bool_(), ast::StorageClass::kPrivate);

  auto* expr = create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalOr,
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"),
                                    Expr("b")),
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("c"),
                                    Expr("d")));
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

TEST_F(GlslGeneratorImplTest_Binary, Logical_Or) {
  Global("a", ty.bool_(), ast::StorageClass::kPrivate);
  Global("b", ty.bool_(), ast::StorageClass::kPrivate);

  auto* expr = create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                             Expr("a"), Expr("b"));
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

TEST_F(GlslGeneratorImplTest_Binary, If_WithLogical) {
  // if (a && b) {
  //   return 1;
  // } else if (b || c) {
  //   return 2;
  // } else {
  //   return 3;
  // }

  Global("a", ty.bool_(), ast::StorageClass::kPrivate);
  Global("b", ty.bool_(), ast::StorageClass::kPrivate);
  Global("c", ty.bool_(), ast::StorageClass::kPrivate);

  auto* expr = If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                Expr("a"), Expr("b")),
                  Block(Return(1)),
                  If(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                                   Expr("b"), Expr("c")),
                     Block(Return(2)), Block(Return(3))));
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

TEST_F(GlslGeneratorImplTest_Binary, Return_WithLogical) {
  // return (a && b) || c;

  Global("a", ty.bool_(), ast::StorageClass::kPrivate);
  Global("b", ty.bool_(), ast::StorageClass::kPrivate);
  Global("c", ty.bool_(), ast::StorageClass::kPrivate);

  auto* expr = Return(create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalOr,
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, Expr("a"),
                                    Expr("b")),
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

TEST_F(GlslGeneratorImplTest_Binary, Assign_WithLogical) {
  // a = (b || c) && d;

  Global("a", ty.bool_(), ast::StorageClass::kPrivate);
  Global("b", ty.bool_(), ast::StorageClass::kPrivate);
  Global("c", ty.bool_(), ast::StorageClass::kPrivate);
  Global("d", ty.bool_(), ast::StorageClass::kPrivate);

  auto* expr = Assign(
      Expr("a"), create<ast::BinaryExpression>(
                     ast::BinaryOp::kLogicalAnd,
                     create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                                   Expr("b"), Expr("c")),
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

TEST_F(GlslGeneratorImplTest_Binary, Decl_WithLogical) {
  // var a : bool = (b && c) || d;

  Global("b", ty.bool_(), ast::StorageClass::kPrivate);
  Global("c", ty.bool_(), ast::StorageClass::kPrivate);
  Global("d", ty.bool_(), ast::StorageClass::kPrivate);

  auto* var = Var("a", ty.bool_(), ast::StorageClass::kNone,
                  create<ast::BinaryExpression>(
                      ast::BinaryOp::kLogicalOr,
                      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                    Expr("b"), Expr("c")),
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

TEST_F(GlslGeneratorImplTest_Binary, Call_WithLogical) {
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
  params.push_back(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                                 Expr("a"), Expr("b")));
  params.push_back(create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                                 Expr("c"), Expr("d")));
  params.push_back(create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalAnd,
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("a"),
                                    Expr("c")),
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, Expr("b"),
                                    Expr("d"))));

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

}  // namespace
}  // namespace tint::writer::glsl
