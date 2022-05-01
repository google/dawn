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

#include <cstring>
#include <iomanip>
#include <string>
#include <vector>

#include "dawn/native/CacheKey.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace dawn::native {

// Testing classes with mock serializing implemented for testing.
class A {
  public:
    MOCK_METHOD(void, SerializeMock, (CacheKey*, const A&), (const));
};
template <>
void CacheKeySerializer<A>::Serialize(CacheKey* key, const A& t) {
    t.SerializeMock(key, t);
}

// Custom printer for CacheKey for clearer debug testing messages.
void PrintTo(const CacheKey& key, std::ostream* stream) {
    *stream << std::hex;
    for (const int b : key) {
        *stream << std::setfill('0') << std::setw(2) << b << " ";
    }
    *stream << std::dec;
}

namespace {

using ::testing::InSequence;
using ::testing::NotNull;
using ::testing::PrintToString;
using ::testing::Ref;

// Matcher to compare CacheKeys for easier testing.
MATCHER_P(CacheKeyEq, key, PrintToString(key)) {
    return arg.size() == key.size() && memcmp(arg.data(), key.data(), key.size()) == 0;
}

TEST(CacheKeyTests, RecordSingleMember) {
    CacheKey key;

    A a;
    EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
    EXPECT_THAT(key.Record(a), CacheKeyEq(CacheKey()));
}

TEST(CacheKeyTests, RecordManyMembers) {
    constexpr size_t kNumMembers = 100;

    CacheKey key;
    for (size_t i = 0; i < kNumMembers; ++i) {
        A a;
        EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
        key.Record(a);
    }
    EXPECT_THAT(key, CacheKeyEq(CacheKey()));
}

TEST(CacheKeyTests, RecordIterable) {
    constexpr size_t kIterableSize = 100;

    // Expecting the size of the container.
    CacheKey expected;
    expected.Record(kIterableSize);

    std::vector<A> iterable(kIterableSize);
    {
        InSequence seq;
        for (const auto& a : iterable) {
            EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
        }
        for (const auto& a : iterable) {
            EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
        }
    }

    EXPECT_THAT(CacheKey().RecordIterable(iterable), CacheKeyEq(expected));
    EXPECT_THAT(CacheKey().RecordIterable(iterable.data(), kIterableSize), CacheKeyEq(expected));
}

TEST(CacheKeyTests, RecordNested) {
    CacheKey expected;
    CacheKey actual;
    {
        // Recording a single member.
        A a;
        EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
        actual.Record(CacheKey().Record(a));
    }
    {
        // Recording multiple members.
        constexpr size_t kNumMembers = 2;
        CacheKey sub;
        for (size_t i = 0; i < kNumMembers; ++i) {
            A a;
            EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
            sub.Record(a);
        }
        actual.Record(sub);
    }
    {
        // Record an iterable.
        constexpr size_t kIterableSize = 2;
        expected.Record(kIterableSize);
        std::vector<A> iterable(kIterableSize);
        {
            InSequence seq;
            for (const auto& a : iterable) {
                EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
            }
        }
        actual.Record(CacheKey().RecordIterable(iterable));
    }
    EXPECT_THAT(actual, CacheKeyEq(expected));
}

TEST(CacheKeySerializerTests, IntegralTypes) {
    // Only testing explicitly sized types for simplicity, and using 0s for larger types to
    // avoid dealing with endianess.
    EXPECT_THAT(CacheKey().Record('c'), CacheKeyEq(CacheKey({'c'})));
    EXPECT_THAT(CacheKey().Record(uint8_t(255)), CacheKeyEq(CacheKey({255})));
    EXPECT_THAT(CacheKey().Record(uint16_t(0)), CacheKeyEq(CacheKey({0, 0})));
    EXPECT_THAT(CacheKey().Record(uint32_t(0)), CacheKeyEq(CacheKey({0, 0, 0, 0})));
}

TEST(CacheKeySerializerTests, FloatingTypes) {
    // Using 0s to avoid dealing with implementation specific float details.
    EXPECT_THAT(CacheKey().Record(float{0}), CacheKeyEq(CacheKey(sizeof(float), 0)));
    EXPECT_THAT(CacheKey().Record(double{0}), CacheKeyEq(CacheKey(sizeof(double), 0)));
}

TEST(CacheKeySerializerTests, LiteralStrings) {
    // Using a std::string here to help with creating the expected result.
    std::string str = "string";

    CacheKey expected;
    expected.Record(size_t(7));
    expected.insert(expected.end(), str.begin(), str.end());
    expected.push_back('\0');

    EXPECT_THAT(CacheKey().Record("string"), CacheKeyEq(expected));
}

TEST(CacheKeySerializerTests, StdStrings) {
    std::string str = "string";

    CacheKey expected;
    expected.Record((size_t)6);
    expected.insert(expected.end(), str.begin(), str.end());

    EXPECT_THAT(CacheKey().Record(str), CacheKeyEq(expected));
}

TEST(CacheKeySerializerTests, CacheKeys) {
    CacheKey data = {'d', 'a', 't', 'a'};

    CacheKey expected;
    expected.insert(expected.end(), data.begin(), data.end());

    EXPECT_THAT(CacheKey().Record(data), CacheKeyEq(expected));
}

}  // namespace

}  // namespace dawn::native
