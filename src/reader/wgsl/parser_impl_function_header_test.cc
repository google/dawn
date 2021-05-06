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

#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionHeader) {
  auto p = parser("fn main(a : i32, b: f32)");
  auto f = p->function_header();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(f.matched);
  EXPECT_FALSE(f.errored);

  EXPECT_EQ(f->name, "main");
  ASSERT_EQ(f->params.size(), 2u);
  EXPECT_EQ(f->params[0]->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_EQ(f->params[1]->symbol(), p->builder().Symbols().Get("b"));
  EXPECT_TRUE(f->return_type->Is<ast::Void>());
}

TEST_F(ParserImplTest, FunctionHeader_TrailingComma) {
  auto p = parser("fn main(a :i32,)");
  auto f = p->function_header();
  EXPECT_TRUE(f.matched);
  EXPECT_FALSE(f.errored);

  EXPECT_EQ(f->name, "main");
  ASSERT_EQ(f->params.size(), 1u);
  EXPECT_EQ(f->params[0]->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_TRUE(f->return_type->Is<ast::Void>());
}

TEST_F(ParserImplTest, FunctionHeader_DecoratedReturnType) {
  auto p = parser("fn main() -> [[location(1)]] f32");
  auto f = p->function_header();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(f.matched);
  EXPECT_FALSE(f.errored);

  EXPECT_EQ(f->name, "main");
  EXPECT_EQ(f->params.size(), 0u);
  EXPECT_TRUE(f->return_type->Is<ast::F32>());
  ASSERT_EQ(f->return_type_decorations.size(), 1u);
  auto* loc = f->return_type_decorations[0]->As<ast::LocationDecoration>();
  ASSERT_TRUE(loc != nullptr);
  EXPECT_EQ(loc->value(), 1u);
}

TEST_F(ParserImplTest, FunctionHeader_MissingIdent) {
  auto p = parser("fn ()");
  auto f = p->function_header();
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(f.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:4: expected identifier for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidIdent) {
  auto p = parser("fn 133main() -> i32");
  auto f = p->function_header();
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(f.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:4: expected identifier for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_MissingParenLeft) {
  auto p = parser("fn main) -> i32");
  auto f = p->function_header();
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(f.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: expected '(' for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidParamList) {
  auto p = parser("fn main(a :i32, ,) -> i32");
  auto f = p->function_header();
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(f.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:17: expected ')' for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_MissingParenRight) {
  auto p = parser("fn main( -> i32");
  auto f = p->function_header();
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(f.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: expected ')' for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidReturnType) {
  auto p = parser("fn main() -> invalid");
  auto f = p->function_header();
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(f.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:14: unknown constructed type 'invalid'");
}

TEST_F(ParserImplTest, FunctionHeader_MissingReturnType) {
  auto p = parser("fn main() ->");
  auto f = p->function_header();
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(f.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:13: unable to determine function return type");
}

TEST_F(ParserImplTest, FunctionHeader_ArrowVoid) {
  auto p = parser("fn main() -> void");
  auto f = p->function_header();
  EXPECT_TRUE(f.matched);
  EXPECT_FALSE(f.errored);
  EXPECT_EQ(
      p->builder().Diagnostics().str(),
      R"(test.wgsl:1:14 warning: use of deprecated language feature: omit '-> void' for functions that do not return a value
fn main() -> void
             ^^^^
)");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
