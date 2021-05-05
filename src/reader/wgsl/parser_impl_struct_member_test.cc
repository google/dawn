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

TEST_F(ParserImplTest, StructMember_Parses) {
  auto p = parser("a : i32;");

  auto& builder = p->builder();

  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  EXPECT_EQ(decos.value.size(), 0u);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_NE(m.value, nullptr);

  EXPECT_EQ(m->symbol(), builder.Symbols().Get("a"));
  EXPECT_TRUE(m->type()->Is<ast::I32>());
  EXPECT_EQ(m->decorations().size(), 0u);

  EXPECT_EQ(m->source().range, (Source::Range{{1u, 1u}, {1u, 2u}}));
  EXPECT_EQ(m->type()->source().range, (Source::Range{{1u, 5u}, {1u, 8u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithOffsetDecoration_DEPRECATED) {
  auto p = parser("[[offset(2)]] a : i32;");

  auto& builder = p->builder();

  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  EXPECT_EQ(decos.value.size(), 1u);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_NE(m.value, nullptr);

  EXPECT_EQ(m->symbol(), builder.Symbols().Get("a"));
  EXPECT_TRUE(m->type()->Is<ast::I32>());
  EXPECT_EQ(m->decorations().size(), 1u);
  EXPECT_TRUE(m->decorations()[0]->Is<ast::StructMemberOffsetDecoration>());
  EXPECT_EQ(
      m->decorations()[0]->As<ast::StructMemberOffsetDecoration>()->offset(),
      2u);

  EXPECT_EQ(m->source().range, (Source::Range{{1u, 15u}, {1u, 16u}}));
  EXPECT_EQ(m->type()->source().range, (Source::Range{{1u, 19u}, {1u, 22u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithAlignDecoration) {
  auto p = parser("[[align(2)]] a : i32;");

  auto& builder = p->builder();

  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  EXPECT_EQ(decos.value.size(), 1u);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_NE(m.value, nullptr);

  EXPECT_EQ(m->symbol(), builder.Symbols().Get("a"));
  EXPECT_TRUE(m->type()->Is<ast::I32>());
  EXPECT_EQ(m->decorations().size(), 1u);
  EXPECT_TRUE(m->decorations()[0]->Is<ast::StructMemberAlignDecoration>());
  EXPECT_EQ(
      m->decorations()[0]->As<ast::StructMemberAlignDecoration>()->align(), 2u);

  EXPECT_EQ(m->source().range, (Source::Range{{1u, 14u}, {1u, 15u}}));
  EXPECT_EQ(m->type()->source().range, (Source::Range{{1u, 18u}, {1u, 21u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithSizeDecoration) {
  auto p = parser("[[size(2)]] a : i32;");

  auto& builder = p->builder();

  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  EXPECT_EQ(decos.value.size(), 1u);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_NE(m.value, nullptr);

  EXPECT_EQ(m->symbol(), builder.Symbols().Get("a"));
  EXPECT_TRUE(m->type()->Is<ast::I32>());
  EXPECT_EQ(m->decorations().size(), 1u);
  EXPECT_TRUE(m->decorations()[0]->Is<ast::StructMemberSizeDecoration>());
  EXPECT_EQ(m->decorations()[0]->As<ast::StructMemberSizeDecoration>()->size(),
            2u);

  EXPECT_EQ(m->source().range, (Source::Range{{1u, 13u}, {1u, 14u}}));
  EXPECT_EQ(m->type()->source().range, (Source::Range{{1u, 17u}, {1u, 20u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithDecoration) {
  auto p = parser("[[size(2)]] a : i32;");

  auto& builder = p->builder();

  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  EXPECT_EQ(decos.value.size(), 1u);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_NE(m.value, nullptr);

  EXPECT_EQ(m->symbol(), builder.Symbols().Get("a"));
  EXPECT_TRUE(m->type()->Is<ast::I32>());
  EXPECT_EQ(m->decorations().size(), 1u);
  EXPECT_TRUE(m->decorations()[0]->Is<ast::StructMemberSizeDecoration>());
  EXPECT_EQ(m->decorations()[0]->As<ast::StructMemberSizeDecoration>()->size(),
            2u);

  EXPECT_EQ(m->source().range, (Source::Range{{1u, 13u}, {1u, 14u}}));
  EXPECT_EQ(m->type()->source().range, (Source::Range{{1u, 17u}, {1u, 20u}}));
}

TEST_F(ParserImplTest, StructMember_ParsesWithMultipleDecorations) {
  auto p = parser(R"([[size(2)]]
[[align(4)]] a : i32;)");

  auto& builder = p->builder();

  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);
  EXPECT_EQ(decos.value.size(), 2u);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(m.errored);
  ASSERT_NE(m.value, nullptr);

  EXPECT_EQ(m->symbol(), builder.Symbols().Get("a"));
  EXPECT_TRUE(m->type()->Is<ast::I32>());
  EXPECT_EQ(m->decorations().size(), 2u);
  EXPECT_TRUE(m->decorations()[0]->Is<ast::StructMemberSizeDecoration>());
  EXPECT_EQ(m->decorations()[0]->As<ast::StructMemberSizeDecoration>()->size(),
            2u);
  EXPECT_TRUE(m->decorations()[1]->Is<ast::StructMemberAlignDecoration>());
  EXPECT_EQ(
      m->decorations()[1]->As<ast::StructMemberAlignDecoration>()->align(), 4u);

  EXPECT_EQ(m->source().range, (Source::Range{{2u, 14u}, {2u, 15u}}));
  EXPECT_EQ(m->type()->source().range, (Source::Range{{2u, 18u}, {2u, 21u}}));
}

TEST_F(ParserImplTest, StructMember_InvalidDecoration) {
  auto p = parser("[[size(nan)]] a : i32;");
  auto decos = p->decoration_list();
  EXPECT_TRUE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_FALSE(m.errored);
  ASSERT_NE(m.value, nullptr);

  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(),
            "1:8: expected signed integer literal for size decoration");
}

TEST_F(ParserImplTest, StructMember_InvalidVariable) {
  auto p = parser("[[size(4)]] a : B;");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_TRUE(decos.matched);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(m.errored);
  ASSERT_EQ(m.value, nullptr);
  EXPECT_EQ(p->error(), "1:17: unknown constructed type 'B'");
}

TEST_F(ParserImplTest, StructMember_MissingSemicolon) {
  auto p = parser("a : i32");
  auto decos = p->decoration_list();
  EXPECT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);

  auto m = p->expect_struct_member(decos.value);
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(m.errored);
  ASSERT_EQ(m.value, nullptr);
  EXPECT_EQ(p->error(), "1:8: expected ';' for struct member");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
