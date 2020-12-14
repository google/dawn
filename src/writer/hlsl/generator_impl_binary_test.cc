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

#include <memory>

#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/call_expression.h"
#include "src/ast/call_statement.h"
#include "src/ast/else_statement.h"
#include "src/ast/float_literal.h"
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/matrix_type.h"
#include "src/ast/type/u32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/type/void_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Binary = TestHelper;

struct BinaryData {
  const char* result;
  ast::BinaryOp op;
};
inline std::ostream& operator<<(std::ostream& out, BinaryData data) {
  out << data.op;
  return out;
}

using HlslBinaryTest = TestParamHelper<BinaryData>;
TEST_P(HlslBinaryTest, Emit_f32) {
  ast::type::F32 f32;

  auto params = GetParam();

  auto* left_var =
      create<ast::Variable>(Source{},                        // source
                            "left",                          // name
                            ast::StorageClass::kFunction,    // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* right_var =
      create<ast::Variable>(Source{},                        // source
                            "right",                         // name
                            ast::StorageClass::kFunction,    // storage_class
                            &f32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* left = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("left"), "left");
  auto* right = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("right"), "right");

  td.RegisterVariableForTesting(left_var);
  td.RegisterVariableForTesting(right_var);

  ast::BinaryExpression expr(Source{}, params.op, left, right);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), params.result);
}
TEST_P(HlslBinaryTest, Emit_u32) {
  ast::type::U32 u32;

  auto params = GetParam();

  auto* left_var =
      create<ast::Variable>(Source{},                        // source
                            "left",                          // name
                            ast::StorageClass::kFunction,    // storage_class
                            &u32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* right_var =
      create<ast::Variable>(Source{},                        // source
                            "right",                         // name
                            ast::StorageClass::kFunction,    // storage_class
                            &u32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* left = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("left"), "left");
  auto* right = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("right"), "right");

  td.RegisterVariableForTesting(left_var);
  td.RegisterVariableForTesting(right_var);

  ast::BinaryExpression expr(Source{}, params.op, left, right);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), params.result);
}
TEST_P(HlslBinaryTest, Emit_i32) {
  ast::type::I32 i32;

  auto params = GetParam();

  auto* left_var =
      create<ast::Variable>(Source{},                        // source
                            "left",                          // name
                            ast::StorageClass::kFunction,    // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* right_var =
      create<ast::Variable>(Source{},                        // source
                            "right",                         // name
                            ast::StorageClass::kFunction,    // storage_class
                            &i32,                            // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  auto* left = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("left"), "left");
  auto* right = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("right"), "right");

  td.RegisterVariableForTesting(left_var);
  td.RegisterVariableForTesting(right_var);

  ast::BinaryExpression expr(Source{}, params.op, left, right);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), params.result);
}
INSTANTIATE_TEST_SUITE_P(
    HlslGeneratorImplTest,
    HlslBinaryTest,
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

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorScalar) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* lhs = create<ast::TypeConstructorExpression>(
      Source{}, &vec3,
      ast::ExpressionList{
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)),
          create<ast::ScalarConstructorExpression>(
              Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)),
      });

  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f));

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kMultiply, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(),
            "(float3(1.0f, 1.0f, 1.0f) * "
            "1.0f)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarVector) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);

  auto* lhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f));

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  auto* rhs = create<ast::TypeConstructorExpression>(Source{}, &vec3, vals);

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kMultiply, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(),
            "(1.0f * float3(1.0f, 1.0f, "
            "1.0f))");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixScalar) {
  ast::type::F32 f32;
  ast::type::Matrix mat3(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "mat",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("mat"), "mat");
  auto* rhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f));

  td.RegisterVariableForTesting(var);

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kMultiply, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "(mat * 1.0f)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_ScalarMatrix) {
  ast::type::F32 f32;
  ast::type::Matrix mat3(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "mat",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* lhs = create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f));
  auto* rhs = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("mat"), "mat");

  td.RegisterVariableForTesting(var);

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kMultiply, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "(1.0f * mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixVector) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Matrix mat3(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "mat",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("mat"), "mat");

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  auto* rhs = create<ast::TypeConstructorExpression>(Source{}, &vec3, vals);

  td.RegisterVariableForTesting(var);

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kMultiply, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "mul(mat, float3(1.0f, 1.0f, 1.0f))");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_VectorMatrix) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Matrix mat3(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "mat",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::ExpressionList vals;
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  vals.push_back(create<ast::ScalarConstructorExpression>(
      Source{}, create<ast::FloatLiteral>(Source{}, &f32, 1.f)));
  auto* lhs = create<ast::TypeConstructorExpression>(Source{}, &vec3, vals);

  auto* rhs = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("mat"), "mat");

  td.RegisterVariableForTesting(var);

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kMultiply, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "mul(float3(1.0f, 1.0f, 1.0f), mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Multiply_MatrixMatrix) {
  ast::type::F32 f32;
  ast::type::Vector vec3(&f32, 3);
  ast::type::Matrix mat3(&f32, 3, 3);

  auto* var =
      create<ast::Variable>(Source{},                        // source
                            "mat",                           // name
                            ast::StorageClass::kFunction,    // storage_class
                            &mat3,                           // type
                            false,                           // is_const
                            nullptr,                         // constructor
                            ast::VariableDecorationList{});  // decorations
  auto* lhs = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("mat"), "mat");
  auto* rhs = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("mat"), "mat");

  td.RegisterVariableForTesting(var);

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kMultiply, lhs, rhs);

  ASSERT_TRUE(td.DetermineResultType(&expr)) << td.error();
  EXPECT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "mul(mat, mat)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_And) {
  auto* left = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("left"), "left");
  auto* right = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("right"), "right");

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kLogicalAnd, left, right);

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "(_tint_tmp)");
  EXPECT_EQ(pre_result(), R"(bool _tint_tmp = left;
if (_tint_tmp) {
  _tint_tmp = right;
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_Multi) {
  // (a && b) || (c || d)
  auto* a =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"), "a");
  auto* b =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"), "b");
  auto* c =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"), "c");
  auto* d =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("d"), "d");

  ast::BinaryExpression expr(
      Source{}, ast::BinaryOp::kLogicalOr,
      create<ast::BinaryExpression>(Source{}, ast::BinaryOp::kLogicalAnd, a, b),
      create<ast::BinaryExpression>(Source{}, ast::BinaryOp::kLogicalOr, c, d));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "(_tint_tmp_0)");
  EXPECT_EQ(pre_result(), R"(bool _tint_tmp = a;
if (_tint_tmp) {
  _tint_tmp = b;
}
bool _tint_tmp_0 = (_tint_tmp);
if (!_tint_tmp_0) {
  bool _tint_tmp_1 = c;
  if (!_tint_tmp_1) {
    _tint_tmp_1 = d;
  }
  _tint_tmp_0 = (_tint_tmp_1);
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_Or) {
  auto* left = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("left"), "left");
  auto* right = create<ast::IdentifierExpression>(
      Source{}, mod.RegisterSymbol("right"), "right");

  ast::BinaryExpression expr(Source{}, ast::BinaryOp::kLogicalOr, left, right);

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(result(), "(_tint_tmp)");
  EXPECT_EQ(pre_result(), R"(bool _tint_tmp = left;
if (!_tint_tmp) {
  _tint_tmp = right;
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, If_WithLogical) {
  // if (a && b) {
  //   return 1;
  // } else if (b || c) {
  //   return 2;
  // } else {
  //   return 3;
  // }

  ast::type::I32 i32;

  auto* body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::ReturnStatement>(
                        Source{}, create<ast::ScalarConstructorExpression>(
                                      Source{}, create<ast::SintLiteral>(
                                                    Source{}, &i32, 3))),
                });
  auto* else_stmt = create<ast::ElseStatement>(Source{}, nullptr, body);

  body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::ReturnStatement>(
                        Source{}, create<ast::ScalarConstructorExpression>(
                                      Source{}, create<ast::SintLiteral>(
                                                    Source{}, &i32, 2))),
                });
  auto* else_if_stmt = create<ast::ElseStatement>(
      Source{},
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kLogicalOr,
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"),
                                            "c")),
      body);

  body = create<ast::BlockStatement>(
      Source{}, ast::StatementList{
                    create<ast::ReturnStatement>(
                        Source{}, create<ast::ScalarConstructorExpression>(
                                      Source{}, create<ast::SintLiteral>(
                                                    Source{}, &i32, 1))),
                });

  ast::IfStatement expr(Source{},
                        create<ast::BinaryExpression>(
                            Source{}, ast::BinaryOp::kLogicalAnd,
                            create<ast::IdentifierExpression>(
                                Source{}, mod.RegisterSymbol("a"), "a"),
                            create<ast::IdentifierExpression>(
                                Source{}, mod.RegisterSymbol("b"), "b")),
                        body,
                        {
                            else_if_stmt,
                            else_stmt,
                        });

  ASSERT_TRUE(gen.EmitStatement(out, &expr)) << gen.error();
  EXPECT_EQ(result(), R"(bool _tint_tmp = a;
if (_tint_tmp) {
  _tint_tmp = b;
}
if ((_tint_tmp)) {
  return 1;
} else {
  bool _tint_tmp_0 = b;
  if (!_tint_tmp_0) {
    _tint_tmp_0 = c;
  }
  if ((_tint_tmp_0)) {
    return 2;
  } else {
    return 3;
  }
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Return_WithLogical) {
  // return (a && b) || c;
  auto* a =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"), "a");
  auto* b =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"), "b");
  auto* c =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"), "c");

  ast::ReturnStatement expr(Source{},
                            create<ast::BinaryExpression>(
                                Source{}, ast::BinaryOp::kLogicalOr,
                                create<ast::BinaryExpression>(
                                    Source{}, ast::BinaryOp::kLogicalAnd, a, b),
                                c));

  ASSERT_TRUE(gen.EmitStatement(out, &expr)) << gen.error();
  EXPECT_EQ(result(), R"(bool _tint_tmp = a;
if (_tint_tmp) {
  _tint_tmp = b;
}
bool _tint_tmp_0 = (_tint_tmp);
if (!_tint_tmp_0) {
  _tint_tmp_0 = c;
}
return (_tint_tmp_0);
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Assign_WithLogical) {
  // a = (b || c) && d;
  auto* a =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"), "a");
  auto* b =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"), "b");
  auto* c =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"), "c");
  auto* d =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("d"), "d");

  ast::AssignmentStatement expr(
      Source{}, a,
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kLogicalAnd,
          create<ast::BinaryExpression>(Source{}, ast::BinaryOp::kLogicalOr, b,
                                        c),
          d));

  ASSERT_TRUE(gen.EmitStatement(out, &expr)) << gen.error();
  EXPECT_EQ(result(), R"(bool _tint_tmp = b;
if (!_tint_tmp) {
  _tint_tmp = c;
}
bool _tint_tmp_0 = (_tint_tmp);
if (_tint_tmp_0) {
  _tint_tmp_0 = d;
}
a = (_tint_tmp_0);
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Decl_WithLogical) {
  // var a : bool = (b && c) || d;
  ast::type::Bool bool_type;

  auto* b =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"), "b");
  auto* c =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"), "c");
  auto* d =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("d"), "d");

  auto* var =
      create<ast::Variable>(Source{},                      // source
                            "a",                           // name
                            ast::StorageClass::kFunction,  // storage_class
                            &bool_type,                    // type
                            false,                         // is_const
                            create<ast::BinaryExpression>(
                                Source{}, ast::BinaryOp::kLogicalOr,
                                create<ast::BinaryExpression>(
                                    Source{}, ast::BinaryOp::kLogicalAnd, b, c),
                                d),                          // constructor
                            ast::VariableDecorationList{});  // decorations

  ast::VariableDeclStatement expr(Source{}, var);

  ASSERT_TRUE(gen.EmitStatement(out, &expr)) << gen.error();
  EXPECT_EQ(result(), R"(bool _tint_tmp = b;
if (_tint_tmp) {
  _tint_tmp = c;
}
bool _tint_tmp_0 = (_tint_tmp);
if (!_tint_tmp_0) {
  _tint_tmp_0 = d;
}
bool a = (_tint_tmp_0);
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Bitcast_WithLogical) {
  // as<i32>(a && (b || c))
  ast::type::I32 i32;

  auto* a =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"), "a");
  auto* b =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"), "b");
  auto* c =
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"), "c");

  ast::BitcastExpression expr(
      Source{}, &i32,
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kLogicalAnd, a,
          create<ast::BinaryExpression>(Source{}, ast::BinaryOp::kLogicalOr, b,
                                        c)));

  ASSERT_TRUE(gen.EmitExpression(pre, out, &expr)) << gen.error();
  EXPECT_EQ(pre_result(), R"(bool _tint_tmp = a;
if (_tint_tmp) {
  bool _tint_tmp_0 = b;
  if (!_tint_tmp_0) {
    _tint_tmp_0 = c;
  }
  _tint_tmp = (_tint_tmp_0);
}
)");
  EXPECT_EQ(result(), R"(asint((_tint_tmp)))");
}

