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

#include "src/reader/wgsl/token.h"

#include <limits>

#include "gtest/gtest.h"

namespace tint {
namespace reader {
namespace wgsl {
namespace {

using TokenTest = testing::Test;

TEST_F(TokenTest, ReturnsF32) {
  Token t1(Source{}, -2.345f);
  EXPECT_EQ(t1.to_f32(), -2.345f);

  Token t2(Source{}, 2.345f);
  EXPECT_EQ(t2.to_f32(), 2.345f);
}

TEST_F(TokenTest, ReturnsI32) {
  Token t1(Source{}, -2345);
  EXPECT_EQ(t1.to_i32(), -2345);

  Token t2(Source{}, 2345);
  EXPECT_EQ(t2.to_i32(), 2345);
}

TEST_F(TokenTest, HandlesMaxI32) {
  Token t1(Source{}, std::numeric_limits<int32_t>::max());
  EXPECT_EQ(t1.to_i32(), std::numeric_limits<int32_t>::max());
}

TEST_F(TokenTest, HandlesMinI32) {
  Token t1(Source{}, std::numeric_limits<int32_t>::min());
  EXPECT_EQ(t1.to_i32(), std::numeric_limits<int32_t>::min());
}

TEST_F(TokenTest, ReturnsU32) {
  Token t2(Source{}, 2345u);
  EXPECT_EQ(t2.to_u32(), 2345u);
}

TEST_F(TokenTest, ReturnsMaxU32) {
  Token t1(Source{}, std::numeric_limits<uint32_t>::max());
  EXPECT_EQ(t1.to_u32(), std::numeric_limits<uint32_t>::max());
}

TEST_F(TokenTest, Source) {
  Source src;
  src.range.begin = Source::Location{3, 9};
  src.range.end = Source::Location{4, 3};

  Token t(Token::Type::kUintLiteral, src);
  EXPECT_EQ(t.source().range.begin.line, 3u);
  EXPECT_EQ(t.source().range.begin.column, 9u);
  EXPECT_EQ(t.source().range.end.line, 4u);
  EXPECT_EQ(t.source().range.end.column, 3u);
}

}  // namespace
}  // namespace wgsl
}  // namespace reader
}  // namespace tint
