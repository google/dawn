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
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, VariableIdentDecl_Parses) {
  auto p = parser("my_var : f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(decl.errored);
  ASSERT_EQ(decl->name, "my_var");
  ASSERT_NE(decl->type, nullptr);
  ASSERT_TRUE(decl->type->IsF32());

  ASSERT_EQ(decl->source.range.begin.line, 1u);
  ASSERT_EQ(decl->source.range.begin.column, 1u);
  ASSERT_EQ(decl->source.range.end.line, 1u);
  ASSERT_EQ(decl->source.range.end.column, 7u);
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingIdent) {
  auto p = parser(": f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingColon) {
  auto p = parser("my_var f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:8: expected ':' for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_MissingType) {
  auto p = parser("my_var :");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:9: invalid type for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_InvalidIdent) {
  auto p = parser("123 : f32");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:1: expected identifier for test");
}

TEST_F(ParserImplTest, VariableIdentDecl_InvalidType) {
  auto p = parser("my_var : invalid");
  auto decl = p->expect_variable_ident_decl("test");
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(decl.errored);
  ASSERT_EQ(p->error(), "1:10: unknown constructed type 'invalid'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
