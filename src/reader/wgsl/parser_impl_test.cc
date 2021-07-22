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

TEST_F(ParserImplTest, Empty) {
  auto p = parser("");
  ASSERT_TRUE(p->Parse()) << p->error();
}

TEST_F(ParserImplTest, Parses) {
  auto p = parser(R"(
[[stage(fragment)]]
fn main() -> [[location(0)]] vec4<f32> {
  return vec4<f32>(.4, .2, .3, 1);
}
)");
  ASSERT_TRUE(p->Parse()) << p->error();

  Program program = p->program();
  ASSERT_EQ(1u, program.AST().Functions().size());
}

TEST_F(ParserImplTest, HandlesError) {
  auto p = parser(R"(
fn main() ->  {  // missing return type
  return;
})");

  ASSERT_FALSE(p->Parse());
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "2:15: unable to determine function return type");
}

TEST_F(ParserImplTest, Comments) {
  auto p = parser(R"(
/**
 * Here is my shader.
 *
 * /* I can nest /**/ comments. */
 * // I can nest line comments too.
 **/
[[stage(fragment)]] // This is the stage
fn main(/*
no
parameters
*/) -> [[location(0)]] vec4<f32> {
  return/*block_comments_delimit_tokens*/vec4<f32>(.4, .2, .3, 1);
}/* unterminated block comments are OK at EOF...)");

  ASSERT_TRUE(p->Parse()) << p->error();
  ASSERT_EQ(1u, p->program().AST().Functions().size());
}

TEST_F(ParserImplTest, GetRegisteredType) {
  auto p = parser("");
  auto* alias = create<ast::Alias>(Sym("my_alias"), ty.i32());
  p->register_type("my_alias", alias);

  auto* got = p->get_type("my_alias");
  EXPECT_EQ(got, alias);
}

TEST_F(ParserImplTest, GetUnregisteredType) {
  auto p = parser("");
  auto* alias = p->get_type("my_alias");
  ASSERT_EQ(alias, nullptr);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
