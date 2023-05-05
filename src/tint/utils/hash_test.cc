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

#include "src/tint/utils/hash.h"

#include <string>
#include <tuple>
#include <unordered_map>

#include "gtest/gtest.h"
#include "src/tint/utils/vector.h"

namespace tint::utils {
namespace {

TEST(HashTests, Basic) {
    EXPECT_EQ(Hash(123), Hash(123));
    EXPECT_EQ(Hash(123, 456), Hash(123, 456));
    EXPECT_EQ(Hash(123, 456, false), Hash(123, 456, false));
    EXPECT_EQ(Hash(std::string("hello")), Hash(std::string("hello")));
}

TEST(HashTests, StdVector) {
    EXPECT_EQ(Hash(std::vector<int>({})), Hash(std::vector<int>({})));
    EXPECT_EQ(Hash(std::vector<int>({1, 2, 3})), Hash(std::vector<int>({1, 2, 3})));
}

TEST(HashTests, TintVector) {
    EXPECT_EQ(Hash(Vector<int, 0>({})), Hash(Vector<int, 0>({})));
    EXPECT_EQ(Hash(Vector<int, 0>({1, 2, 3})), Hash(Vector<int, 0>({1, 2, 3})));
    EXPECT_EQ(Hash(Vector<int, 3>({1, 2, 3})), Hash(Vector<int, 4>({1, 2, 3})));
    EXPECT_EQ(Hash(Vector<int, 3>({1, 2, 3})), Hash(Vector<int, 2>({1, 2, 3})));
}

TEST(HashTests, Tuple) {
    EXPECT_EQ(Hash(std::make_tuple(1)), Hash(std::make_tuple(1)));
    EXPECT_EQ(Hash(std::make_tuple(1, 2, 3)), Hash(std::make_tuple(1, 2, 3)));
}

TEST(HashTests, UnorderedKeyWrapper) {
    using W = UnorderedKeyWrapper<std::vector<int>>;

    std::unordered_map<W, int> m;

    m.emplace(W{{1, 2}}, -1);
    EXPECT_EQ(m.size(), 1u);
    EXPECT_EQ(m[W({1, 2})], -1);

    m.emplace(W{{3, 2}}, 1);
    EXPECT_EQ(m.size(), 2u);
    EXPECT_EQ(m[W({3, 2})], 1);
    EXPECT_EQ(m[W({1, 2})], -1);

    m.emplace(W{{100}}, 100);
    EXPECT_EQ(m.size(), 3u);
    EXPECT_EQ(m[W({100})], 100);
    EXPECT_EQ(m[W({3, 2})], 1);
    EXPECT_EQ(m[W({1, 2})], -1);

    // Reversed vector element order
    EXPECT_EQ(m[W({2, 3})], 0);
    EXPECT_EQ(m[W({2, 1})], 0);
}

TEST(EqualTo, String) {
    std::string str_a = "hello";
    std::string str_b = "world";
    const char* cstr_a = "hello";
    const char* cstr_b = "world";
    std::string_view sv_a = "hello";
    std::string_view sv_b = "world";
    EXPECT_TRUE(EqualTo<std::string>()(str_a, str_a));
    EXPECT_TRUE(EqualTo<std::string>()(str_a, cstr_a));
    EXPECT_TRUE(EqualTo<std::string>()(str_a, sv_a));
    EXPECT_TRUE(EqualTo<std::string>()(str_a, str_a));
    EXPECT_TRUE(EqualTo<std::string>()(cstr_a, str_a));
    EXPECT_TRUE(EqualTo<std::string>()(sv_a, str_a));

    EXPECT_FALSE(EqualTo<std::string>()(str_a, str_b));
    EXPECT_FALSE(EqualTo<std::string>()(str_a, cstr_b));
    EXPECT_FALSE(EqualTo<std::string>()(str_a, sv_b));
    EXPECT_FALSE(EqualTo<std::string>()(str_a, str_b));
    EXPECT_FALSE(EqualTo<std::string>()(cstr_a, str_b));
    EXPECT_FALSE(EqualTo<std::string>()(sv_a, str_b));

    EXPECT_FALSE(EqualTo<std::string>()(str_b, str_a));
    EXPECT_FALSE(EqualTo<std::string>()(str_b, cstr_a));
    EXPECT_FALSE(EqualTo<std::string>()(str_b, sv_a));
    EXPECT_FALSE(EqualTo<std::string>()(str_b, str_a));
    EXPECT_FALSE(EqualTo<std::string>()(cstr_b, str_a));
    EXPECT_FALSE(EqualTo<std::string>()(sv_b, str_a));

    EXPECT_TRUE(EqualTo<std::string>()(str_b, str_b));
    EXPECT_TRUE(EqualTo<std::string>()(str_b, cstr_b));
    EXPECT_TRUE(EqualTo<std::string>()(str_b, sv_b));
    EXPECT_TRUE(EqualTo<std::string>()(str_b, str_b));
    EXPECT_TRUE(EqualTo<std::string>()(cstr_b, str_b));
    EXPECT_TRUE(EqualTo<std::string>()(sv_b, str_b));
}

}  // namespace
}  // namespace tint::utils
