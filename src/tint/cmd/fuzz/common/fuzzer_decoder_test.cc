// Copyright 2026 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/cmd/fuzz/common/fuzzer_decoder.h"

#include <gtest/gtest.h>

#include "src/tint/cmd/fuzz/common/fuzzer_encoder.h"

namespace tint::fuzz {
namespace {

enum class EnumWithRange {
    kZero = 0,
    kOne = 1,
    kTwo = 2,
};

enum class EnumWithNoRange {
    kZero = 0,
    kOne = 1,
};

}  // namespace
}  // namespace tint::fuzz

namespace tint {
TINT_REFLECT_ENUM_RANGE(tint::fuzz::EnumWithRange, kZero, kTwo);
}  // namespace tint

namespace tint::fuzz {
namespace {

struct InnerStruct {
    uint32_t x;
    bool b;

    TINT_REFLECT(InnerStruct, x, b);
};

struct OuterStruct {
    std::string s;
    std::vector<InnerStruct> vec;
    std::unordered_map<uint32_t, std::string> map;
    std::optional<uint32_t> opt;

    TINT_REFLECT(OuterStruct, s, vec, map, opt);
};

TEST(SafeFuzzerDecoderTest, EmptyInputDecodesToDefaults) {
    std::vector<std::byte> empty;
    SafeFuzzerDecoder decoder{empty};

    EXPECT_EQ(decoder.ConsumeIntegral<uint32_t>(), 0u);
    EXPECT_FALSE(decoder.ConsumeBool());
    EXPECT_EQ(decoder.ConsumeFloat<float>(), 0.0f);
    EXPECT_EQ(decoder.ConsumeCollectionLength(10), 0u);

    auto decoded_struct = FuzzDecoder<OuterStruct>::Decode(decoder);
    EXPECT_EQ(decoded_struct.s, "");
    EXPECT_TRUE(decoded_struct.vec.empty());
    EXPECT_TRUE(decoded_struct.map.empty());
    EXPECT_FALSE(decoded_struct.opt.has_value());
}

TEST(SafeFuzzerDecoderTest, SafeClamping) {
    std::vector<std::byte> data;
    data.push_back(std::byte{255});

    SafeFuzzerDecoder decoder{data};
    size_t len = decoder.ConsumeCollectionLength(8);
    EXPECT_EQ(len, 3u);  // 255 % 9
}

TEST(SafeFuzzerDecoderTest, RoundTripSymmetric) {
    OuterStruct original;
    original.s = std::string(kMaxStringLength, 'a');
    original.vec.resize(kMaxCollectionLength);
    for (size_t i = 0; i < kMaxCollectionLength; ++i) {
        original.vec[i] = InnerStruct{static_cast<uint32_t>(i), i % 2 == 0};
    }
    original.map = {{1u, "one"}, {2u, "two"}};
    original.opt = 100u;

    std::vector<std::byte> buffer;
    SafeFuzzerEncoder encoder{buffer};
    FuzzEncoder<OuterStruct>::Encode(encoder, original);

    SafeFuzzerDecoder decoder{buffer};
    auto decoded = FuzzDecoder<OuterStruct>::Decode(decoder);

    EXPECT_EQ(decoded.s, original.s);
    ASSERT_EQ(decoded.vec.size(), original.vec.size());
    for (size_t i = 0; i < kMaxCollectionLength; ++i) {
        EXPECT_EQ(decoded.vec[i].x, original.vec[i].x);
        EXPECT_EQ(decoded.vec[i].b, original.vec[i].b);
    }

    ASSERT_EQ(decoded.map.size(), original.map.size());
    EXPECT_EQ(decoded.map[1u], "one");
    EXPECT_EQ(decoded.map[2u], "two");

    ASSERT_TRUE(decoded.opt.has_value());
    EXPECT_EQ(*decoded.opt, 100u);
}

TEST(SafeFuzzerDecoderTest, PartialReadsAreZeroedAndConsumed) {
    std::vector<std::byte> data = {std::byte{0x12}, std::byte{0x34}};
    SafeFuzzerDecoder decoder{data};

    uint32_t val = decoder.ConsumeIntegral<uint32_t>();
    EXPECT_EQ(val, 0u);
    EXPECT_TRUE(decoder.IsEOF());
}

TEST(SafeFuzzerDecoderTest, ComplexTypesGracefulPartialReads) {
    std::vector<std::byte> data;

    data.push_back(std::byte{3});  // length prefix
    data.push_back(std::byte{'a'});
    data.push_back(std::byte{'b'});
    data.push_back(std::byte{'c'});

    data.push_back(std::byte{2});  // length prefix

    uint32_t x1 = 42;
    size_t offset = data.size();
    data.resize(offset + sizeof(uint32_t));
    tint::Copy(std::span{data}.subspan(offset, sizeof(uint32_t)), x1);
    data.push_back(std::byte{1});  // boolean true (1)

    data.push_back(std::byte{0x10});
    data.push_back(std::byte{0x20});

    SafeFuzzerDecoder decoder{data};
    auto decoded = FuzzDecoder<OuterStruct>::Decode(decoder);

    EXPECT_EQ(decoded.s, "abc");
    ASSERT_EQ(decoded.vec.size(), 2u);

    EXPECT_EQ(decoded.vec[0].x, 42u);
    EXPECT_TRUE(decoded.vec[0].b);

    EXPECT_EQ(decoded.vec[1].x, 0u);
    EXPECT_FALSE(decoded.vec[1].b);

    EXPECT_TRUE(decoded.map.empty());
    EXPECT_FALSE(decoded.opt.has_value());
    EXPECT_TRUE(decoder.IsEOF());
}

TEST(SafeFuzzerDecoderTest, DecodeEnumClamped) {
    std::vector<std::byte> data = {std::byte{42}, std::byte{0}, std::byte{0}, std::byte{0}};
    SafeFuzzerDecoder decoder{data};
    auto val = FuzzDecoder<EnumWithRange>::Decode(decoder);
    EXPECT_EQ(val, EnumWithRange::kZero);

    std::vector<std::byte> data2 = {std::byte{43}, std::byte{0}, std::byte{0}, std::byte{0}};
    SafeFuzzerDecoder decoder2{data2};
    auto val2 = FuzzDecoder<EnumWithRange>::Decode(decoder2);
    EXPECT_EQ(val2, EnumWithRange::kOne);
}

TEST(SafeFuzzerDecoderTest, DecodeEnumWithNoRange) {
    std::vector<std::byte> data = {std::byte{42}, std::byte{0}, std::byte{0}, std::byte{0}};
    SafeFuzzerDecoder decoder{data};
    auto val = FuzzDecoder<EnumWithNoRange>::Decode(decoder);
    EXPECT_EQ(static_cast<int>(val), 42);
}

TEST(SafeFuzzerDecoderTest, UnorderedSetDeterministicEncoding) {
    std::unordered_set<size_t> set_a;
    std::unordered_set<size_t> set_b;
    // Seed with enough elements to guarantee that the encoder will only encode part of the
    // container.
    for (size_t i = 0; i < 2 * kMaxCollectionLength; ++i) {
        set_a.insert(i);
        set_b.insert((2 * kMaxCollectionLength - 1) - i);
    }

    std::vector<std::byte> buffer_a;
    SafeFuzzerEncoder encoder_a{buffer_a};
    FuzzEncoder<std::unordered_set<size_t>>::Encode(encoder_a, set_a);

    std::vector<std::byte> buffer_b;
    SafeFuzzerEncoder encoder_b{buffer_b};
    FuzzEncoder<std::unordered_set<size_t>>::Encode(encoder_b, set_b);

    EXPECT_EQ(buffer_a, buffer_b);
}

TEST(SafeFuzzerDecoderTest, UnorderedMapDeterministicEncoding) {
    std::unordered_map<size_t, size_t> map_a;
    std::unordered_map<size_t, size_t> map_b;
    // Seed with enough elements to guarantee that the encoder will only encode part of the
    // container.
    for (size_t i = 0; i < 2 * kMaxCollectionLength; ++i) {
        map_a[i] = i * 2;
        map_b[(2 * kMaxCollectionLength - 1) - i] = ((2 * kMaxCollectionLength - 1) - i) * 2;
    }

    std::vector<std::byte> buffer_a;
    SafeFuzzerEncoder encoder_a{buffer_a};
    FuzzEncoder<std::unordered_map<size_t, size_t>>::Encode(encoder_a, map_a);

    std::vector<std::byte> buffer_b;
    SafeFuzzerEncoder encoder_b{buffer_b};
    FuzzEncoder<std::unordered_map<size_t, size_t>>::Encode(encoder_b, map_b);

    EXPECT_EQ(buffer_a, buffer_b);
}

}  // namespace
}  // namespace tint::fuzz
