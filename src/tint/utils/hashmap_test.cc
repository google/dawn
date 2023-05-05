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

TEST(Hashmap, Generation) {
    Hashmap<int, std::string, 8> map;
    EXPECT_EQ(map.Generation(), 0u);
    map.Add(1, "one");
    EXPECT_EQ(map.Generation(), 1u);
    map.Add(1, "uno");
    EXPECT_EQ(map.Generation(), 1u);
    map.Replace(1, "une");
    EXPECT_EQ(map.Generation(), 2u);
    map.Add(2, "dos");
    EXPECT_EQ(map.Generation(), 3u);
    map.Remove(1);
    EXPECT_EQ(map.Generation(), 4u);
    map.Clear();
    EXPECT_EQ(map.Generation(), 5u);
    map.Find(2);
    EXPECT_EQ(map.Generation(), 5u);
    map.Get(2);
    EXPECT_EQ(map.Generation(), 5u);
}

TEST(Hashmap, Index) {
    Hashmap<int, std::string, 4> map;
    auto zero = map.Find(0);
    EXPECT_FALSE(zero);

    map.Add(3, "three");
    auto three = map.Find(3);
    map.Add(2, "two");
    auto two = map.Find(2);
    map.Add(4, "four");
    auto four = map.Find(4);
    map.Add(8, "eight");
    auto eight = map.Find(8);

    EXPECT_FALSE(zero);
    ASSERT_TRUE(three);
    ASSERT_TRUE(two);
    ASSERT_TRUE(four);
    ASSERT_TRUE(eight);

    EXPECT_EQ(*three, "three");
    EXPECT_EQ(*two, "two");
    EXPECT_EQ(*four, "four");
    EXPECT_EQ(*eight, "eight");

    map.Add(0, "zero");  // Note: Find called before Add() is okay!

    map.Add(5, "five");
    auto five = map.Find(5);
    map.Add(6, "six");
    auto six = map.Find(6);
    map.Add(1, "one");
    auto one = map.Find(1);
    map.Add(7, "seven");
    auto seven = map.Find(7);

    ASSERT_TRUE(zero);
    ASSERT_TRUE(three);
    ASSERT_TRUE(two);
    ASSERT_TRUE(four);
    ASSERT_TRUE(eight);
    ASSERT_TRUE(five);
    ASSERT_TRUE(six);
    ASSERT_TRUE(one);
    ASSERT_TRUE(seven);

    EXPECT_EQ(*zero, "zero");
    EXPECT_EQ(*three, "three");
    EXPECT_EQ(*two, "two");
    EXPECT_EQ(*four, "four");
    EXPECT_EQ(*eight, "eight");
    EXPECT_EQ(*five, "five");
    EXPECT_EQ(*six, "six");
    EXPECT_EQ(*one, "one");
    EXPECT_EQ(*seven, "seven");

    map.Remove(2);
    map.Remove(8);
    map.Remove(1);

    EXPECT_FALSE(two);
    EXPECT_FALSE(eight);
    EXPECT_FALSE(one);
}

