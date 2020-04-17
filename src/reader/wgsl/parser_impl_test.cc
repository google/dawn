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

#include "src/reader/wgsl/parser_impl.h"

#include "gtest/gtest.h"
#include "src/ast/type/i32_type.h"
#include "src/reader/wgsl/parser_impl_test_helper.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

TEST_F(ParserImplTest, Empty) {
  auto* p = parser("");
  ASSERT_TRUE(p->Parse()) << p->error();
}

TEST_F(ParserImplTest, Parses) {
  auto* p = parser(R"(
import "GLSL.std.430" as glsl;

[[location 0]] var<out> gl_FragColor : vec4<f32>;

entry_point vertex = main;
fn main() -> void {
  gl_FragColor = vec4<f32>(.4, .2, .3, 1);
}
)");
  ASSERT_TRUE(p->Parse()) << p->error();

  auto m = p->module();
  ASSERT_EQ(1u, m.imports().size());
  ASSERT_EQ(1u, m.entry_points().size());
  ASSERT_EQ(1u, m.functions().size());
  ASSERT_EQ(1u, m.global_variables().size());
}

TEST_F(ParserImplTest, HandlesError) {
  auto* p = parser(R"(
import "GLSL.std.430" as glsl;

fn main() ->  {  # missing return type
  return;
})");

  ASSERT_FALSE(p->Parse());
  ASSERT_TRUE(p->has_error());
  EXPECT_EQ(p->error(), "4:15: unable to determine function return type");
}

TEST_F(ParserImplTest, GetRegisteredType) {
  auto* p = parser("");
  ast::type::I32Type i32;
  p->register_alias("my_alias", &i32);

  auto* alias = p->get_alias("my_alias");
  ASSERT_NE(alias, nullptr);
  ASSERT_EQ(alias, &i32);
}

TEST_F(ParserImplTest, GetUnregisteredType) {
  auto* p = parser("");
  auto* alias = p->get_alias("my_alias");
  ASSERT_EQ(alias, nullptr);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
