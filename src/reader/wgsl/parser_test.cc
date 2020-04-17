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

#include "src/reader/wgsl/parser.h"

#include "gtest/gtest.h"
#include "src/context.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

using ParserTest = testing::Test;

TEST_F(ParserTest, Empty) {
  Context ctx;
  Parser p(&ctx, "");
  ASSERT_TRUE(p.Parse()) << p.error();
}

TEST_F(ParserTest, Parses) {
  Context ctx;

  Parser p(&ctx, R"(
import "GLSL.std.430" as glsl;

[[location 0]] var<out> gl_FragColor : vec4<f32>;

entry_point vertex = main;
fn main() -> void {
  gl_FragColor = vec4<f32>(.4, .2, .3, 1);
}
)");
  ASSERT_TRUE(p.Parse()) << p.error();

  auto m = p.module();
  ASSERT_EQ(1u, m.imports().size());
  ASSERT_EQ(1u, m.entry_points().size());
  ASSERT_EQ(1u, m.functions().size());
  ASSERT_EQ(1u, m.global_variables().size());
}

TEST_F(ParserTest, HandlesError) {
  Context ctx;
  Parser p(&ctx, R"(
import "GLSL.std.430" as glsl;

fn main() ->  {  # missing return type
  return;
})");

  ASSERT_FALSE(p.Parse());
  ASSERT_TRUE(p.has_error());
  EXPECT_EQ(p.error(), "4:15: unable to determine function return type");
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
