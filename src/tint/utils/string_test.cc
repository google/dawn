// Copyright 2021 The Tint Authors.
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

#include "src/tint/utils/string.h"

#include "gtest/gtest.h"
#include "src/tint/utils/string_stream.h"

namespace tint::utils {
namespace {

TEST(StringTest, ReplaceAll) {
    EXPECT_EQ("xybbcc", ReplaceAll("aabbcc", "aa", "xy"));
    EXPECT_EQ("aaxycc", ReplaceAll("aabbcc", "bb", "xy"));
    EXPECT_EQ("aabbxy", ReplaceAll("aabbcc", "cc", "xy"));
    EXPECT_EQ("xyxybbcc", ReplaceAll("aabbcc", "a", "xy"));
    EXPECT_EQ("aaxyxycc", ReplaceAll("aabbcc", "b", "xy"));
    EXPECT_EQ("aabbxyxy", ReplaceAll("aabbcc", "c", "xy"));
    // Replacement string includes the searched-for string.
    // This proves that the algorithm needs to advance 'pos'
    // past the replacement.
    EXPECT_EQ("aabxybbxybcc", ReplaceAll("aabbcc", "b", "bxyb"));
}

TEST(StringTest, ToString) {
    EXPECT_EQ("123", ToString(123));
    EXPECT_EQ("hello", ToString("hello"));
}

TEST(StringTest, HasPrefix) {
    EXPECT_TRUE(HasPrefix("abc", "a"));
    EXPECT_TRUE(HasPrefix("abc", "ab"));
    EXPECT_TRUE(HasPrefix("abc", "abc"));
    EXPECT_FALSE(HasPrefix("abc", "abc1"));
    EXPECT_FALSE(HasPrefix("abc", "ac"));
    EXPECT_FALSE(HasPrefix("abc", "b"));
}

TEST(StringTest, HasSuffix) {
    EXPECT_TRUE(HasSuffix("abc", "c"));
    EXPECT_TRUE(HasSuffix("abc", "bc"));
    EXPECT_TRUE(HasSuffix("abc", "abc"));
    EXPECT_FALSE(HasSuffix("abc", "1abc"));
    EXPECT_FALSE(HasSuffix("abc", "ac"));
    EXPECT_FALSE(HasSuffix("abc", "b"));
}

TEST(StringTest, Distance) {
    EXPECT_EQ(Distance("hello world", "hello world"), 0u);
    EXPECT_EQ(Distance("hello world", "helloworld"), 1u);
    EXPECT_EQ(Distance("helloworld", "hello world"), 1u);
    EXPECT_EQ(Distance("hello world", "hello  world"), 1u);
    EXPECT_EQ(Distance("hello  world", "hello world"), 1u);
    EXPECT_EQ(Distance("Hello World", "hello world"), 2u);
    EXPECT_EQ(Distance("hello world", "Hello World"), 2u);
    EXPECT_EQ(Distance("Hello world", ""), 11u);
    EXPECT_EQ(Distance("", "Hello world"), 11u);
}

TEST(StringTest, SuggestAlternatives) {
    {
        const char* alternatives[] = {"hello world", "Hello World"};
        utils::StringStream ss;
        SuggestAlternatives("hello wordl", alternatives, ss);
        EXPECT_EQ(ss.str(), R"(Did you mean 'hello world'?
Possible values: 'hello world', 'Hello World')");
    }
    {
        const char* alternatives[] = {"foobar", "something else"};
        utils::StringStream ss;
        SuggestAlternatives("hello world", alternatives, ss);
        EXPECT_EQ(ss.str(), R"(Possible values: 'foobar', 'something else')");
    }
}

TEST(StringTest, TrimLeft) {
    EXPECT_EQ(TrimLeft("hello world", [](char) { return false; }), "hello world");
    EXPECT_EQ(TrimLeft("hello world", [](char c) { return c == 'h'; }), "ello world");
    EXPECT_EQ(TrimLeft("hello world", [](char c) { return c == 'h' || c == 'e'; }), "llo world");
    EXPECT_EQ(TrimLeft("hello world", [](char c) { return c == 'e'; }), "hello world");
    EXPECT_EQ(TrimLeft("hello world", [](char) { return true; }), "");
    EXPECT_EQ(TrimLeft("", [](char) { return false; }), "");
    EXPECT_EQ(TrimLeft("", [](char) { return true; }), "");
}

TEST(StringTest, TrimRight) {
    EXPECT_EQ(TrimRight("hello world", [](char) { return false; }), "hello world");
    EXPECT_EQ(TrimRight("hello world", [](char c) { return c == 'd'; }), "hello worl");
    EXPECT_EQ(TrimRight("hello world", [](char c) { return c == 'd' || c == 'l'; }), "hello wor");
    EXPECT_EQ(TrimRight("hello world", [](char c) { return c == 'l'; }), "hello world");
    EXPECT_EQ(TrimRight("hello world", [](char) { return true; }), "");
    EXPECT_EQ(TrimRight("", [](char) { return false; }), "");
    EXPECT_EQ(TrimRight("", [](char) { return true; }), "");
}

TEST(StringTest, TrimPrefix) {
    EXPECT_EQ(TrimPrefix("abc", "a"), "bc");
    EXPECT_EQ(TrimPrefix("abc", "ab"), "c");
    EXPECT_EQ(TrimPrefix("abc", "abc"), "");
    EXPECT_EQ(TrimPrefix("abc", "abc1"), "abc");
    EXPECT_EQ(TrimPrefix("abc", "ac"), "abc");
    EXPECT_EQ(TrimPrefix("abc", "b"), "abc");
    EXPECT_EQ(TrimPrefix("abc", "c"), "abc");
}

TEST(StringTest, TrimSuffix) {
    EXPECT_EQ(TrimSuffix("abc", "c"), "ab");
    EXPECT_EQ(TrimSuffix("abc", "bc"), "a");
    EXPECT_EQ(TrimSuffix("abc", "abc"), "");
    EXPECT_EQ(TrimSuffix("abc", "1abc"), "abc");
    EXPECT_EQ(TrimSuffix("abc", "ac"), "abc");
    EXPECT_EQ(TrimSuffix("abc", "b"), "abc");
    EXPECT_EQ(TrimSuffix("abc", "a"), "abc");
}

TEST(StringTest, Trim) {
    EXPECT_EQ(Trim("hello world", [](char) { return false; }), "hello world");
    EXPECT_EQ(Trim("hello world", [](char c) { return c == 'h'; }), "ello world");
    EXPECT_EQ(Trim("hello world", [](char c) { return c == 'd'; }), "hello worl");
    EXPECT_EQ(Trim("hello world", [](char c) { return c == 'h' || c == 'd'; }), "ello worl");
    EXPECT_EQ(Trim("hello world", [](char) { return true; }), "");
    EXPECT_EQ(Trim("", [](char) { return false; }), "");
    EXPECT_EQ(Trim("", [](char) { return true; }), "");
}

TEST(StringTest, IsSpace) {
    EXPECT_FALSE(IsSpace('a'));
    EXPECT_FALSE(IsSpace('z'));
    EXPECT_FALSE(IsSpace('\0'));
    EXPECT_TRUE(IsSpace(' '));
    EXPECT_TRUE(IsSpace('\f'));
    EXPECT_TRUE(IsSpace('\n'));
    EXPECT_TRUE(IsSpace('\r'));
    EXPECT_TRUE(IsSpace('\t'));
    EXPECT_TRUE(IsSpace('\v'));
}

TEST(StringTest, TrimSpace) {
    EXPECT_EQ(TrimSpace("hello world"), "hello world");
    EXPECT_EQ(TrimSpace(" \t hello world\v\f"), "hello world");
    EXPECT_EQ(TrimSpace("hello \t world"), "hello \t world");
    EXPECT_EQ(TrimSpace(""), "");
}

}  // namespace
}  // namespace tint::utils
