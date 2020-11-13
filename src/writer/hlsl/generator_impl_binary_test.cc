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
#include "src/ast/function.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/if_statement.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/bool_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/void_type.h"
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
TEST_P(HlslBinaryTest, Emit) {
  auto params = GetParam();

  auto left = create<ast::IdentifierExpression>("left");
  auto right = create<ast::IdentifierExpression>("right");

  ast::BinaryExpression expr(params.op, std::move(left), std::move(right));

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

TEST_F(HlslGeneratorImplTest_Binary, Logical_And) {
  auto left = create<ast::IdentifierExpression>("left");
  auto right = create<ast::IdentifierExpression>("right");

  ast::BinaryExpression expr(ast::BinaryOp::kLogicalAnd, std::move(left),
                             std::move(right));

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
  auto a = create<ast::IdentifierExpression>("a");
  auto b = create<ast::IdentifierExpression>("b");
  auto c = create<ast::IdentifierExpression>("c");
  auto d = create<ast::IdentifierExpression>("d");

  ast::BinaryExpression expr(
      ast::BinaryOp::kLogicalOr,
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, std::move(a),
                                    std::move(b)),
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, std::move(c),
                                    std::move(d)));

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
  auto left = create<ast::IdentifierExpression>("left");
  auto right = create<ast::IdentifierExpression>("right");

  ast::BinaryExpression expr(ast::BinaryOp::kLogicalOr, std::move(left),
                             std::move(right));

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

  ast::type::I32Type i32;

  auto body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 3))));
  auto else_stmt = create<ast::ElseStatement>(std::move(body));

  body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 2))));
  auto else_if_stmt = create<ast::ElseStatement>(
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                    create<ast::IdentifierExpression>("b"),
                                    create<ast::IdentifierExpression>("c")),
      std::move(body));

  ast::ElseStatementList else_stmts;
  else_stmts.push_back(std::move(else_if_stmt));
  else_stmts.push_back(std::move(else_stmt));

  body = create<ast::BlockStatement>();
  body->append(
      create<ast::ReturnStatement>(create<ast::ScalarConstructorExpression>(
          create<ast::SintLiteral>(&i32, 1))));

  ast::IfStatement expr(
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                    create<ast::IdentifierExpression>("a"),
                                    create<ast::IdentifierExpression>("b")),
      std::move(body));
  expr.set_else_statements(std::move(else_stmts));

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
  auto a = create<ast::IdentifierExpression>("a");
  auto b = create<ast::IdentifierExpression>("b");
  auto c = create<ast::IdentifierExpression>("c");

  ast::ReturnStatement expr(create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalOr,
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, std::move(a),
                                    std::move(b)),
      std::move(c)));

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
  auto a = create<ast::IdentifierExpression>("a");
  auto b = create<ast::IdentifierExpression>("b");
  auto c = create<ast::IdentifierExpression>("c");
  auto d = create<ast::IdentifierExpression>("d");

  ast::AssignmentStatement expr(
      std::move(a),
      create<ast::BinaryExpression>(
          ast::BinaryOp::kLogicalAnd,
          create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr, std::move(b),
                                        std::move(c)),
          std::move(d)));

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
  ast::type::BoolType bool_type;

  auto b = create<ast::IdentifierExpression>("b");
  auto c = create<ast::IdentifierExpression>("c");
  auto d = create<ast::IdentifierExpression>("d");

  auto var =
      create<ast::Variable>("a", ast::StorageClass::kFunction, &bool_type);
  var->set_constructor(create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalOr,
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd, std::move(b),
                                    std::move(c)),
      std::move(d)));

  ast::VariableDeclStatement expr(std::move(var));

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
  ast::type::I32Type i32;

  auto a = create<ast::IdentifierExpression>("a");
  auto b = create<ast::IdentifierExpression>("b");
  auto c = create<ast::IdentifierExpression>("c");

  ast::BitcastExpression expr(
      &i32, create<ast::BinaryExpression>(
                ast::BinaryOp::kLogicalAnd, std::move(a),
                create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                              std::move(b), std::move(c))));

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

  ast::type::VoidType void_type;

  auto func = create<ast::Function>("foo", ast::VariableList{}, &void_type);
  mod.AddFunction(std::move(func));

  ast::ExpressionList params;
  params.push_back(create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalAnd, create<ast::IdentifierExpression>("a"),
      create<ast::IdentifierExpression>("b")));
  params.push_back(create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalOr, create<ast::IdentifierExpression>("c"),
      create<ast::IdentifierExpression>("d")));
  params.push_back(create<ast::BinaryExpression>(
      ast::BinaryOp::kLogicalAnd,
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                    create<ast::IdentifierExpression>("a"),
                                    create<ast::IdentifierExpression>("c")),
      create<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                    create<ast::IdentifierExpression>("b"),
                                    create<ast::IdentifierExpression>("d"))));

  ast::CallStatement expr(create<ast::CallExpression>(
      create<ast::IdentifierExpression>("foo"), std::move(params)));

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
