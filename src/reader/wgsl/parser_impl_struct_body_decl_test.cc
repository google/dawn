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
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {

using ParserImplTest = testing::Test;

TEST_F(ParserImplTest, StructBodyDecl_Parses) {
  auto i32 =
      TypeManager::Instance()->Get(std::make_unique<ast::type::I32Type>());

  ParserImpl p{"{a : i32;}"};
  auto m = p.struct_body_decl();
  ASSERT_FALSE(p.has_error());
  ASSERT_EQ(m.size(), 1);

  const auto& mem = m[0];
  EXPECT_EQ(mem->name(), "a");
  EXPECT_EQ(mem->type(), i32);
  EXPECT_EQ(mem->decorations().size(), 0);

  TypeManager::Destroy();
}

TEST_F(ParserImplTest, StructBodyDecl_ParsesEmpty) {
  ParserImpl p{"{}"};
  auto m = p.struct_body_decl();
  ASSERT_FALSE(p.has_error());
  ASSERT_EQ(m.size(), 0);
}

TEST_F(ParserImplTest, StructBodyDecl_InvalidMember) {
  ParserImpl p{R"(
{
  [[offset nan]] a : i32;
})"};
  auto m = p.struct_body_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "3:12: invalid value for offset decoration");
}

TEST_F(ParserImplTest, StructBodyDecl_MissingClosingBracket) {
  ParserImpl p{"{a : i32;"};
  auto m = p.struct_body_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "1:10: missing } for struct declaration");
}

TEST_F(ParserImplTest, StructBodyDecl_InvalidToken) {
  ParserImpl p{R"(
{
  a : i32;
  1.23
} )"};
  auto m = p.struct_body_decl();
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "4:3: invalid identifier declaration");
}

}  // namespace wgsl
}  // namespace reader
}  // namespace tint
