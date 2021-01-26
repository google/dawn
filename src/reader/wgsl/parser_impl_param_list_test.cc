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

#include <memory>

#include "gtest/gtest.h"
#include "src/ast/variable.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type/f32_type.h"
#include "src/type/i32_type.h"
#include "src/type/vector_type.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ParamList_Single) {
  auto p = parser("a : i32");

  auto& mod = p->get_program();
  auto* i32 = mod.create<type::I32>();

  auto e = p->expect_param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  EXPECT_EQ(e.value.size(), 1u);

  EXPECT_EQ(e.value[0]->symbol(), p->get_program().RegisterSymbol("a"));
  EXPECT_EQ(e.value[0]->type(), i32);
  EXPECT_TRUE(e.value[0]->is_const());

  ASSERT_EQ(e.value[0]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.begin.column, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.column, 2u);
}

TEST_F(ParserImplTest, ParamList_Multiple) {
  auto p = parser("a : i32, b: f32, c: vec2<f32>");

  auto& mod = p->get_program();
  auto* i32 = mod.create<type::I32>();
  auto* f32 = mod.create<type::F32>();
  auto* vec2 = mod.create<type::Vector>(f32, 2);

  auto e = p->expect_param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  ASSERT_FALSE(e.errored);
  EXPECT_EQ(e.value.size(), 3u);

  EXPECT_EQ(e.value[0]->symbol(), p->get_program().RegisterSymbol("a"));
  EXPECT_EQ(e.value[0]->type(), i32);
  EXPECT_TRUE(e.value[0]->is_const());

  ASSERT_EQ(e.value[0]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.begin.column, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[0]->source().range.end.column, 2u);

  EXPECT_EQ(e.value[1]->symbol(), p->get_program().RegisterSymbol("b"));
  EXPECT_EQ(e.value[1]->type(), f32);
  EXPECT_TRUE(e.value[1]->is_const());

  ASSERT_EQ(e.value[1]->source().range.begin.line, 1u);
  ASSERT_EQ(e.value[1]->source().range.begin.column, 10u);
  ASSERT_EQ(e.value[1]->source().range.end.line, 1u);
  ASSERT_EQ(e.value[1]->source().range.end.column, 11u);

  EXPECT_EQ(e.value[2]->symbol(), p->get_program().RegisterSymbol("c"));
  EXPECT_EQ(e.value[2]->type(), vec2);
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

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
