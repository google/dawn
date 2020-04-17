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
#include "src/ast/struct_member_offset_decoration.h"
#include "src/ast/type/i32_type.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, StructMember_Parses) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser("a : i32;");
  auto m = p->struct_member();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(m, nullptr);

  EXPECT_EQ(m->name(), "a");
  EXPECT_EQ(m->type(), i32);
  EXPECT_EQ(m->decorations().size(), 0u);
}

TEST_F(ParserImplTest, StructMember_ParsesWithDecoration) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser("[[offset 2]] a : i32;");
  auto m = p->struct_member();
  ASSERT_FALSE(p->has_error());
  ASSERT_NE(m, nullptr);

  EXPECT_EQ(m->name(), "a");
  EXPECT_EQ(m->type(), i32);
  EXPECT_EQ(m->decorations().size(), 1u);
  EXPECT_TRUE(m->decorations()[0]->IsOffset());
  EXPECT_EQ(m->decorations()[0]->AsOffset()->offset(), 2u);
}

TEST_F(ParserImplTest, StructMember_InvalidDecoration) {
  auto* p = parser("[[offset nan]] a : i32;");
  auto m = p->struct_member();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(m, nullptr);
  EXPECT_EQ(p->error(), "1:10: invalid value for offset decoration");
}

TEST_F(ParserImplTest, StructMember_InvalidVariable) {
  auto* p = parser("[[offset 4]] a : B;");
  auto m = p->struct_member();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(m, nullptr);
  EXPECT_EQ(p->error(), "1:18: unknown type alias 'B'");
}

TEST_F(ParserImplTest, StructMember_MissingSemicolon) {
  auto* p = parser("a : i32");
  auto m = p->struct_member();
  ASSERT_TRUE(p->has_error());
  ASSERT_EQ(m, nullptr);
  EXPECT_EQ(p->error(), "1:8: missing ; for struct member");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
