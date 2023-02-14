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

using namespace tint::number_suffixes;  // NOLINT

TEST_F(ParserImplTest, Callable_Array) {
    auto p = parser("array");
    auto t = p->callable();
    ASSERT_TRUE(p->peek_is(Token::Type::kEOF));
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(p->builder().Symbols(), t.value, "array");
}

TEST_F(ParserImplTest, Callable_VecPrefix) {
    auto p = parser("vec3");
    auto t = p->callable();
    ASSERT_TRUE(p->peek_is(Token::Type::kEOF));
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(p->builder().Symbols(), t.value, "vec3");
}

TEST_F(ParserImplTest, Callable_MatPrefix) {
    auto p = parser("mat3x2");
    auto t = p->callable();
    ASSERT_TRUE(p->peek_is(Token::Type::kEOF));
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(p->builder().Symbols(), t.value, "mat3x2");
}

TEST_F(ParserImplTest, Callable_TypeDecl_Array) {
    auto p = parser("array<f32, 2>");
    auto t = p->callable();
    ASSERT_TRUE(p->peek_is(Token::Type::kEOF));
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(p->builder().Symbols(), t.value, ast::Template("array", "f32", 2_a));
}

TEST_F(ParserImplTest, Callable_TypeDecl_Array_Runtime) {
    auto p = parser("array<f32>");
    auto t = p->callable();
    ASSERT_TRUE(p->peek_is(Token::Type::kEOF));
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(p->builder().Symbols(), t.value, ast::Template("array", "f32"));
}

TEST_F(ParserImplTest, Callable_TypeDecl_VecPrefix) {
    auto p = parser("vec3<f32>");
    auto t = p->callable();
    ASSERT_TRUE(p->peek_is(Token::Type::kEOF));
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(p->builder().Symbols(), t.value, ast::Template("vec3", "f32"));
}

TEST_F(ParserImplTest, Callable_TypeDecl_MatPrefix) {
    auto p = parser("mat3x2<f32>");
    auto t = p->callable();
    ASSERT_TRUE(p->peek_is(Token::Type::kEOF));
    EXPECT_TRUE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_NE(t.value, nullptr);

    ast::CheckIdentifier(p->builder().Symbols(), t.value, ast::Template("mat3x2", "f32"));
}

TEST_F(ParserImplTest, Callable_NoMatch) {
    auto p = parser("ident");
    auto t = p->callable();
    EXPECT_FALSE(t.matched);
    EXPECT_FALSE(t.errored);
    ASSERT_FALSE(p->has_error()) << p->error();
    EXPECT_EQ(nullptr, t.value);
}

}  // namespace
}  // namespace tint::reader::wgsl
