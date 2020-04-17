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
#include "src/ast/variable.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, VariableDecl_Parses) {
  auto* p = parser("var my_var : f32");
  auto var = p->variable_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(var, nullptr);
  ASSERT_EQ(var->name(), "my_var");
  ASSERT_NE(var->type(), nullptr);
  ASSERT_EQ(var->source().line, 1u);
  ASSERT_EQ(var->source().column, 1u);
  ASSERT_TRUE(var->type()->IsF32());
}

TEST_F(ParserImplTest, VariableDecl_MissingVar) {
  auto* p = parser("my_var : f32");
  auto v = p->variable_decl();
  ASSERT_EQ(v, nullptr);
  ASSERT_FALSE(p->has_error());

  auto t = p->next();
  ASSERT_TRUE(t.IsIdentifier());
}

TEST_F(ParserImplTest, VariableDecl_InvalidIdentDecl) {
  auto* p = parser("var my_var f32");
  auto v = p->variable_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(v, nullptr);
  ASSERT_EQ(p->error(), "1:12: missing : for identifier declaration");
}

TEST_F(ParserImplTest, VariableDecl_WithStorageClass) {
  auto* p = parser("var<private> my_var : f32");
  auto v = p->variable_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(v, nullptr);
  EXPECT_EQ(v->name(), "my_var");
  EXPECT_TRUE(v->type()->IsF32());
  EXPECT_EQ(v->storage_class(), ast::StorageClass::kPrivate);
}

TEST_F(ParserImplTest, VariableDecl_InvalidStorageClass) {
  auto* p = parser("var<unknown> my_var : f32");
  auto v = p->variable_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(v, nullptr);
  EXPECT_EQ(p->error(), "1:5: invalid storage class for variable decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
