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

#include "src/ast/fallthrough_statement.h"
#include "src/writer/hlsl/test_helper.h"

namespace tint {
namespace writer {
namespace hlsl {
namespace {

using HlslGeneratorImplTest_Case = TestHelper;

TEST_F(HlslGeneratorImplTest_Case, Emit_Case) {
  auto* body = Block(create<ast::BreakStatement>());
  ast::CaseSelectorList lit;
  lit.push_back(Literal(5));
  auto* c = create<ast::CaseStatement>(lit, body);
  WrapInFunction(c);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitCase(out, c)) << gen.error();
  EXPECT_EQ(result(), R"(  case 5: {
    break;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Case, Emit_Case_BreaksByDefault) {
  ast::CaseSelectorList lit;
  lit.push_back(Literal(5));
  auto* c = create<ast::CaseStatement>(lit, Block());
  WrapInFunction(c);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitCase(out, c)) << gen.error();
  EXPECT_EQ(result(), R"(  case 5: {
    break;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Case, Emit_Case_WithFallthrough) {
  auto* body = Block(create<ast::FallthroughStatement>());
  ast::CaseSelectorList lit;
  lit.push_back(Literal(5));
  auto* c = create<ast::CaseStatement>(lit, body);
  WrapInFunction(c);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitCase(out, c)) << gen.error();
  EXPECT_EQ(result(), R"(  case 5: {
    /* fallthrough */
  }
)");
}

TEST_F(HlslGeneratorImplTest_Case, Emit_Case_MultipleSelectors) {
  auto* body = Block(create<ast::BreakStatement>());
  ast::CaseSelectorList lit;
  lit.push_back(Literal(5));
  lit.push_back(Literal(6));
  auto* c = create<ast::CaseStatement>(lit, body);
  WrapInFunction(c);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitCase(out, c)) << gen.error();
  EXPECT_EQ(result(), R"(  case 5:
  case 6: {
    break;
  }
)");
}

TEST_F(HlslGeneratorImplTest_Case, Emit_Case_Default) {
  auto* body = Block(create<ast::BreakStatement>());
  auto* c = create<ast::CaseStatement>(ast::CaseSelectorList{}, body);
  WrapInFunction(c);

  GeneratorImpl& gen = Build();

  gen.increment_indent();

  ASSERT_TRUE(gen.EmitCase(out, c)) << gen.error();
  EXPECT_EQ(result(), R"(  default: {
    break;
  }
)");
}

}  // namespace
}  // namespace hlsl
}  // namespace writer
}  // namespace tint
