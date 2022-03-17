// Copyright 2022 The Dawn Authors
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <string>

#include "dawn/native/CacheKey.h"

namespace dawn::native {

    // Testing classes/structs with serializing implemented for testing.
    struct A {};
    template <>
    void CacheKeySerializer<A>::Serialize(CacheKey* key, const A& t) {
        std::string str = "structA";
        key->insert(key->end(), str.begin(), str.end());
    }

    class B {};
    template <>
    void CacheKeySerializer<B>::Serialize(CacheKey* key, const B& t) {
        std::string str = "classB";
        key->insert(key->end(), str.begin(), str.end());
    }

    namespace {

        // Matcher to compare CacheKey to a string for easier testing.
        MATCHER_P(CacheKeyEq,
                  key,
                  "cache key " + std::string(negation ? "not" : "") + "equal to " + key) {
            return std::string(arg.begin(), arg.end()) == key;
        }

        TEST(CacheKeyTest, IntegralTypes) {
            EXPECT_THAT(GetCacheKey((int)-1), CacheKeyEq("{0:-1}"));
            EXPECT_THAT(GetCacheKey((uint8_t)2), CacheKeyEq("{0:2}"));
            EXPECT_THAT(GetCacheKey((uint16_t)4), CacheKeyEq("{0:4}"));
            EXPECT_THAT(GetCacheKey((uint32_t)8), CacheKeyEq("{0:8}"));
            EXPECT_THAT(GetCacheKey((uint64_t)16), CacheKeyEq("{0:16}"));

            EXPECT_THAT(GetCacheKey((int)-1, (uint8_t)2, (uint16_t)4, (uint32_t)8, (uint64_t)16),
                        CacheKeyEq("{0:-1,1:2,2:4,3:8,4:16}"));
        }

        TEST(CacheKeyTest, FloatingTypes) {
            EXPECT_THAT(GetCacheKey((float)0.5), CacheKeyEq("{0:0.500000}"));
            EXPECT_THAT(GetCacheKey((double)32.0), CacheKeyEq("{0:32.000000}"));

            EXPECT_THAT(GetCacheKey((float)0.5, (double)32.0),
                        CacheKeyEq("{0:0.500000,1:32.000000}"));
        }

        TEST(CacheKeyTest, Strings) {
            std::string str0 = "string0";
            std::string str1 = "string1";

            EXPECT_THAT(GetCacheKey("string0"), CacheKeyEq(R"({0:7"string0"})"));
            EXPECT_THAT(GetCacheKey(str0), CacheKeyEq(R"({0:7"string0"})"));
            EXPECT_THAT(GetCacheKey("string0", str1), CacheKeyEq(R"({0:7"string0",1:7"string1"})"));
        }

        TEST(CacheKeyTest, NestedCacheKey) {
            EXPECT_THAT(GetCacheKey(GetCacheKey((int)-1)), CacheKeyEq("{0:{0:-1}}"));
            EXPECT_THAT(GetCacheKey(GetCacheKey("string")), CacheKeyEq(R"({0:{0:6"string"}})"));
            EXPECT_THAT(GetCacheKey(GetCacheKey(A{})), CacheKeyEq("{0:{0:structA}}"));
            EXPECT_THAT(GetCacheKey(GetCacheKey(B())), CacheKeyEq("{0:{0:classB}}"));

            EXPECT_THAT(GetCacheKey(GetCacheKey((int)-1), GetCacheKey("string"), GetCacheKey(A{}),
                                    GetCacheKey(B())),
                        CacheKeyEq(R"({0:{0:-1},1:{0:6"string"},2:{0:structA},3:{0:classB}})"));
        }

    }  // namespace

}  // namespace dawn::native