TEST(Hashmap, StringKeys) {
    Hashmap<std::string, int, 4> map;
    EXPECT_FALSE(map.Find("zero"));
    EXPECT_FALSE(map.Find(std::string("zero")));
    EXPECT_FALSE(map.Find(std::string_view("zero")));

    map.Add("three", 3);
    auto three_cstr = map.Find("three");
    auto three_str = map.Find(std::string("three"));
    auto three_sv = map.Find(std::string_view("three"));
    map.Add(std::string("two"), 2);
    auto two_cstr = map.Find("two");
    auto two_str = map.Find(std::string("two"));
    auto two_sv = map.Find(std::string_view("two"));
    map.Add("four", 4);
    auto four_cstr = map.Find("four");
    auto four_str = map.Find(std::string("four"));
    auto four_sv = map.Find(std::string_view("four"));
    map.Add(std::string("eight"), 8);
    auto eight_cstr = map.Find("eight");
    auto eight_str = map.Find(std::string("eight"));
    auto eight_sv = map.Find(std::string_view("eight"));

    ASSERT_TRUE(three_cstr);
    ASSERT_TRUE(three_str);
    ASSERT_TRUE(three_sv);
    ASSERT_TRUE(two_cstr);
    ASSERT_TRUE(two_str);
    ASSERT_TRUE(two_sv);
    ASSERT_TRUE(four_cstr);
    ASSERT_TRUE(four_str);
    ASSERT_TRUE(four_sv);
    ASSERT_TRUE(eight_cstr);
    ASSERT_TRUE(eight_str);
    ASSERT_TRUE(eight_sv);

    EXPECT_EQ(*three_cstr, 3);
    EXPECT_EQ(*three_str, 3);
    EXPECT_EQ(*three_sv, 3);
    EXPECT_EQ(*two_cstr, 2);
    EXPECT_EQ(*two_str, 2);
    EXPECT_EQ(*two_sv, 2);
    EXPECT_EQ(*four_cstr, 4);
    EXPECT_EQ(*four_str, 4);
    EXPECT_EQ(*four_sv, 4);
    EXPECT_EQ(*eight_cstr, 8);
    EXPECT_EQ(*eight_str, 8);
    EXPECT_EQ(*eight_sv, 8);

    map.Add("zero", 0);  // Note: Find called before Add() is okay!
    auto zero_cstr = map.Find("zero");
    auto zero_str = map.Find(std::string("zero"));
    auto zero_sv = map.Find(std::string_view("zero"));

    map.Add(std::string("five"), 5);
    auto five_cstr = map.Find("five");
    auto five_str = map.Find(std::string("five"));
    auto five_sv = map.Find(std::string_view("five"));
    map.Add("six", 6);
    auto six_cstr = map.Find("six");
    auto six_str = map.Find(std::string("six"));
    auto six_sv = map.Find(std::string_view("six"));
    map.Add("one", 1);
    auto one_cstr = map.Find("one");
    auto one_str = map.Find(std::string("one"));
    auto one_sv = map.Find(std::string_view("one"));
    map.Add(std::string("seven"), 7);
    auto seven_cstr = map.Find("seven");
    auto seven_str = map.Find(std::string("seven"));
    auto seven_sv = map.Find(std::string_view("seven"));

    ASSERT_TRUE(zero_cstr);
    ASSERT_TRUE(zero_str);
    ASSERT_TRUE(zero_sv);
    ASSERT_TRUE(three_cstr);
    ASSERT_TRUE(three_str);
    ASSERT_TRUE(three_sv);
    ASSERT_TRUE(two_cstr);
    ASSERT_TRUE(two_str);
    ASSERT_TRUE(two_sv);
    ASSERT_TRUE(four_cstr);
    ASSERT_TRUE(four_str);
    ASSERT_TRUE(four_sv);
    ASSERT_TRUE(eight_cstr);
    ASSERT_TRUE(eight_str);
    ASSERT_TRUE(eight_sv);
    ASSERT_TRUE(five_cstr);
    ASSERT_TRUE(five_str);
    ASSERT_TRUE(five_sv);
    ASSERT_TRUE(six_cstr);
    ASSERT_TRUE(six_str);
    ASSERT_TRUE(six_sv);
    ASSERT_TRUE(one_cstr);
    ASSERT_TRUE(one_str);
    ASSERT_TRUE(one_sv);
    ASSERT_TRUE(seven_cstr);
    ASSERT_TRUE(seven_str);
    ASSERT_TRUE(seven_sv);

    EXPECT_EQ(*zero_cstr, 0);
    EXPECT_EQ(*zero_str, 0);
    EXPECT_EQ(*zero_sv, 0);
    EXPECT_EQ(*three_cstr, 3);
    EXPECT_EQ(*three_str, 3);
    EXPECT_EQ(*three_sv, 3);
    EXPECT_EQ(*two_cstr, 2);
    EXPECT_EQ(*two_str, 2);
    EXPECT_EQ(*two_sv, 2);
    EXPECT_EQ(*four_cstr, 4);
    EXPECT_EQ(*four_str, 4);
    EXPECT_EQ(*four_sv, 4);
    EXPECT_EQ(*eight_cstr, 8);
    EXPECT_EQ(*eight_str, 8);
    EXPECT_EQ(*eight_sv, 8);
    EXPECT_EQ(*five_cstr, 5);
    EXPECT_EQ(*five_str, 5);
    EXPECT_EQ(*five_sv, 5);
    EXPECT_EQ(*six_cstr, 6);
    EXPECT_EQ(*six_str, 6);
    EXPECT_EQ(*six_sv, 6);
    EXPECT_EQ(*one_cstr, 1);
    EXPECT_EQ(*one_str, 1);
    EXPECT_EQ(*one_sv, 1);
    EXPECT_EQ(*seven_cstr, 7);
    EXPECT_EQ(*seven_str, 7);
    EXPECT_EQ(*seven_sv, 7);
}

TEST(Hashmap, Iterator) {
    using Map = Hashmap<int, std::string, 8>;
    using Entry = typename Map::Entry;
    Map map;
    map.Add(1, "one");
    map.Add(4, "four");
    map.Add(3, "three");
    map.Add(2, "two");
    EXPECT_THAT(map, testing::UnorderedElementsAre(Entry{1, "one"}, Entry{2, "two"},
                                                   Entry{3, "three"}, Entry{4, "four"}));
}

TEST(Hashmap, MutableIterator) {
    using Map = Hashmap<int, std::string, 8>;
    using Entry = typename Map::Entry;
    Map map;
    map.Add(1, "one");
    map.Add(4, "four");
    map.Add(3, "three");
    map.Add(2, "two");
    for (auto pair : map) {
        pair.value += "!";
    }
    EXPECT_THAT(map, testing::UnorderedElementsAre(Entry{1, "one!"}, Entry{2, "two!"},
                                                   Entry{3, "three!"}, Entry{4, "four!"}));
}

