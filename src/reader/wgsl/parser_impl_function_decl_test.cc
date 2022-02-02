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

#include "src/ast/workgroup_attribute.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, FunctionDecl) {
  auto p = parser("fn main(a : i32, b : f32) { return; }");
  auto attrs = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attrs.errored);
  EXPECT_FALSE(attrs.matched);
  auto f = p->function_decl(attrs.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol, p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type, nullptr);
  EXPECT_TRUE(f->return_type->Is<ast::Void>());

  ASSERT_EQ(f->params.size(), 2u);
  EXPECT_EQ(f->params[0]->symbol, p->builder().Symbols().Get("a"));
  EXPECT_EQ(f->params[1]->symbol, p->builder().Symbols().Get("b"));

  ASSERT_NE(f->return_type, nullptr);
  EXPECT_TRUE(f->return_type->Is<ast::Void>());

  auto* body = f->body;
  ASSERT_EQ(body->statements.size(), 1u);
  EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_AttributeList) {
  auto p = parser("@workgroup_size(2, 3, 4) fn main() { return; }");
  auto attrs = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attrs.errored);
  ASSERT_TRUE(attrs.matched);
  auto f = p->function_decl(attrs.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol, p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type, nullptr);
  EXPECT_TRUE(f->return_type->Is<ast::Void>());
  ASSERT_EQ(f->params.size(), 0u);

  auto& attributes = f->attributes;
  ASSERT_EQ(attributes.size(), 1u);
  ASSERT_TRUE(attributes[0]->Is<ast::WorkgroupAttribute>());

  auto values = attributes[0]->As<ast::WorkgroupAttribute>()->Values();

  ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->ValueAsU32(), 2u);

  ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->ValueAsU32(), 3u);

  ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->ValueAsU32(), 4u);

  auto* body = f->body;
  ASSERT_EQ(body->statements.size(), 1u);
  EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_AttributeList_MultipleEntries) {
  auto p = parser(R"(
@workgroup_size(2, 3, 4) @stage(compute)
fn main() { return; })");
  auto attrs = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attrs.errored);
  ASSERT_TRUE(attrs.matched);
  auto f = p->function_decl(attrs.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol, p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type, nullptr);
  EXPECT_TRUE(f->return_type->Is<ast::Void>());
  ASSERT_EQ(f->params.size(), 0u);

  auto& attributes = f->attributes;
  ASSERT_EQ(attributes.size(), 2u);

  ASSERT_TRUE(attributes[0]->Is<ast::WorkgroupAttribute>());
  auto values = attributes[0]->As<ast::WorkgroupAttribute>()->Values();

  ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->ValueAsU32(), 2u);

  ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->ValueAsU32(), 3u);

  ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->ValueAsU32(), 4u);

  ASSERT_TRUE(attributes[1]->Is<ast::StageAttribute>());
  EXPECT_EQ(attributes[1]->As<ast::StageAttribute>()->stage,
            ast::PipelineStage::kCompute);

  auto* body = f->body;
  ASSERT_EQ(body->statements.size(), 1u);
  EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_AttributeList_MultipleLists) {
  auto p = parser(R"(
@workgroup_size(2, 3, 4)
@stage(compute)
fn main() { return; })");
  auto attributes = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attributes.errored);
  ASSERT_TRUE(attributes.matched);
  auto f = p->function_decl(attributes.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol, p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type, nullptr);
  EXPECT_TRUE(f->return_type->Is<ast::Void>());
  ASSERT_EQ(f->params.size(), 0u);

  auto& attrs = f->attributes;
  ASSERT_EQ(attrs.size(), 2u);

  ASSERT_TRUE(attrs[0]->Is<ast::WorkgroupAttribute>());
  auto values = attrs[0]->As<ast::WorkgroupAttribute>()->Values();

  ASSERT_TRUE(values[0]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[0]->As<ast::IntLiteralExpression>()->ValueAsU32(), 2u);

  ASSERT_TRUE(values[1]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[1]->As<ast::IntLiteralExpression>()->ValueAsU32(), 3u);

  ASSERT_TRUE(values[2]->Is<ast::IntLiteralExpression>());
  EXPECT_EQ(values[2]->As<ast::IntLiteralExpression>()->ValueAsU32(), 4u);

  ASSERT_TRUE(attrs[1]->Is<ast::StageAttribute>());
  EXPECT_EQ(attrs[1]->As<ast::StageAttribute>()->stage,
            ast::PipelineStage::kCompute);

  auto* body = f->body;
  ASSERT_EQ(body->statements.size(), 1u);
  EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_ReturnTypeAttributeList) {
  auto p = parser("fn main() -> @location(1) f32 { return 1.0; }");
  auto attrs = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attrs.errored);
  EXPECT_FALSE(attrs.matched);
  auto f = p->function_decl(attrs.value);
  EXPECT_FALSE(p->has_error()) << p->error();
  EXPECT_FALSE(f.errored);
  EXPECT_TRUE(f.matched);
  ASSERT_NE(f.value, nullptr);

  EXPECT_EQ(f->symbol, p->builder().Symbols().Get("main"));
  ASSERT_NE(f->return_type, nullptr);
  EXPECT_TRUE(f->return_type->Is<ast::F32>());
  ASSERT_EQ(f->params.size(), 0u);

  auto& attributes = f->attributes;
  EXPECT_EQ(attributes.size(), 0u);

  auto& ret_type_attributes = f->return_type_attributes;
  ASSERT_EQ(ret_type_attributes.size(), 1u);
  auto* loc = ret_type_attributes[0]->As<ast::LocationAttribute>();
  ASSERT_TRUE(loc != nullptr);
  EXPECT_EQ(loc->value, 1u);

  auto* body = f->body;
  ASSERT_EQ(body->statements.size(), 1u);
  EXPECT_TRUE(body->statements[0]->Is<ast::ReturnStatement>());
}

TEST_F(ParserImplTest, FunctionDecl_InvalidHeader) {
  auto p = parser("fn main() -> { }");
  auto attrs = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attrs.errored);
  EXPECT_FALSE(attrs.matched);
  auto f = p->function_decl(attrs.value);
  EXPECT_TRUE(f.errored);
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(f.value, nullptr);
  EXPECT_EQ(p->error(), "1:14: unable to determine function return type");
}

TEST_F(ParserImplTest, FunctionDecl_InvalidBody) {
  auto p = parser("fn main() { return }");
  auto attrs = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attrs.errored);
  EXPECT_FALSE(attrs.matched);
  auto f = p->function_decl(attrs.value);
  EXPECT_TRUE(f.errored);
  EXPECT_FALSE(f.matched);
  EXPECT_TRUE(p->has_error());
  EXPECT_EQ(f.value, nullptr);
  EXPECT_EQ(p->error(), "1:20: expected ';' for return statement");
}

TEST_F(ParserImplTest, FunctionDecl_MissingLeftBrace) {
  auto p = parser("fn main() return; }");
  auto attrs = p->attribute_list();
  EXPECT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(attrs.errored);
  EXPECT_FALSE(attrs.matched);
  auto f = p->function_decl(attrs.value);
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
