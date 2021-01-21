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
#include "src/type/i32_type.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructBodyDecl_Parses) {
  auto p = parser("{a : i32;}");

  auto& mod = p->get_module();
  auto* i32 = mod.create<type::I32>();

  auto m = p->expect_struct_body_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_EQ(m.value.size(), 1u);

  const auto* mem = m.value[0];
  EXPECT_EQ(mem->symbol(), p->get_module().RegisterSymbol("a"));
  EXPECT_EQ(mem->type(), i32);
  EXPECT_EQ(mem->decorations().size(), 0u);
}

TEST_F(ParserImplTest, StructBodyDecl_ParsesEmpty) {
  auto p = parser("{}");
  auto m = p->expect_struct_body_decl();
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_EQ(m.value.size(), 0u);
}

TEST_F(ParserImplTest, StructBodyDecl_InvalidMember) {
  auto p = parser(R"(
{
  [[offset(nan)]] a : i32;
})");
  auto m = p->expect_struct_body_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(m.errored);
  EXPECT_EQ(p->error(),
            "3:12: expected signed integer literal for offset decoration");
}

TEST_F(ParserImplTest, StructBodyDecl_MissingClosingBracket) {
  auto p = parser("{a : i32;");
  auto m = p->expect_struct_body_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(m.errored);
  EXPECT_EQ(p->error(), "1:10: expected '}' for struct declaration");
}

TEST_F(ParserImplTest, StructBodyDecl_InvalidToken) {
  auto p = parser(R"(
{
  a : i32;
  1.23
} )");
  auto m = p->expect_struct_body_decl();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(m.errored);
  EXPECT_EQ(p->error(), "4:3: expected identifier for struct member");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
