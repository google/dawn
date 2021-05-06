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

TEST_F(ParserImplTest, VariableDecl_Parses) {
  auto p = parser("var my_var : f32");
  auto v = p->variable_decl();
  EXPECT_FALSE(p->has_error());
  EXPECT_TRUE(v.matched);
  EXPECT_FALSE(v.errored);
  EXPECT_EQ(v->name, "my_var");
  EXPECT_NE(v->type, nullptr);
  EXPECT_TRUE(v->type->Is<ast::F32>());

  EXPECT_EQ(v->source.range, (Source::Range{{1u, 5u}, {1u, 11u}}));
  EXPECT_EQ(v->type->source().range, (Source::Range{{1u, 14u}, {1u, 17u}}));
}

TEST_F(ParserImplTest, VariableDecl_MissingVar) {
  auto p = parser("my_var : f32");
  auto v = p->variable_decl();
  EXPECT_FALSE(v.matched);
  EXPECT_FALSE(v.errored);
  EXPECT_FALSE(p->has_error());

  auto t = p->next();
  ASSERT_TRUE(t.IsIdentifier());
}

TEST_F(ParserImplTest, VariableDecl_InvalidIdentDecl) {
  auto p = parser("var my_var f32");
  auto v = p->variable_decl();
  EXPECT_FALSE(v.matched);
  EXPECT_TRUE(v.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:12: expected ':' for variable declaration");
}

TEST_F(ParserImplTest, VariableDecl_WithStorageClass) {
  auto p = parser("var<private> my_var : f32");
  auto v = p->variable_decl();
  EXPECT_TRUE(v.matched);
  EXPECT_FALSE(v.errored);
  EXPECT_FALSE(p->has_error());
  EXPECT_EQ(v->name, "my_var");
  EXPECT_TRUE(v->type->Is<ast::F32>());
  EXPECT_EQ(v->storage_class, ast::StorageClass::kPrivate);

  EXPECT_EQ(v->source.range.begin.line, 1u);
  EXPECT_EQ(v->source.range.begin.column, 14u);
  EXPECT_EQ(v->source.range.end.line, 1u);
  EXPECT_EQ(v->source.range.end.column, 20u);
}

TEST_F(ParserImplTest, VariableDecl_InvalidStorageClass) {
  auto p = parser("var<unknown> my_var : f32");
  auto v = p->variable_decl();
  EXPECT_FALSE(v.matched);
  EXPECT_TRUE(v.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:5: invalid storage class for variable decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
