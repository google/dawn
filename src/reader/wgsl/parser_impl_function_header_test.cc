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

#include "gtest/gtest.h"
#include "src/ast/function.h"
#include "src/ast/type/type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionHeader) {
  auto* p = parser("fn main(a : i32, b: f32) -> void");
  auto f = p->function_header();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(f, nullptr);

  EXPECT_EQ(f->name(), "main");
  ASSERT_EQ(f->params().size(), 2u);
  EXPECT_EQ(f->params()[0]->name(), "a");
  EXPECT_EQ(f->params()[1]->name(), "b");
  EXPECT_TRUE(f->return_type()->IsVoid());
}

TEST_F(ParserImplTest, FunctionHeader_MissingIdent) {
  auto* p = parser("fn () ->");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:4: missing identifier for function");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidIdent) {
  auto* p = parser("fn 133main() -> i32");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:4: missing identifier for function");
}

TEST_F(ParserImplTest, FunctionHeader_MissingParenLeft) {
  auto* p = parser("fn main) -> i32");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing ( for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidParamList) {
  auto* p = parser("fn main(a :i32,) -> i32");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:15: found , but no variable declaration");
}

TEST_F(ParserImplTest, FunctionHeader_MissingParenRight) {
  auto* p = parser("fn main( -> i32");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:10: missing ) for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_MissingArrow) {
  auto* p = parser("fn main() i32");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:11: missing -> for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidReturnType) {
  auto* p = parser("fn main() -> invalid");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:14: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, FunctionHeader_MissingReturnType) {
  auto* p = parser("fn main() ->");
  auto f = p->function_header();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(f, nullptr);
  EXPECT_EQ(p->error(), "1:13: unable to determine function return type");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
