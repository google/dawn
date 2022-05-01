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

#include "src/tint/reader/wgsl/parser_impl_test_helper.h"

namespace tint::reader::wgsl {
namespace {

TEST_F(ParserImplTest, ParamList_Single) {
    auto p = parser("a : i32");

    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.size(), 1u);

    EXPECT_EQ(e.value[0]->symbol, p->builder().Symbols().Get("a"));
    EXPECT_TRUE(e.value[0]->type->Is<ast::I32>());
    EXPECT_TRUE(e.value[0]->is_const);

    ASSERT_EQ(e.value[0]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.begin.column, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.column, 2u);
}

TEST_F(ParserImplTest, ParamList_Multiple) {
    auto p = parser("a : i32, b: f32, c: vec2<f32>");

    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.size(), 3u);

    EXPECT_EQ(e.value[0]->symbol, p->builder().Symbols().Get("a"));
    EXPECT_TRUE(e.value[0]->type->Is<ast::I32>());
    EXPECT_TRUE(e.value[0]->is_const);

    ASSERT_EQ(e.value[0]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.begin.column, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.column, 2u);

    EXPECT_EQ(e.value[1]->symbol, p->builder().Symbols().Get("b"));
    EXPECT_TRUE(e.value[1]->type->Is<ast::F32>());
    EXPECT_TRUE(e.value[1]->is_const);

    ASSERT_EQ(e.value[1]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[1]->source.range.begin.column, 10u);
    ASSERT_EQ(e.value[1]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[1]->source.range.end.column, 11u);

    EXPECT_EQ(e.value[2]->symbol, p->builder().Symbols().Get("c"));
    ASSERT_TRUE(e.value[2]->type->Is<ast::Vector>());
    ASSERT_TRUE(e.value[2]->type->As<ast::Vector>()->type->Is<ast::F32>());
    EXPECT_EQ(e.value[2]->type->As<ast::Vector>()->width, 2u);
    EXPECT_TRUE(e.value[2]->is_const);

    ASSERT_EQ(e.value[2]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[2]->source.range.begin.column, 18u);
    ASSERT_EQ(e.value[2]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[2]->source.range.end.column, 19u);
}

TEST_F(ParserImplTest, ParamList_Empty) {
    auto p = parser("");
    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.size(), 0u);
}

TEST_F(ParserImplTest, ParamList_TrailingComma) {
    auto p = parser("a : i32,");
    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error());
    ASSERT_FALSE(e.errored);
    EXPECT_EQ(e.value.size(), 1u);
}

TEST_F(ParserImplTest, ParamList_Attributes) {
    auto p = parser("@builtin(position) coord : vec4<f32>, @location(1) loc1 : f32");

    auto e = p->expect_param_list();
    ASSERT_FALSE(p->has_error()) << p->error();
    ASSERT_FALSE(e.errored);
    ASSERT_EQ(e.value.size(), 2u);

    EXPECT_EQ(e.value[0]->symbol, p->builder().Symbols().Get("coord"));
    ASSERT_TRUE(e.value[0]->type->Is<ast::Vector>());
    EXPECT_TRUE(e.value[0]->type->As<ast::Vector>()->type->Is<ast::F32>());
    EXPECT_EQ(e.value[0]->type->As<ast::Vector>()->width, 4u);
    EXPECT_TRUE(e.value[0]->is_const);
    auto attrs_0 = e.value[0]->attributes;
    ASSERT_EQ(attrs_0.size(), 1u);
    EXPECT_TRUE(attrs_0[0]->Is<ast::BuiltinAttribute>());
    EXPECT_EQ(attrs_0[0]->As<ast::BuiltinAttribute>()->builtin, ast::Builtin::kPosition);

    ASSERT_EQ(e.value[0]->source.range.begin.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.begin.column, 20u);
    ASSERT_EQ(e.value[0]->source.range.end.line, 1u);
    ASSERT_EQ(e.value[0]->source.range.end.column, 25u);

    EXPECT_EQ(e.value[1]->symbol, p->builder().Symbols().Get("loc1"));
    EXPECT_TRUE(e.value[1]->type->Is<ast::F32>());
    EXPECT_TRUE(e.value[1]->is_const);
    auto attrs_1 = e.value[1]->attributes;
    ASSERT_EQ(attrs_1.size(), 1u);
    EXPECT_TRUE(attrs_1[0]->Is<ast::LocationAttribute>());
    EXPECT_EQ(attrs_1[0]->As<ast::LocationAttribute>()->value, 1u);

    EXPECT_EQ(e.value[1]->source.range.begin.line, 1u);
    EXPECT_EQ(e.value[1]->source.range.begin.column, 52u);
    EXPECT_EQ(e.value[1]->source.range.end.line, 1u);
    EXPECT_EQ(e.value[1]->source.range.end.column, 56u);
}

}  // namespace
}  // namespace tint::reader::wgsl
