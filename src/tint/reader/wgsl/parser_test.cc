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

#include "src/tint/reader/wgsl/parser.h"

#include "gtest/gtest.h"

#include "src/tint/ast/module.h"

namespace tint::reader::wgsl {
namespace {

using ParserTest = testing::Test;

TEST_F(ParserTest, Empty) {
    Source::File file("test.wgsl", "");
    auto program = Parse(&file);
    auto errs = diag::Formatter().format(program.Diagnostics());
    ASSERT_TRUE(program.IsValid()) << errs;
}

TEST_F(ParserTest, Parses) {
    Source::File file("test.wgsl", R"(
@fragment
fn main() -> @location(0) vec4<f32> {
  return vec4<f32>(.4, .2, .3, 1.);
}
)");
    auto program = Parse(&file);
    auto errs = diag::Formatter().format(program.Diagnostics());
    ASSERT_TRUE(program.IsValid()) << errs;

    ASSERT_EQ(1u, program.AST().Functions().Length());
}

TEST_F(ParserTest, HandlesError) {
    Source::File file("test.wgsl", R"(
fn main() ->  {  // missing return type
  return;
})");

    auto program = Parse(&file);
    auto errs = diag::Formatter().format(program.Diagnostics());
    ASSERT_FALSE(program.IsValid()) << errs;
    EXPECT_EQ(errs,
              R"(test.wgsl:2:15 error: unable to determine function return type
fn main() ->  {  // missing return type
              ^

)");
}

}  // namespace
}  // namespace tint::reader::wgsl
