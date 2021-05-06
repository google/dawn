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
[[location(0)]] var<out> gl_FragColor : vec4<f32>;

[[stage(vertex)]]
fn main() {
  gl_FragColor = vec4<f32>(.4, .2, .3, 1);
}
)");
  ASSERT_TRUE(p->Parse()) << p->error();

  Program program = p->program();
  ASSERT_EQ(1u, program.AST().Functions().size());
  ASSERT_EQ(1u, program.AST().GlobalVariables().size());
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

TEST_F(ParserImplTest, GetRegisteredType) {
  auto p = parser("");
  p->register_constructed("my_alias", ty.i32());

  auto* alias = p->get_constructed("my_alias");
  ASSERT_NE(alias, nullptr);
  EXPECT_TRUE(alias->Is<ast::I32>());
}

TEST_F(ParserImplTest, GetUnregisteredType) {
  auto p = parser("");
  auto* alias = p->get_constructed("my_alias");
  ASSERT_EQ(alias, nullptr);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
