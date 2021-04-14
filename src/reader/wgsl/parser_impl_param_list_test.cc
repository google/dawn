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

TEST_F(ParserImplTest, ParamList_Single) {
  auto p = parser("a : i32");

  auto* i32 = p->builder().create<type::I32>();

  auto e = p->expect_param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  EXPECT_EQ(e.value.size(), 1u);

  EXPECT_EQ(e.value[0]->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_EQ(e.value[0]->declared_type(), i32);
  EXPECT_TRUE(e.value[0]->is_const());

  ASSERT_EQ(e.value[0]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.begin.column, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.column, 2u);
}

TEST_F(ParserImplTest, ParamList_Multiple) {
  auto p = parser("a : i32, b: f32, c: vec2<f32>");

  auto* i32 = p->builder().create<type::I32>();
  auto* f32 = p->builder().create<type::F32>();
  auto* vec2 = p->builder().create<type::Vector>(f32, 2);

  auto e = p->expect_param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  EXPECT_EQ(e.value.size(), 3u);

  EXPECT_EQ(e.value[0]->symbol(), p->builder().Symbols().Get("a"));
  EXPECT_EQ(e.value[0]->declared_type(), i32);
  EXPECT_TRUE(e.value[0]->is_const());

  ASSERT_EQ(e.value[0]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.begin.column, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.column, 2u);

  EXPECT_EQ(e.value[1]->symbol(), p->builder().Symbols().Get("b"));
  EXPECT_EQ(e.value[1]->declared_type(), f32);
  EXPECT_TRUE(e.value[1]->is_const());

  ASSERT_EQ(e.value[1]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[1]->source().range.begin.column, 10u);
  ASSERT_EQ(e.value[1]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[1]->source().range.end.column, 11u);

  EXPECT_EQ(e.value[2]->symbol(), p->builder().Symbols().Get("c"));
  EXPECT_EQ(e.value[2]->declared_type(), vec2);
  EXPECT_TRUE(e.value[2]->is_const());

  ASSERT_EQ(e.value[2]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[2]->source().range.begin.column, 18u);
  ASSERT_EQ(e.value[2]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[2]->source().range.end.column, 19u);
}

TEST_F(ParserImplTest, ParamList_Empty) {
  auto p = parser("");
  auto e = p->expect_param_list();
  ASSERT_FALSE(p->has_error());
  ASSERT_FALSE(e.errored);
  EXPECT_EQ(e.value.size(), 0u);
}

TEST_F(ParserImplTest, ParamList_HangingComma) {
  auto p = parser("a : i32,");
  auto e = p->expect_param_list();
  ASSERT_TRUE(p->has_error());
  ASSERT_TRUE(e.errored);
  EXPECT_EQ(p->error(), "1:9: expected identifier for parameter");
}

TEST_F(ParserImplTest, ParamList_Decorations) {
  auto p = parser(
      "[[builtin(position)]] coord : vec4<f32>, "
      "[[location(1)]] loc1 : f32");

  auto* f32 = p->builder().create<type::F32>();
  auto* vec4 = p->builder().create<type::Vector>(f32, 4);

  auto e = p->expect_param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  ASSERT_EQ(e.value.size(), 2u);

  EXPECT_EQ(e.value[0]->symbol(), p->builder().Symbols().Get("coord"));
  EXPECT_EQ(e.value[0]->declared_type(), vec4);
  EXPECT_TRUE(e.value[0]->is_const());
  auto decos0 = e.value[0]->decorations();
  ASSERT_EQ(decos0.size(), 1u);
  EXPECT_TRUE(decos0[0]->Is<ast::BuiltinDecoration>());
  EXPECT_EQ(decos0[0]->As<ast::BuiltinDecoration>()->value(),
            ast::Builtin::kPosition);

  ASSERT_EQ(e.value[0]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.begin.column, 23u);
  ASSERT_EQ(e.value[0]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.column, 28u);

  EXPECT_EQ(e.value[1]->symbol(), p->builder().Symbols().Get("loc1"));
  EXPECT_EQ(e.value[1]->declared_type(), f32);
  EXPECT_TRUE(e.value[1]->is_const());
  auto decos1 = e.value[1]->decorations();
  ASSERT_EQ(decos1.size(), 1u);
  EXPECT_TRUE(decos1[0]->Is<ast::LocationDecoration>());
  EXPECT_EQ(decos1[0]->As<ast::LocationDecoration>()->value(), 1u);

  ASSERT_EQ(e.value[1]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[1]->source().range.begin.column, 58u);
  ASSERT_EQ(e.value[1]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[1]->source().range.end.column, 62u);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