TEST(Hashmap, KeysValues) {
    using Map = Hashmap<int, std::string, 8>;
    Map map;
    map.Add(1, "one");
    map.Add(4, "four");
    map.Add(3, "three");
    map.Add(2, "two");
    EXPECT_THAT(map.Keys(), testing::UnorderedElementsAre(1, 2, 3, 4));
    EXPECT_THAT(map.Values(), testing::UnorderedElementsAre("one", "two", "three", "four"));
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
    std::optional<std::string> value_of_key_0_at_create;
    EXPECT_EQ(map.GetOrCreate(0,
                              [&] {
                                  value_of_key_0_at_create = map.Get(0);
                                  return "zero";
                              }),
              "zero");
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_EQ(map.Get(0), "zero");
    EXPECT_EQ(value_of_key_0_at_create, "");

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

TEST(Hashmap, GetOrCreate_CreateModifiesMap) {
    Hashmap<int, std::string, 8> map;
    EXPECT_EQ(map.GetOrCreate(0,
                              [&] {
                                  map.Add(3, "three");
                                  map.Add(1, "one");
                                  map.Add(2, "two");
                                  return "zero";
                              }),
              "zero");
    EXPECT_EQ(map.Count(), 4u);
    EXPECT_EQ(map.Get(0), "zero");
    EXPECT_EQ(map.Get(1), "one");
    EXPECT_EQ(map.Get(2), "two");
    EXPECT_EQ(map.Get(3), "three");

    bool create_called = false;
    EXPECT_EQ(map.GetOrCreate(0,
                              [&] {
                                  create_called = true;
                                  return "oh noes";
                              }),
              "zero");
    EXPECT_FALSE(create_called);
    EXPECT_EQ(map.Count(), 4u);
    EXPECT_EQ(map.Get(0), "zero");
    EXPECT_EQ(map.Get(1), "one");
    EXPECT_EQ(map.Get(2), "two");
    EXPECT_EQ(map.Get(3), "three");

    EXPECT_EQ(map.GetOrCreate(4,
                              [&] {
                                  map.Add(6, "six");
                                  map.Add(5, "five");
                                  map.Add(7, "seven");
                                  return "four";
                              }),
              "four");
    EXPECT_EQ(map.Count(), 8u);
    EXPECT_EQ(map.Get(0), "zero");
    EXPECT_EQ(map.Get(1), "one");
    EXPECT_EQ(map.Get(2), "two");
    EXPECT_EQ(map.Get(3), "three");
    EXPECT_EQ(map.Get(4), "four");
    EXPECT_EQ(map.Get(5), "five");
    EXPECT_EQ(map.Get(6), "six");
    EXPECT_EQ(map.Get(7), "seven");
}

TEST(Hashmap, GetOrCreate_CreateAddsSameKeyedValue) {
    Hashmap<int, std::string, 8> map;
    EXPECT_EQ(map.GetOrCreate(42,
                              [&] {
                                  map.Add(42, "should-be-replaced");
                                  return "expected-value";
                              }),
              "expected-value");
    EXPECT_EQ(map.Count(), 1u);
    EXPECT_EQ(map.Get(42), "expected-value");
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

TEST(Hashmap, EqualitySameSize) {
    Hashmap<int, std::string, 8> a;
    Hashmap<int, std::string, 8> b;
    EXPECT_EQ(a, b);
    a.Add(1, "one");
    EXPECT_NE(a, b);
    b.Add(2, "two");
    EXPECT_NE(a, b);
    a.Add(2, "two");
    EXPECT_NE(a, b);
    b.Add(1, "one");
    EXPECT_EQ(a, b);
}

TEST(Hashmap, EqualityDifferentSize) {
    Hashmap<int, std::string, 8> a;
    Hashmap<int, std::string, 4> b;
    EXPECT_EQ(a, b);
    a.Add(1, "one");
    EXPECT_NE(a, b);
    b.Add(2, "two");
    EXPECT_NE(a, b);
    a.Add(2, "two");
    EXPECT_NE(a, b);
    b.Add(1, "one");
    EXPECT_EQ(a, b);
}

TEST(Hashmap, HashSameSize) {
    Hashmap<int, std::string, 8> a;
    Hashmap<int, std::string, 8> b;
    EXPECT_EQ(Hash(a), Hash(b));
    a.Add(1, "one");
    b.Add(2, "two");
    a.Add(2, "two");
    b.Add(1, "one");
    EXPECT_EQ(Hash(a), Hash(b));
}

TEST(Hashmap, HashDifferentSize) {
    Hashmap<int, std::string, 8> a;
    Hashmap<int, std::string, 4> b;
    EXPECT_EQ(Hash(a), Hash(b));
    a.Add(1, "one");
    b.Add(2, "two");
    a.Add(2, "two");
    b.Add(1, "one");
    EXPECT_EQ(Hash(a), Hash(b));
}

}  // namespace
}  // namespace tint::utils
