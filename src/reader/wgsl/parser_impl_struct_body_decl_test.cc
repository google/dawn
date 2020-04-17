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
#include "src/ast/type/i32_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructBodyDecl_Parses) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser("{a : i32;}");
  auto m = p->struct_body_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_EQ(m.size(), 1u);

  const auto& mem = m[0];
  EXPECT_EQ(mem->name(), "a");
  EXPECT_EQ(mem->type(), i32);
  EXPECT_EQ(mem->decorations().size(), 0u);
}

TEST_F(ParserImplTest, StructBodyDecl_ParsesEmpty) {
  auto* p = parser("{}");
  auto m = p->struct_body_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_EQ(m.size(), 0u);
}

TEST_F(ParserImplTest, StructBodyDecl_InvalidMember) {
  auto* p = parser(R"(
{
  [[offset nan]] a : i32;
})");
  auto m = p->struct_body_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "3:12: invalid value for offset decoration");
}

TEST_F(ParserImplTest, StructBodyDecl_MissingClosingBracket) {
  auto* p = parser("{a : i32;");
  auto m = p->struct_body_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:10: missing } for struct declaration");
}

TEST_F(ParserImplTest, StructBodyDecl_InvalidToken) {
  auto* p = parser(R"(
{
  a : i32;
  1.23
} )");
  auto m = p->struct_body_decl();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "4:3: invalid identifier declaration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
