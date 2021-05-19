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

#include "src/ast/workgroup_decoration.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionDecl) {
  auto p = parser("fn main(a : i32, b : f32) { return; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<ast::Void>());

  ASSERT_EQ(f->params().size(), 2u);
  EXPECT_EQ(f->params()[0]->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_EQ(f->params()[1]->symbol(), p->builder().Symbols().Get("b"));

  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<ast::Void>());

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList) {
  auto p = parser("[[workgroup_size(2, 3, 4)]] fn main() { return; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  ASSERT_TRUE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<ast::Void>());
  ASSERT_EQ(f->params().size(), 0u);

  auto& decorations = f->decorations();
  ASSERT_EQ(decorations.size(), 1u);
  ASSERT_TRUE(decorations[0]->Is<ast::WorkgroupDecoration>());

  auto values = decorations[0]->As<ast::WorkgroupDecoration>()->values();

  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(y_scalar, nullptr);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 3u);

  ASSERT_NE(values[2], nullptr);
  auto* z_scalar = values[2]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(z_scalar, nullptr);
  ASSERT_TRUE(z_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(z_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList_MultipleEntries) {
  auto p = parser(R"(
[[workgroup_size(2, 3, 4), stage(compute)]]
fn main() { return; })");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  ASSERT_TRUE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<ast::Void>());
  ASSERT_EQ(f->params().size(), 0u);

  auto& decorations = f->decorations();
  ASSERT_EQ(decorations.size(), 2u);

  ASSERT_TRUE(decorations[0]->Is<ast::WorkgroupDecoration>());
  auto values = decorations[0]->As<ast::WorkgroupDecoration>()->values();

  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(y_scalar, nullptr);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 3u);

  ASSERT_NE(values[2], nullptr);
  auto* z_scalar = values[2]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(z_scalar, nullptr);
  ASSERT_TRUE(z_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(z_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  ASSERT_TRUE(decorations[1]->Is<ast::StageDecoration>());
  EXPECT_EQ(decorations[1]->As<ast::StageDecoration>()->value(),
            ast::PipelineStage::kCompute);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_DecorationList_MultipleLists) {
  auto p = parser(R"(
[[workgroup_size(2, 3, 4)]]
[[stage(compute)]]
fn main() { return; })");
  auto decorations = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decorations.errored);
  ASSERT_TRUE(decorations.matched);
  auto f = p->function_decl(decorations.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<ast::Void>());
  ASSERT_EQ(f->params().size(), 0u);

  auto& decos = f->decorations();
  ASSERT_EQ(decos.size(), 2u);

  ASSERT_TRUE(decos[0]->Is<ast::WorkgroupDecoration>());
  auto values = decos[0]->As<ast::WorkgroupDecoration>()->values();

  ASSERT_NE(values[0], nullptr);
  auto* x_scalar = values[0]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(x_scalar, nullptr);
  ASSERT_TRUE(x_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(x_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 2u);

  ASSERT_NE(values[1], nullptr);
  auto* y_scalar = values[1]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(y_scalar, nullptr);
  ASSERT_TRUE(y_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(y_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 3u);

  ASSERT_NE(values[2], nullptr);
  auto* z_scalar = values[2]->As<ast::ScalarConstructorExpression>();
  ASSERT_NE(z_scalar, nullptr);
  ASSERT_TRUE(z_scalar->literal()->Is<ast::IntLiteral>());
  EXPECT_EQ(z_scalar->literal()->As<ast::IntLiteral>()->value_as_u32(), 4u);

  ASSERT_TRUE(decos[1]->Is<ast::StageDecoration>());
  EXPECT_EQ(decos[1]->As<ast::StageDecoration>()->value(),
            ast::PipelineStage::kCompute);

  auto* body = f->body();
  ASSERT_EQ(body->size(), 1u);
  EXPECT_TRUE(body->get(0)->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_ReturnTypeDecorationList) {
  auto p = parser("fn main() -> [[location(1)]] f32 { return 1.0; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol(), p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type(), nullptr);
  EXPECT_TRUE(f->return_type()->Is<ast::F32>());
  ASSERT_EQ(f->params().size(), 0u);

  auto& decorations = f->decorations();
  EXPECT_EQ(decorations.size(), 0u);

  auto& ret_type_decorations = f->return_type_decorations();
  ASSERT_EQ(ret_type_decorations.size(), 1u);
  auto* loc = ret_type_decorations[0]->As<ast::LocationDecoration>();
  ASSERT_TRUE(loc != nullptr);
  EXPECT_EQ(loc->value(), 1u);

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
  auto p = parser("fn main() { return }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_TRUE(f.errored);
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(f.value, nullptr);
  EXPECT_EQ(p->error(), "1:20: expected ';' for return statement");
}

TEST_F(ParserImplTest, FunctionDecl_MissingLeftBrace) {
  auto p = parser("fn main() return; }");
  auto decos = p->decoration_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(decos.errored);
  EXPECT_FALSE(decos.matched);
  auto f = p->function_decl(decos.value);
  EXPECT_TRUE(f.errored);
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(f.value, nullptr);
  EXPECT_EQ(p->error(), "1:11: expected '{'");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
