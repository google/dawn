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
#include "src/ast/function.h"
#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type/type.h"
#include "src/type/void_type.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionDecl) {
  auto p = parser("fn main(a : i32, b : f32) -> void { return; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->get_program().Symbols().Register("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());

  ASSERT_EQ(f->params().size(), 2u);
  EXPECT_EQ(f->params()[0]->symbol(), p->get_program().Symbols().Register("a"));
  EXPECT_EQ(f->params()[1]->symbol(), p->get_program().Symbols().Register("b"));

  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList) {
  auto p = parser("[[workgroup_size(2, 3, 4)]] fn main() -> void { return; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  ASSERT_TRUE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->get_program().Symbols().Register("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());
  ASSERT_EQ(f->params().size(), 0u);
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());

  auto& decorations = f->decorations();
  ASSERT_EQ(decorations.size(), 1u);
  ASSERT_TRUE(decorations[0]->Is<ast::WorkgroupDecoration>());

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  std::tie(x, y, z) = decorations[0]->As<ast::WorkgroupDecoration>()->values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 3u);
  EXPECT_EQ(z, 4u);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList_MultipleEntries) {
  auto p = parser(R"(
[[workgroup_size(2, 3, 4), workgroup_size(5, 6, 7)]]
fn main() -> void { return; })");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  ASSERT_TRUE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->get_program().Symbols().Register("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());
  ASSERT_EQ(f->params().size(), 0u);
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());

  auto& decorations = f->decorations();
  ASSERT_EQ(decorations.size(), 2u);

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  ASSERT_TRUE(decorations[0]->Is<ast::WorkgroupDecoration>());
  std::tie(x, y, z) = decorations[0]->As<ast::WorkgroupDecoration>()->values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 3u);
  EXPECT_EQ(z, 4u);

  ASSERT_TRUE(decorations[1]->Is<ast::WorkgroupDecoration>());
  std::tie(x, y, z) = decorations[1]->As<ast::WorkgroupDecoration>()->values();
  EXPECT_EQ(x, 5u);
  EXPECT_EQ(y, 6u);
  EXPECT_EQ(z, 7u);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList_MultipleLists) {
  auto p = parser(R"(
[[workgroup_size(2, 3, 4)]]
[[workgroup_size(5, 6, 7)]]
fn main() -> void { return; })");
  auto decorations = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decorations.errored);
  ASSERT_TRUE(decorations.matched);
  auto f = p->function_decl(decorations.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->get_program().Symbols().Register("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());
  ASSERT_EQ(f->params().size(), 0u);
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<type::Void>());

  auto& decos = f->decorations();
  ASSERT_EQ(decos.size(), 2u);

  uint32_t x = 0;
  uint32_t y = 0;
  uint32_t z = 0;
  ASSERT_TRUE(decos[0]->Is<ast::WorkgroupDecoration>());
  std::tie(x, y, z) = decos[0]->As<ast::WorkgroupDecoration>()->values();
  EXPECT_EQ(x, 2u);
  EXPECT_EQ(y, 3u);
  EXPECT_EQ(z, 4u);

  ASSERT_TRUE(decos[1]->Is<ast::WorkgroupDecoration>());
  std::tie(x, y, z) = decos[1]->As<ast::WorkgroupDecoration>()->values();
  EXPECT_EQ(x, 5u);
  EXPECT_EQ(y, 6u);
  EXPECT_EQ(z, 7u);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_InvalidHeader) {
  auto p = parser("fn main() -> { }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_TRUE(f.errored);
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(f.value, nullptr);
  EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(ParserImplTest, FunctionDecl_InvalidBody) {
  auto p = parser("fn main() -> void { return }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_TRUE(f.errored);
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(f.value, nullptr);
  EXPECT_EQ(p->error(), "1:28: expected ';' for return statement");
}

TEST_F(ParserImplTest, FunctionDecl_MissingLeftBrace) {
  auto p = parser("fn main() -> void return; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_TRUE(f.errored);
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(f.value, nullptr);
  EXPECT_EQ(p->error(), "1:19: expected '{'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
