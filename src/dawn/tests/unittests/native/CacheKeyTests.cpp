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
#include <unordered_map>
#include <utility>
#include <vector>

#include "dawn/native/CacheKey.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tint/tint.h"

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

// Test that CacheKey::Record calls serialize on the single member of a struct.
TEST(CacheKeyTests, RecordSingleMember) {
    CacheKey key;

    A a;
    EXPECT_CALL(a, SerializeMock(NotNull(), Ref(a))).Times(1);
    EXPECT_THAT(key.Record(a), CacheKeyEq(CacheKey()));
}

// Test that CacheKey::Record calls serialize on all members of a struct.
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

// Test that CacheKey::Record calls serialize on all elements of an iterable.
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

// Test that CacheKey::Record calls serialize on all members and nested struct members.
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

// Test that CacheKey::Record serializes integral data as expected.
TEST(CacheKeySerializerTests, IntegralTypes) {
    // Only testing explicitly sized types for simplicity, and using 0s for larger types to
    // avoid dealing with endianess.
    EXPECT_THAT(CacheKey().Record('c'), CacheKeyEq(CacheKey({'c'})));
    EXPECT_THAT(CacheKey().Record(uint8_t(255)), CacheKeyEq(CacheKey({255})));
    EXPECT_THAT(CacheKey().Record(uint16_t(0)), CacheKeyEq(CacheKey({0, 0})));
    EXPECT_THAT(CacheKey().Record(uint32_t(0)), CacheKeyEq(CacheKey({0, 0, 0, 0})));
}

// Test that CacheKey::Record serializes floating-point data as expected.
TEST(CacheKeySerializerTests, FloatingTypes) {
    // Using 0s to avoid dealing with implementation specific float details.
    EXPECT_THAT(CacheKey().Record(float{0}), CacheKeyEq(CacheKey(sizeof(float), 0)));
    EXPECT_THAT(CacheKey().Record(double{0}), CacheKeyEq(CacheKey(sizeof(double), 0)));
}

// Test that CacheKey::Record serializes literal strings as expected.
TEST(CacheKeySerializerTests, LiteralStrings) {
    // Using a std::string here to help with creating the expected result.
    std::string str = "string";

    CacheKey expected;
    expected.Record(size_t(7));
    expected.insert(expected.end(), str.begin(), str.end());
    expected.push_back('\0');

    EXPECT_THAT(CacheKey().Record("string"), CacheKeyEq(expected));
}

// Test that CacheKey::Record serializes std::strings as expected.
TEST(CacheKeySerializerTests, StdStrings) {
    std::string str = "string";

    CacheKey expected;
    expected.Record(size_t(6));
    expected.insert(expected.end(), str.begin(), str.end());

    EXPECT_THAT(CacheKey().Record(str), CacheKeyEq(expected));
}

// Test that CacheKey::Record serializes std::string_views as expected.
TEST(CacheKeySerializerTests, StdStringViews) {
    static constexpr std::string_view str("string");

    CacheKey expected;
    expected.Record(size_t(6));
    expected.insert(expected.end(), str.begin(), str.end());

    EXPECT_THAT(CacheKey().Record(str), CacheKeyEq(expected));
}

// Test that CacheKey::Record serializes other CacheKeys as expected.
TEST(CacheKeySerializerTests, CacheKeys) {
    CacheKey data = {'d', 'a', 't', 'a'};

    CacheKey expected;
    expected.insert(expected.end(), data.begin(), data.end());

    EXPECT_THAT(CacheKey().Record(data), CacheKeyEq(expected));
}

// Test that CacheKey::Record serializes std::pair as expected.
TEST(CacheKeySerializerTests, StdPair) {
    std::string_view s = "hi!";

    CacheKey expected;
    expected.Record(s);
    expected.Record(uint32_t(42));

    EXPECT_THAT(CacheKey().Record(std::make_pair(s, uint32_t(42))), CacheKeyEq(expected));
}

// Test that CacheKey::Record serializes std::unordered_map as expected.
TEST(CacheKeySerializerTests, StdUnorderedMap) {
    std::unordered_map<uint32_t, std::string_view> m;

    m[4] = "hello";
    m[1] = "world";
    m[7] = "test";
    m[3] = "data";

    // Expect the number of entries, followed by (K, V) pairs sorted in order of key.
    CacheKey expected;
    expected.Record(size_t(4));
    expected.Record(std::make_pair(uint32_t(1), m[1]));
    expected.Record(std::make_pair(uint32_t(3), m[3]));
    expected.Record(std::make_pair(uint32_t(4), m[4]));
    expected.Record(std::make_pair(uint32_t(7), m[7]));

    EXPECT_THAT(CacheKey().Record(m), CacheKeyEq(expected));
}

// Test that CacheKey::Record serializes tint::sem::BindingPoint as expected.
TEST(CacheKeySerializerTests, TintSemBindingPoint) {
    tint::sem::BindingPoint bp{3, 6};
    EXPECT_THAT(CacheKey().Record(bp), CacheKeyEq(CacheKey().Record(uint32_t(3), uint32_t(6))));
}

// Test that CacheKey::Record serializes tint::transform::BindingPoints as expected.
TEST(CacheKeySerializerTests, TintTransformBindingPoints) {
    tint::transform::BindingPoints points{
        tint::sem::BindingPoint{1, 4},
        tint::sem::BindingPoint{3, 7},
    };
    EXPECT_THAT(CacheKey().Record(points),
                CacheKeyEq(CacheKey().Record(uint32_t(1), uint32_t(4), uint32_t(3), uint32_t(7))));
}

}  // namespace

}  // namespace dawn::native
