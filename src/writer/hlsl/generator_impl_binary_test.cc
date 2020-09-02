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

#include "src/ast/binary_expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
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

using HlslBinaryTest = TestHelperBase<testing::TestWithParam<BinaryData>>;
TEST_P(HlslBinaryTest, Emit) {
  auto params = GetParam();

  auto left = std::make_unique<ast::IdentifierExpression>("left");
  auto right = std::make_unique<ast::IdentifierExpression>("right");

  ast::BinaryExpression expr(params.op, std::move(left), std::move(right));

  ASSERT_TRUE(gen().EmitExpression(pre(), out(), &expr)) << gen().error();
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
  auto left = std::make_unique<ast::IdentifierExpression>("left");
  auto right = std::make_unique<ast::IdentifierExpression>("right");

  ast::BinaryExpression expr(ast::BinaryOp::kLogicalAnd, std::move(left),
                             std::move(right));

  ASSERT_TRUE(gen().EmitExpression(pre(), out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "(_tint_tmp)");
  EXPECT_EQ(pre_result(), R"(bool _tint_tmp = false;
if (left) {
  if (right) {
    _tint_tmp = true;
  }
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_Multi) {
  // (a && b) || (c || d)
  auto a = std::make_unique<ast::IdentifierExpression>("a");
  auto b = std::make_unique<ast::IdentifierExpression>("b");
  auto c = std::make_unique<ast::IdentifierExpression>("c");
  auto d = std::make_unique<ast::IdentifierExpression>("d");

  ast::BinaryExpression expr(
      ast::BinaryOp::kLogicalOr,
      std::make_unique<ast::BinaryExpression>(ast::BinaryOp::kLogicalAnd,
                                              std::move(a), std::move(b)),
      std::make_unique<ast::BinaryExpression>(ast::BinaryOp::kLogicalOr,
                                              std::move(c), std::move(d)));

  ASSERT_TRUE(gen().EmitExpression(pre(), out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "(_tint_tmp_0)");
  EXPECT_EQ(pre_result(), R"(bool _tint_tmp = false;
if (a) {
  if (b) {
    _tint_tmp = true;
  }
}
bool _tint_tmp_0 = false;
if ((_tint_tmp)) {
  _tint_tmp_0 = true;
} else {
  bool _tint_tmp_1 = false;
  if (c) {
    _tint_tmp_1 = true;
  } else {
    if (d) {
      _tint_tmp_1 = true;
    }
  }
  if ((_tint_tmp_1)) {
    _tint_tmp_0 = true;
  }
}
)");
}

TEST_F(HlslGeneratorImplTest_Binary, Logical_Or) {
  auto left = std::make_unique<ast::IdentifierExpression>("left");
  auto right = std::make_unique<ast::IdentifierExpression>("right");

  ast::BinaryExpression expr(ast::BinaryOp::kLogicalOr, std::move(left),
                             std::move(right));

  ASSERT_TRUE(gen().EmitExpression(pre(), out(), &expr)) << gen().error();
  EXPECT_EQ(result(), "(_tint_tmp)");
  EXPECT_EQ(pre_result(), R"(bool _tint_tmp = false;
if (left) {
  _tint_tmp = true;
} else {
  if (right) {
    _tint_tmp = true;
  }
}
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
