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

TEST_F(ParserImplTest, GlobalConstantDecl) {
  auto* p = parser("const a : f32 = 1.");
  auto e = p->global_constant_decl();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_NE(e, nullptr);

  EXPECT_TRUE(e->is_const());
  EXPECT_EQ(e->name(), "a");
  ASSERT_NE(e->type(), nullptr);
  EXPECT_TRUE(e->type()->IsF32());

  ASSERT_NE(e->constructor(), nullptr);
  EXPECT_TRUE(e->constructor()->IsConstructor());
}

TEST_F(ParserImplTest, GlobalConstantDecl_MissingEqual) {
  auto* p = parser("const a: f32 1.");
  auto e = p->global_constant_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:14: missing = for const declaration");
}

TEST_F(ParserImplTest, GlobalConstantDecl_InvalidVariable) {
  auto* p = parser("const a: invalid = 1.");
  auto e = p->global_constant_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:10: unknown type alias 'invalid'");
}

TEST_F(ParserImplTest, GlobalConstantDecl_InvalidExpression) {
  auto* p = parser("const a: f32 = if (a) {}");
  auto e = p->global_constant_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:16: unable to parse const literal");
}

TEST_F(ParserImplTest, GlobalConstantDecl_MissingExpression) {
  auto* p = parser("const a: f32 =");
  auto e = p->global_constant_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(e, nullptr);
  EXPECT_EQ(p->error(), "1:15: unable to parse const literal");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
