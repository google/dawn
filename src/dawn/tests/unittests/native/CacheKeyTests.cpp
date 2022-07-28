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
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "dawn/common/TypedInteger.h"
#include "dawn/native/Blob.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/native/stream/Stream.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "tint/tint.h"

namespace dawn::native::stream {

// Testing classes with mock serializing implemented for testing.
class A {
  public:
    MOCK_METHOD(void, WriteMock, (stream::Sink*, const A&), (const));
};
template <>
void stream::Stream<A>::Write(stream::Sink* s, const A& t) {
    t.WriteMock(s, t);
}

struct Nested {
    A a1;
    A a2;
};
template <>
void stream::Stream<Nested>::Write(stream::Sink* s, const Nested& t) {
    StreamIn(s, t.a1);
    StreamIn(s, t.a2);
}

// Custom printer for ByteVectorSink for clearer debug testing messages.
void PrintTo(const ByteVectorSink& key, std::ostream* stream) {
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

using TypedIntegerForTest = TypedInteger<struct TypedIntegerForTestTag, uint32_t>;

// Matcher to compare ByteVectorSinks for easier testing.
MATCHER_P(VectorEq, key, PrintToString(key)) {
    return arg.size() == key.size() && memcmp(arg.data(), key.data(), key.size()) == 0;
}

#define EXPECT_CACHE_KEY_EQ(lhs, rhs)       \
    do {                                    \
        ByteVectorSink actual;              \
        StreamIn(&actual, lhs);             \
        EXPECT_THAT(actual, VectorEq(rhs)); \
    } while (0)

// Test that ByteVectorSink calls Write on a value.
TEST(SerializeTests, CallsWrite) {
    ByteVectorSink sink;

    A a;
    EXPECT_CALL(a, WriteMock(NotNull(), Ref(a))).Times(1);
    StreamIn(&sink, a);
}

// Test that ByteVectorSink calls Write on all elements of an iterable.
TEST(SerializeTests, StreamInIterable) {
    constexpr size_t kIterableSize = 100;

    std::vector<A> vec(kIterableSize);
    auto iterable = stream::Iterable(vec.data(), kIterableSize);

    // Expect write to be called for each element
    for (const auto& a : vec) {
        EXPECT_CALL(a, WriteMock(NotNull(), Ref(a))).Times(1);
    }

    ByteVectorSink sink;
    StreamIn(&sink, iterable);

    // Expecting the size of the container.
    ByteVectorSink expected;
    StreamIn(&expected, kIterableSize);
    EXPECT_THAT(sink, VectorEq(expected));
}

// Test that ByteVectorSink calls Write on all nested members of a struct.
TEST(SerializeTests, StreamInNested) {
    ByteVectorSink sink;

    Nested n;
    EXPECT_CALL(n.a1, WriteMock(NotNull(), Ref(n.a1))).Times(1);
    EXPECT_CALL(n.a2, WriteMock(NotNull(), Ref(n.a2))).Times(1);
    StreamIn(&sink, n);
}

// Test that ByteVectorSink serializes integral data as expected.
TEST(SerializeTests, IntegralTypes) {
    // Only testing explicitly sized types for simplicity, and using 0s for larger types to
    // avoid dealing with endianess.
    EXPECT_CACHE_KEY_EQ('c', ByteVectorSink({'c'}));
    EXPECT_CACHE_KEY_EQ(uint8_t(255), ByteVectorSink({255}));
    EXPECT_CACHE_KEY_EQ(uint16_t(0), ByteVectorSink({0, 0}));
    EXPECT_CACHE_KEY_EQ(uint32_t(0), ByteVectorSink({0, 0, 0, 0}));
}

// Test that ByteVectorSink serializes floating-point data as expected.
TEST(SerializeTests, FloatingTypes) {
    // Using 0s to avoid dealing with implementation specific float details.
    ByteVectorSink k1, k2;
    EXPECT_CACHE_KEY_EQ(float{0}, ByteVectorSink(sizeof(float), 0));
    EXPECT_CACHE_KEY_EQ(double{0}, ByteVectorSink(sizeof(double), 0));
}

// Test that ByteVectorSink serializes literal strings as expected.
TEST(SerializeTests, LiteralStrings) {
    // Using a std::string here to help with creating the expected result.
    std::string str = "string";

    ByteVectorSink expected;
    expected.insert(expected.end(), str.begin(), str.end());
    expected.push_back('\0');

    EXPECT_CACHE_KEY_EQ("string", expected);
}

// Test that ByteVectorSink serializes std::strings as expected.
TEST(SerializeTests, StdStrings) {
    std::string str = "string";

    ByteVectorSink expected;
    StreamIn(&expected, size_t(6));
    expected.insert(expected.end(), str.begin(), str.end());

    EXPECT_CACHE_KEY_EQ(str, expected);
}

// Test that ByteVectorSink serializes std::string_views as expected.
TEST(SerializeTests, StdStringViews) {
    static constexpr std::string_view str("string");

    ByteVectorSink expected;
    StreamIn(&expected, size_t(6));
    expected.insert(expected.end(), str.begin(), str.end());

    EXPECT_CACHE_KEY_EQ(str, expected);
}

// Test that ByteVectorSink serializes other ByteVectorSinks as expected.
TEST(SerializeTests, ByteVectorSinks) {
    ByteVectorSink data = {'d', 'a', 't', 'a'};

    ByteVectorSink expected;
    expected.insert(expected.end(), data.begin(), data.end());

    EXPECT_CACHE_KEY_EQ(data, expected);
}

// Test that ByteVectorSink serializes std::pair as expected.
TEST(SerializeTests, StdPair) {
    std::string_view s = "hi!";

    ByteVectorSink expected;
    StreamIn(&expected, s, uint32_t(42));

    EXPECT_CACHE_KEY_EQ(std::make_pair(s, uint32_t(42)), expected);
}

// Test that ByteVectorSink serializes std::unordered_map as expected.
TEST(SerializeTests, StdUnorderedMap) {
    std::unordered_map<uint32_t, std::string_view> m;

    m[4] = "hello";
    m[1] = "world";
    m[7] = "test";
    m[3] = "data";

    // Expect the number of entries, followed by (K, V) pairs sorted in order of key.
    ByteVectorSink expected;
    StreamIn(&expected, size_t(4), std::make_pair(uint32_t(1), m[1]),
             std::make_pair(uint32_t(3), m[3]), std::make_pair(uint32_t(4), m[4]),
             std::make_pair(uint32_t(7), m[7]));

    EXPECT_CACHE_KEY_EQ(m, expected);
}

// Test that ByteVectorSink serializes tint::sem::BindingPoint as expected.
TEST(SerializeTests, TintSemBindingPoint) {
    tint::sem::BindingPoint bp{3, 6};

    ByteVectorSink expected;
    StreamIn(&expected, uint32_t(3), uint32_t(6));

    EXPECT_CACHE_KEY_EQ(bp, expected);
}

// Test that ByteVectorSink serializes tint::transform::BindingPoints as expected.
TEST(SerializeTests, TintTransformBindingPoints) {
    tint::transform::BindingPoints points{
        tint::sem::BindingPoint{1, 4},
        tint::sem::BindingPoint{3, 7},
    };

    ByteVectorSink expected;
    StreamIn(&expected, uint32_t(1), uint32_t(4), uint32_t(3), uint32_t(7));

    EXPECT_CACHE_KEY_EQ(points, expected);
}

// Test that serializing then deserializing a param pack yields the same values.
TEST(StreamTests, SerializeDeserializeParamPack) {
    int a = 1;
    float b = 2.0;
    std::pair<std::string, double> c = std::make_pair("dawn", 3.4);

    ByteVectorSink sink;
    StreamIn(&sink, a, b, c);

    BlobSource source(CreateBlob(std::move(sink)));
    int aOut;
    float bOut;
    std::pair<std::string, double> cOut;
    auto err = StreamOut(&source, &aOut, &bOut, &cOut);
    if (err.IsError()) {
        FAIL() << err.AcquireError()->GetFormattedMessage();
    }
    EXPECT_EQ(a, aOut);
    EXPECT_EQ(b, bOut);
    EXPECT_EQ(c, cOut);
}

template <size_t N>
std::bitset<N - 1> BitsetFromBitString(const char (&str)[N]) {
    // N - 1 because the last character is the null terminator.
    return std::bitset<N - 1>(str, N - 1);
}

static auto kStreamValueVectorParams = std::make_tuple(
    // Test primitives.
    std::vector<int>{4, 5, 6, 2},
    std::vector<float>{6.50, 78.28, 92., 8.28},
    // Test various types of strings.
    std::vector<std::string>{"abcdefg", "9461849495", ""},
    // Test pairs.
    std::vector<std::pair<int, float>>{{1, 3.}, {6, 4.}},
    // Test TypedIntegers
    std::vector<TypedIntegerForTest>{TypedIntegerForTest(42), TypedIntegerForTest(13)},
    // Test enums
    std::vector<wgpu::TextureUsage>{wgpu::TextureUsage::CopyDst,
                                    wgpu::TextureUsage::RenderAttachment},
    // Test bitsets of various sizes.
    std::vector<std::bitset<7>>{0b1001011, 0b0011010, 0b0000000, 0b1111111},
    std::vector<std::bitset<17>>{0x0000, 0xFFFF1},
    std::vector<std::bitset<32>>{0x0C0FFEE0, 0xDEADC0DE, 0x00000000, 0xFFFFFFFF},
    std::vector<std::bitset<57>>{
        BitsetFromBitString("100110010101011001100110101011001100101010110011001011011"),
        BitsetFromBitString("000110010101011000100110101011001100101010010011001010100"),
        BitsetFromBitString("111111111111111111111111111111111111111111111111111111111"), 0},
    // Test vectors.
    std::vector<std::vector<int>>{{}, {1, 5, 2, 7, 4}, {3, 3, 3, 3, 3, 3, 3}});

static auto kStreamValueInitListParams = std::make_tuple(
    std::initializer_list<char[12]>{"test string", "string test"},
    std::initializer_list<double[3]>{{5.435, 32.3, 1.23}, {8.2345, 0.234532, 4.435}});

template <typename, typename>
struct StreamValueTestTypesImpl;

template <typename... T, typename... T2>
struct StreamValueTestTypesImpl<std::tuple<std::vector<T>...>,
                                std::tuple<std::initializer_list<T2>...>> {
    using type = ::testing::Types<T..., T2...>;
};

using StreamValueTestTypes =
    typename StreamValueTestTypesImpl<decltype(kStreamValueVectorParams),
                                      decltype(kStreamValueInitListParams)>::type;

template <typename T>
class StreamParameterizedTests : public ::testing::Test {
  protected:
    static std::vector<T> GetParams() { return std::get<std::vector<T>>(kStreamValueVectorParams); }

    void ExpectEq(const T& lhs, const T& rhs) { EXPECT_EQ(lhs, rhs); }
};

template <typename T, size_t N>
class StreamParameterizedTests<T[N]> : public ::testing::Test {
  protected:
    static std::initializer_list<T[N]> GetParams() {
        return std::get<std::initializer_list<T[N]>>(kStreamValueInitListParams);
    }

    void ExpectEq(const T lhs[N], const T rhs[N]) { EXPECT_EQ(memcmp(lhs, rhs, sizeof(T[N])), 0); }
};

TYPED_TEST_SUITE_P(StreamParameterizedTests);

// Test that serializing a value, then deserializing it yields the same value.
TYPED_TEST_P(StreamParameterizedTests, SerializeDeserialize) {
    for (const auto& value : this->GetParams()) {
        ByteVectorSink sink;
        StreamIn(&sink, value);

        BlobSource source(CreateBlob(std::move(sink)));
        TypeParam deserialized;
        auto err = StreamOut(&source, &deserialized);
        if (err.IsError()) {
            FAIL() << err.AcquireError()->GetFormattedMessage();
        }
        this->ExpectEq(deserialized, value);
    }
}

// Test that serializing a value, then deserializing it with insufficient space, an error is raised.
TYPED_TEST_P(StreamParameterizedTests, SerializeDeserializeOutOfBounds) {
    for (const auto& value : this->GetParams()) {
        ByteVectorSink sink;
        StreamIn(&sink, value);

        // Make the vector 1 byte too small.
        std::vector<uint8_t> src = sink;
        src.pop_back();

        BlobSource source(CreateBlob(std::move(src)));
        TypeParam deserialized;
        auto err = StreamOut(&source, &deserialized);
        EXPECT_TRUE(err.IsError());
        err.AcquireError();
    }
}

// Test that deserializing from an empty source raises an error.
TYPED_TEST_P(StreamParameterizedTests, DeserializeEmpty) {
    BlobSource source(CreateBlob(0));
    TypeParam deserialized;
    auto err = StreamOut(&source, &deserialized);
    EXPECT_TRUE(err.IsError());
    err.AcquireError();
}

REGISTER_TYPED_TEST_SUITE_P(StreamParameterizedTests,
                            SerializeDeserialize,
                            SerializeDeserializeOutOfBounds,
                            DeserializeEmpty);
INSTANTIATE_TYPED_TEST_SUITE_P(DawnUnittests, StreamParameterizedTests, StreamValueTestTypes, );

}  // namespace

}  // namespace dawn::native::stream
