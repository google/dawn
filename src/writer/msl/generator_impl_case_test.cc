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

#include "gtest/gtest.h"
#include "src/ast/break_statement.h"
#include "src/ast/case_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/sint_literal.h"
#include "src/ast/type/i32_type.h"
#include "src/writer/msl/generator_impl.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = testing::Test;

TEST_F(MslGeneratorImplTest, Emit_Case) {
  ast::type::I32Type i32;

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::BreakStatement>());

  ast::CaseSelectorList lit;
  lit.push_back(std::make_unique<ast::SintLiteral>(&i32, 5));
  ast::CaseStatement c(std::move(lit), std::move(body));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitCase(&c)) << g.error();
  EXPECT_EQ(g.result(), R"(  case 5: {
    break;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_Case_BreaksByDefault) {
  ast::type::I32Type i32;

  ast::CaseSelectorList lit;
  lit.push_back(std::make_unique<ast::SintLiteral>(&i32, 5));
  ast::CaseStatement c(std::move(lit), std::make_unique<ast::BlockStatement>());

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitCase(&c)) << g.error();
  EXPECT_EQ(g.result(), R"(  case 5: {
    break;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_Case_WithFallthrough) {
  ast::type::I32Type i32;

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::FallthroughStatement>());

  ast::CaseSelectorList lit;
  lit.push_back(std::make_unique<ast::SintLiteral>(&i32, 5));
  ast::CaseStatement c(std::move(lit), std::move(body));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitCase(&c)) << g.error();
  EXPECT_EQ(g.result(), R"(  case 5: {
    /* fallthrough */
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_Case_MultipleSelectors) {
  ast::type::I32Type i32;

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::BreakStatement>());

  ast::CaseSelectorList lit;
  lit.push_back(std::make_unique<ast::SintLiteral>(&i32, 5));
  lit.push_back(std::make_unique<ast::SintLiteral>(&i32, 6));
  ast::CaseStatement c(std::move(lit), std::move(body));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitCase(&c)) << g.error();
  EXPECT_EQ(g.result(), R"(  case 5:
  case 6: {
    break;
  }
)");
}

TEST_F(MslGeneratorImplTest, Emit_Case_Default) {
  ast::CaseStatement c;

  auto body = std::make_unique<ast::BlockStatement>();
  body->append(std::make_unique<ast::BreakStatement>());
  c.set_body(std::move(body));

  ast::Module m;
  GeneratorImpl g(&m);
  g.increment_indent();

  ASSERT_TRUE(g.EmitCase(&c)) << g.error();
  EXPECT_EQ(g.result(), R"(  default: {
    break;
  }
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
