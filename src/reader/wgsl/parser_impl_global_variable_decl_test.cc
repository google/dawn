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
#include "src/ast/decorated_variable.h"
#include "src/ast/variable_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, GlobalVariableDecl_WithoutConstructor) {
  auto* p = parser("var<out> a : f32");
  auto e = p->global_variable_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  EXPECT_EQ(e->name(), "a");
  EXPECT_TRUE(e->type()->IsF32());
  EXPECT_EQ(e->storage_class(), ast::StorageClass::kOutput);

  ASSERT_EQ(e->constructor(), nullptr);
  ASSERT_FALSE(e->IsDecorated());
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithConstructor) {
  auto* p = parser("var<out> a : f32 = 1.");
  auto e = p->global_variable_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  EXPECT_EQ(e->name(), "a");
  EXPECT_TRUE(e->type()->IsF32());
  EXPECT_EQ(e->storage_class(), ast::StorageClass::kOutput);

  ASSERT_NE(e->constructor(), nullptr);
  ASSERT_TRUE(e->constructor()->IsConstructor());
  ASSERT_TRUE(e->constructor()->AsConstructor()->IsScalarConstructor());

  ASSERT_FALSE(e->IsDecorated());
}

TEST_F(ParserImplTest, GlobalVariableDecl_WithDecoration) {
  auto* p = parser("[[binding 2, set 1]] var<out> a : f32");
  auto e = p->global_variable_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);
  ASSERT_TRUE(e->IsDecorated());

  EXPECT_EQ(e->name(), "a");
  ASSERT_NE(e->type(), nullptr);
  EXPECT_TRUE(e->type()->IsF32());
  EXPECT_EQ(e->storage_class(), ast::StorageClass::kOutput);

  ASSERT_EQ(e->constructor(), nullptr);

  ASSERT_TRUE(e->IsDecorated());
  auto* v = e->AsDecorated();

  auto& decos = v->decorations();
  ASSERT_EQ(decos.size(), 2u);
  ASSERT_TRUE(decos[0]->IsBinding());
  ASSERT_TRUE(decos[1]->IsSet());
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidDecoration) {
  auto* p = parser("[[binding]] var<out> a : f32");
  auto e = p->global_variable_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:10: invalid value for binding decoration");
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidConstExpr) {
  auto* p = parser("var<out> a : f32 = if (a) {}");
  auto e = p->global_variable_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:20: unable to parse const literal");
}

TEST_F(ParserImplTest, GlobalVariableDecl_InvalidVariableDecl) {
  auto* p = parser("var<invalid> a : f32;");
  auto e = p->global_variable_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:5: invalid storage class for variable decoration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
