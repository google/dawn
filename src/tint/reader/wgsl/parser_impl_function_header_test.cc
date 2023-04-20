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

#include "src/tint/ast/test_helper.h"
#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, FunctionHeader) {
    auto p = parser("fn main(a : i32, b: f32)");
    auto f = p->function_header();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    ASSERT_EQ(f->params.Length(), 2u);
    EXPECT_EQ(f->params[0]->name->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(f->params[1]->name->symbol, p->builder().Symbols().Get("b"));
    EXPECT_EQ(f->return_type, nullptr);
}

TEST_F(ParserImplTest, FunctionHeader_TrailingComma) {
    auto p = parser("fn main(a :i32,)");
    auto f = p->function_header();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    ASSERT_EQ(f->params.Length(), 1u);
    EXPECT_EQ(f->params[0]->name->symbol, p->builder().Symbols().Get("a"));
    EXPECT_EQ(f->return_type, nullptr);
}

TEST_F(ParserImplTest, FunctionHeader_AttributeReturnType) {
    auto p = parser("fn main() -> @location(1) f32");
    auto f = p->function_header();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    EXPECT_EQ(f->params.Length(), 0u);
    ast::CheckIdentifier(f->return_type, "f32");
    ASSERT_EQ(f->return_type_attributes.Length(), 1u);

    auto* loc = f->return_type_attributes[0]->As<ast::LocationAttribute>();
    ASSERT_TRUE(loc != nullptr);
    ASSERT_TRUE(loc->expr->Is<ast::IntLiteralExpression>());
    auto* exp = loc->expr->As<ast::IntLiteralExpression>();
    EXPECT_EQ(exp->value, 1u);
}

TEST_F(ParserImplTest, FunctionHeader_InvariantReturnType) {
    auto p = parser("fn main() -> @invariant f32");
    auto f = p->function_header();
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_TRUE(f.matched);
    EXPECT_FALSE(f.errored);

    ast::CheckIdentifier(f->name, "main");
    EXPECT_EQ(f->params.Length(), 0u);
    ast::CheckIdentifier(f->return_type, "f32");
    ASSERT_EQ(f->return_type_attributes.Length(), 1u);
    EXPECT_TRUE(f->return_type_attributes[0]->Is<ast::InvariantAttribute>());
}

TEST_F(ParserImplTest, FunctionHeader_MissingIdent) {
    auto p = parser("fn ()");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected identifier for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidIdent) {
    auto p = parser("fn 133main() -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:4: expected identifier for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_MissingParenLeft) {
    auto p = parser("fn main) -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:8: expected '(' for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_InvalidParamList) {
    auto p = parser("fn main(a :i32, ,) -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:17: expected ')' for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_MissingParenRight) {
    auto p = parser("fn main( -> i32");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:10: expected ')' for function declaration");
}

TEST_F(ParserImplTest, FunctionHeader_MissingReturnType) {
    auto p = parser("fn main() ->");
    auto f = p->function_header();
    EXPECT_FALSE(f.matched);
    EXPECT_TRUE(f.errored);
    EXPECT_TRUE(p->has_error());
    EXPECT_EQ(p->error(), "1:13: unable to determine function return type");
}

}  // namespace
}  // namespace tint::reader::wgsl
