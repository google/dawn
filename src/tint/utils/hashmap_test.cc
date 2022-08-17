// Copyright 2022 The Tint Authors.
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

#include "src/tint/utils/hashmap.h"

#include <array>
#include <random>
#include <string>
#include <tuple>
#include <unordered_map>

#include "gmock/gmock.h"

namespace tint::utils {
namespace {

constexpr std::array kPrimes{
    2,   3,   5,   7,   11,  13,  17,  19,  23,  29,  31,  37,  41,  43,  47,  53,
    59,  61,  67,  71,  73,  79,  83,  89,  97,  101, 103, 107, 109, 113, 127, 131,
    137, 139, 149, 151, 157, 163, 167, 173, 179, 181, 191, 193, 197, 199, 211, 223,
    227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281, 283, 293, 307, 311,
    313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397, 401, 409,
};

TEST(Hashmap, Empty) {
    Hashmap<std::string, int, 8> map;
    EXPECT_EQ(map.Count(), 0u);
}

TEST(Hashmap, AddRemove) {
    Hashmap<std::string, std::string, 8> map;
    EXPECT_TRUE(map.Add("hello", "world"));
    EXPECT_EQ(map.Get("hello"), "world");
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_TRUE(map.Contains("hello"));
    EXPECT_FALSE(map.Contains("world"));
    EXPECT_FALSE(map.Add("hello", "cat"));
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_TRUE(map.Remove("hello"));
    EXPECT_EQ(map.Count(), 0u);
    EXPECT_FALSE(map.Contains("hello"));
    EXPECT_FALSE(map.Contains("world"));
}

TEST(Hashmap, ReplaceRemove) {
    Hashmap<std::string, std::string, 8> map;
    map.Replace("hello", "world");
    EXPECT_EQ(map.Get("hello"), "world");
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_TRUE(map.Contains("hello"));
    EXPECT_FALSE(map.Contains("world"));
    map.Replace("hello", "cat");
    EXPECT_EQ(map.Get("hello"), "cat");
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_TRUE(map.Remove("hello"));
    EXPECT_EQ(map.Count(), 0u);
    EXPECT_FALSE(map.Contains("hello"));
    EXPECT_FALSE(map.Contains("world"));
}

TEST(Hashmap, Iterator) {
    using Map = Hashmap<int, std::string, 8>;
    using KV = typename Map::KeyValue;
    Map map;
    map.Add(1, "one");
    map.Add(4, "four");
    map.Add(3, "three");
    map.Add(2, "two");
    EXPECT_THAT(map, testing::UnorderedElementsAre(KV{1, "one"}, KV{2, "two"}, KV{3, "three"},
                                                   KV{4, "four"}));
}

TEST(Hashmap, AddMany) {
    Hashmap<int, std::string, 8> map;
    for (size_t i = 0; i < kPrimes.size(); i++) {
        int prime = kPrimes[i];
        ASSERT_TRUE(map.Add(prime, std::to_string(prime))) << "i: " << i;
        ASSERT_FALSE(map.Add(prime, std::to_string(prime))) << "i: " << i;
        ASSERT_EQ(map.Count(), i + 1);
    }
    ASSERT_EQ(map.Count(), kPrimes.size());
    for (int prime : kPrimes) {
        ASSERT_TRUE(map.Contains(prime)) << prime;
        ASSERT_EQ(map.Get(prime), std::to_string(prime)) << prime;
    }
}

TEST(Hashmap, GetOrCreate) {
    Hashmap<int, std::string, 8> map;
    EXPECT_EQ(map.GetOrCreate(0, [&] { return "zero"; }), "zero");
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_EQ(map.Get(0), "zero");

    bool create_called = false;
    EXPECT_EQ(map.GetOrCreate(0,
                              [&] {
                                  create_called = true;
                                  return "oh noes";
                              }),
              "zero");
    EXPECT_FALSE(create_called);
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_EQ(map.Get(0), "zero");

    EXPECT_EQ(map.GetOrCreate(1, [&] { return "one"; }), "one");
    EXPECT_EQ(map.Count(), 2u);
    EXPECT_EQ(map.Get(1), "one");
}

TEST(Hashmap, Soak) {
    std::mt19937 rnd;
    std::unordered_map<std::string, std::string> reference;
    Hashmap<std::string, std::string, 8> map;
    for (size_t i = 0; i < 1000000; i++) {
        std::string key = std::to_string(rnd() & 64);
        std::string value = "V" + key;
        switch (rnd() % 7) {
            case 0: {  // Add
                auto expected = reference.emplace(key, value).second;
                EXPECT_EQ(map.Add(key, value), expected) << "i:" << i;
                EXPECT_EQ(map.Get(key), value) << "i:" << i;
                EXPECT_TRUE(map.Contains(key)) << "i:" << i;
                break;
            }
            case 1: {  // Replace
                reference[key] = value;
                map.Replace(key, value);
                EXPECT_EQ(map.Get(key), value) << "i:" << i;
                EXPECT_TRUE(map.Contains(key)) << "i:" << i;
                break;
            }
            case 2: {  // Remove
                auto expected = reference.erase(key) != 0;
                EXPECT_EQ(map.Remove(key), expected) << "i:" << i;
                EXPECT_FALSE(map.Get(key).has_value()) << "i:" << i;
                EXPECT_FALSE(map.Contains(key)) << "i:" << i;
                break;
            }
            case 3: {  // Contains
                auto expected = reference.count(key) != 0;
                EXPECT_EQ(map.Contains(key), expected) << "i:" << i;
                break;
            }
            case 4: {  // Get
                if (reference.count(key) != 0) {
                    auto expected = reference[key];
                    EXPECT_EQ(map.Get(key), expected) << "i:" << i;
                } else {
                    EXPECT_FALSE(map.Get(key).has_value()) << "i:" << i;
                }
                break;
            }
            case 5: {  // Copy / Move
                Hashmap<std::string, std::string, 8> tmp(map);
                map = std::move(tmp);
                break;
            }
            case 6: {  // Clear
                reference.clear();
                map.Clear();
                break;
            }
        }
    }
}

}  // namespace
}  // namespace tint::utils
