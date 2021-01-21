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
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/sint_literal.h"
#include "src/ast/switch_statement.h"
#include "src/type/i32_type.h"
#include "src/writer/msl/generator_impl.h"
#include "src/writer/msl/test_helper.h"

namespace tint {
namespace writer {
namespace msl {
namespace {

using MslGeneratorImplTest = TestHelper;

TEST_F(MslGeneratorImplTest, Emit_Switch) {
  auto* def_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::BreakStatement>(),
  });
  auto* def = create<ast::CaseStatement>(ast::CaseSelectorList{}, def_body);

  ast::CaseSelectorList case_val;
  case_val.push_back(Literal(5));

  auto* case_body = create<ast::BlockStatement>(ast::StatementList{
      create<ast::BreakStatement>(),
  });

  auto* case_stmt = create<ast::CaseStatement>(case_val, case_body);

  ast::CaseStatementList body;
  body.push_back(case_stmt);
  body.push_back(def);

  auto* s = create<ast::SwitchStatement>(Expr("cond"), body);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitStatement(s)) << gen.error();
  EXPECT_EQ(gen.result(), R"(  switch(cond) {
    case 5: {
      break;
    }
    default: {
      break;
    }
  }
)");
}

}  // namespace
}  // namespace msl
}  // namespace writer
}  // namespace tint
