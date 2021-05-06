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

TEST_F(ParserImplTest, FunctionTypeDecl_Void) {
  auto p = parser("void");

  auto e = p->function_type_decl();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_TRUE(e.value->Is<ast::Void>());
  EXPECT_EQ(e.value->source().range, (Source::Range{{1u, 1u}, {1u, 5u}}));
}

TEST_F(ParserImplTest, FunctionTypeDecl_Type) {
  auto p = parser("vec2<f32>");

  auto e = p->function_type_decl();
  EXPECT_TRUE(e.matched);
  EXPECT_FALSE(e.errored);
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_TRUE(e.value->Is<ast::Vector>());
  EXPECT_EQ(e.value->As<ast::Vector>()->size(), 2u);
  EXPECT_TRUE(e.value->As<ast::Vector>()->type()->Is<ast::F32>());
  EXPECT_EQ(e.value->source().range, (Source::Range{{1u, 1u}, {1u, 10u}}));
}

TEST_F(ParserImplTest, FunctionTypeDecl_InvalidType) {
  auto p = parser("vec2<invalid>");
  auto e = p->function_type_decl();
  EXPECT_FALSE(e.matched);
  EXPECT_TRUE(e.errored);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(e.value, nullptr);
  EXPECT_EQ(p->error(), "1:6: unknown constructed type 'invalid'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
