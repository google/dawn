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
#include "src/ast/type/f32_type.h"
#include "src/ast/type/i32_type.h"
#include "src/ast/type/vector_type.h"
#include "src/ast/variable.h"
#include "src/reader/wgsl/parser_impl.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"
#include "src/type_manager.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, ParamList_Single) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());

  auto* p = parser("a : i32");
  auto e = p->param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_EQ(e.size(), 1u);

  EXPECT_EQ(e[0]->name(), "a");
  EXPECT_EQ(e[0]->type(), i32);
}

TEST_F(ParserImplTest, ParamList_Multiple) {
  auto* i32 = tm()->Get(std::make_unique<ast::type::I32Type>());
  auto* f32 = tm()->Get(std::make_unique<ast::type::F32Type>());
  auto* vec2 = tm()->Get(std::make_unique<ast::type::VectorType>(f32, 2));

  auto* p = parser("a : i32, b: f32, c: vec2<f32>");
  auto e = p->param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_EQ(e.size(), 3u);

  EXPECT_EQ(e[0]->name(), "a");
  EXPECT_EQ(e[0]->type(), i32);

  EXPECT_EQ(e[1]->name(), "b");
  EXPECT_EQ(e[1]->type(), f32);

  EXPECT_EQ(e[2]->name(), "c");
  EXPECT_EQ(e[2]->type(), vec2);
}

TEST_F(ParserImplTest, ParamList_Empty) {
  auto* p = parser("");
  auto e = p->param_list();
  ASSERT_FALSE(p->has_error()) << p->error();
  EXPECT_EQ(e.size(), 0u);
}

TEST_F(ParserImplTest, ParamList_HangingComma) {
  auto* p = parser("a : i32,");
  auto e = p->param_list();
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "1:8: found , but no variable declaration");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
