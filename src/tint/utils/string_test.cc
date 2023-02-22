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
        std::ostringstream ss;
        SuggestAlternatives("hello wordl", alternatives, ss);
        EXPECT_EQ(ss.str(), R"(Did you mean 'hello world'?
Possible values: 'hello world', 'Hello World')");
    }
    {
        const char* alternatives[] = {"foobar", "something else"};
        std::ostringstream ss;
        SuggestAlternatives("hello world", alternatives, ss);
        EXPECT_EQ(ss.str(), R"(Possible values: 'foobar', 'something else')");
    }
}

}  // namespace
}  // namespace tint::utils