TEST_F(HlslGeneratorImplTest_Binary, Call_WithLogical) {
  // foo(a && b, c || d, (a || c) && (b || d))

  ast::type::Void void_type;

  auto* func = create<ast::Function>(
      Source{}, mod.RegisterSymbol("foo"), "foo", ast::VariableList{},
      &void_type, create<ast::BlockStatement>(Source{}, ast::StatementList{}),
      ast::FunctionDecorationList{});
  mod.AddFunction(func);

  ast::ExpressionList params;
  params.push_back(create<ast::BinaryExpression>(
      Source{}, ast::BinaryOp::kLogicalAnd,
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"), "a"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                        "b")));
  params.push_back(create<ast::BinaryExpression>(
      Source{}, ast::BinaryOp::kLogicalOr,
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"), "c"),
      create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("d"),
                                        "d")));
  params.push_back(create<ast::BinaryExpression>(
      Source{}, ast::BinaryOp::kLogicalAnd,
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kLogicalOr,
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("a"),
                                            "a"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("c"),
                                            "c")),
      create<ast::BinaryExpression>(
          Source{}, ast::BinaryOp::kLogicalOr,
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("b"),
                                            "b"),
          create<ast::IdentifierExpression>(Source{}, mod.RegisterSymbol("d"),
                                            "d"))));

  ast::CallStatement expr(Source{},
                          create<ast::CallExpression>(
                              Source{},
                              create<ast::IdentifierExpression>(
                                  Source{}, mod.RegisterSymbol("foo"), "foo"),
                              params));

  ASSERT_TRUE(gen.EmitStatement(out, &expr)) << gen.error();
  EXPECT_EQ(result(), R"(bool _tint_tmp = a;
if (_tint_tmp) {
  _tint_tmp = b;
}
bool _tint_tmp_0 = c;
if (!_tint_tmp_0) {
  _tint_tmp_0 = d;
}
bool _tint_tmp_1 = a;
if (!_tint_tmp_1) {
  _tint_tmp_1 = c;
}
bool _tint_tmp_2 = (_tint_tmp_1);
if (_tint_tmp_2) {
  bool _tint_tmp_3 = b;
  if (!_tint_tmp_3) {
    _tint_tmp_3 = d;
  }
  _tint_tmp_2 = (_tint_tmp_3);
}
foo((_tint_tmp), (_tint_tmp_0), (_tint_tmp_2));
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
