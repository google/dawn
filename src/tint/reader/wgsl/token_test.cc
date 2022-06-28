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

#include "src/tint/reader/wgsl/token.h"

#include <limits>

#include "gmock/gmock.h"

namespace tint::reader::wgsl {
namespace {

using ::testing::EndsWith;
using ::testing::Not;
using ::testing::StartsWith;

using TokenTest = testing::Test;

TEST_F(TokenTest, ReturnsF64) {
    Token t1(Token::Type::kFloatLiteral_F, Source{}, -2.345);
    EXPECT_EQ(t1.to_f64(), -2.345);

    Token t2(Token::Type::kFloatLiteral_F, Source{}, 2.345);
    EXPECT_EQ(t2.to_f64(), 2.345);
}

TEST_F(TokenTest, ReturnsI32) {
    Token t1(Token::Type::kIntLiteral_I, Source{}, static_cast<int64_t>(-2345));
    EXPECT_EQ(t1.to_i64(), -2345);

    Token t2(Token::Type::kIntLiteral_I, Source{}, static_cast<int64_t>(2345));
    EXPECT_EQ(t2.to_i64(), 2345);
}

TEST_F(TokenTest, HandlesMaxI32) {
    Token t1(Token::Type::kIntLiteral_I, Source{},
             static_cast<int64_t>(std::numeric_limits<int32_t>::max()));
    EXPECT_EQ(t1.to_i64(), std::numeric_limits<int32_t>::max());
}

TEST_F(TokenTest, HandlesMinI32) {
    Token t1(Token::Type::kIntLiteral_I, Source{},
             static_cast<int64_t>(std::numeric_limits<int32_t>::min()));
    EXPECT_EQ(t1.to_i64(), std::numeric_limits<int32_t>::min());
}

TEST_F(TokenTest, ReturnsU32) {
    Token t2(Token::Type::kIntLiteral_U, Source{}, static_cast<int64_t>(2345u));
    EXPECT_EQ(t2.to_i64(), 2345u);
}

TEST_F(TokenTest, ReturnsMaxU32) {
    Token t1(Token::Type::kIntLiteral_U, Source{},
             static_cast<int64_t>(std::numeric_limits<uint32_t>::max()));
    EXPECT_EQ(t1.to_i64(), std::numeric_limits<uint32_t>::max());
}

TEST_F(TokenTest, Source) {
    Source src;
    src.range.begin = Source::Location{3, 9};
    src.range.end = Source::Location{4, 3};

    Token t(Token::Type::kIntLiteral, src);
    EXPECT_EQ(t.source().range.begin.line, 3u);
    EXPECT_EQ(t.source().range.begin.column, 9u);
    EXPECT_EQ(t.source().range.end.line, 4u);
    EXPECT_EQ(t.source().range.end.column, 3u);
}

TEST_F(TokenTest, ToStr) {
    double d = 123.0;
    int64_t i = 123;
    EXPECT_THAT(Token(Token::Type::kFloatLiteral, Source{}, d).to_str(), StartsWith("123"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral, Source{}, d).to_str(), Not(EndsWith("f")));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_F, Source{}, d).to_str(), StartsWith("123"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_F, Source{}, d).to_str(), EndsWith("f"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_H, Source{}, d).to_str(), StartsWith("123"));
    EXPECT_THAT(Token(Token::Type::kFloatLiteral_H, Source{}, d).to_str(), EndsWith("h"));
    EXPECT_EQ(Token(Token::Type::kIntLiteral, Source{}, i).to_str(), "123");
    EXPECT_EQ(Token(Token::Type::kIntLiteral_I, Source{}, i).to_str(), "123i");
    EXPECT_EQ(Token(Token::Type::kIntLiteral_U, Source{}, i).to_str(), "123u");
    EXPECT_EQ(Token(Token::Type::kIdentifier, Source{}, "blah").to_str(), "blah");
    EXPECT_EQ(Token(Token::Type::kError, Source{}, "blah").to_str(), "blah");
}

}  // namespace
}  // namespace tint::reader::wgsl
