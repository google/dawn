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

#include <algorithm>
#include <array>
#include <string>
#include <vector>

#include "gmock/gmock.h"
#include "partition_alloc/pointers/raw_ptr.h"
#include "src/utils/gtest.h"
#include "src/utils/span.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

using testing::ElementsAreArray;

static constexpr std::array<int, 5> kSpanData = {1, 2, 3, 4, 5};

using Index = TypedInteger<struct IndexT, uint32_t>;
using Index8 = TypedInteger<struct IndexT, uint8_t>;
using Index64 = TypedInteger<struct IndexT, uint64_t>;

struct FakeRange {
    size_t size() const { return kSpanData.size(); }
    const int* data() const { return kSpanData.data(); }
    auto begin() { return kSpanData.begin(); }
    auto end() { return kSpanData.end(); }
};

struct FakeTypedRange {
    Index size() const {
        return Index{uint32_t{kSpanData.size()}};
    }
    const int* data() const { return kSpanData.data(); }
    auto begin() { return kSpanData.begin(); }
    auto end() { return kSpanData.end(); }
};

struct FakeTyped64Range {
    Index64 size() const {
        return Index64{uint64_t{kSpanData.size()}};
    }
    const int* data() const { return kSpanData.data(); }
    auto begin() { return kSpanData.begin(); }
    auto end() { return kSpanData.end(); }
};

struct SpanTypes {
    template <typename T, size_t Extent = detail::DynamicExtent<size_t>>
    using Span = dawn::Span<T, Extent>;

    template <typename Index, typename T, Index Extent = detail::DynamicExtent<Index>>
    using ItypSpan = dawn::ityp::span<Index, T, Extent>;
};

struct RawSpanTypes {
    template <typename T, size_t Extent = detail::DynamicExtent<size_t>>
    using Span = dawn::RawSpan<T, Extent>;

    template <typename Index, typename T, Index Extent = detail::DynamicExtent<Index>>
    using ItypSpan = dawn::ityp::raw_span<Index, T, Extent>;
};

template <typename T>
class SpanTest : public ::testing::Test {};

template <typename T>
class SpanDeathTest : public ::testing::Test {};

struct SpanTestTypeNames {
    template <typename T>
    static std::string GetName(int) {
        if constexpr (std::is_same_v<T, SpanTypes>) {
            return "PointerSpan";
        } else if constexpr (std::is_same_v<T, RawSpanTypes>) {
            return "RawPointerSpan";
        }
    }
};

using SpanTestTypes = ::testing::Types<SpanTypes, RawSpanTypes>;
TYPED_TEST_SUITE(SpanTest, SpanTestTypes, SpanTestTypeNames);
TYPED_TEST_SUITE(SpanDeathTest, SpanTestTypes, SpanTestTypeNames);

#define SPAN typename TypeParam::template Span
#define ITYP_SPAN typename TypeParam::template ItypSpan

TYPED_TEST(SpanTest, Constructor_Default) {
    {
        SPAN<int> sp;
        EXPECT_EQ(sp.size(), 0u);
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        SPAN<int, 0> sp;
        EXPECT_EQ(sp.size(), 0u);
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        SPAN<int, 5> sp;
        EXPECT_EQ(sp.size(), 0u);
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);

        SPAN<const int, 5> sp_const = sp;
        EXPECT_EQ(sp_const.size(), 0u);
        EXPECT_EQ(sp_const.data(), nullptr);
        EXPECT_TRUE(sp_const.empty());
        EXPECT_EQ(sp_const.size_bytes(), 0u);
    }
    {
        ITYP_SPAN<Index, int> sp;
        EXPECT_EQ(sp.size(), Index{0u});
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        ITYP_SPAN<Index, int, Index{0u}> sp;
        EXPECT_EQ(sp.size(), Index{0u});
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);
    }
    {
        ITYP_SPAN<Index, int, Index{5u}> sp;
        EXPECT_EQ(sp.size(), Index{0u});
        EXPECT_EQ(sp.data(), nullptr);
        EXPECT_TRUE(sp.empty());
        EXPECT_EQ(sp.size_bytes(), 0u);

        ITYP_SPAN<Index, const int, Index{5u}> sp_const = sp;
        EXPECT_EQ(sp_const.size(), Index{0u});
        EXPECT_EQ(sp_const.data(), nullptr);
        EXPECT_TRUE(sp_const.empty());
        EXPECT_EQ(sp_const.size_bytes(), 0u);
    }
}

TYPED_TEST(SpanDeathTest, Constructor_DefaultFixedSpan) {
    {
        SPAN<int, 5> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp[0], "");
        EXPECT_DEATH_IF_SUPPORTED(sp.at(0), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.begin(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.first(1), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.last(1), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(1), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(0, 1), "");
    }
    {
        ITYP_SPAN<Index, int, Index{5u}> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp[Index{0u}], "");
        EXPECT_DEATH_IF_SUPPORTED(sp.at(Index{0u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.begin(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.first(Index{1u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.last(Index{1u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(Index{1u}), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(Index{0u}, Index{1u}), "");
    }
}

TYPED_TEST(SpanTest, Constructor_PointerAndSize) {
    int data[] = {1, 2, 3};
    const int constData[] = {1, 2, 3};
    raw_ptr<int> rptr = data;

    // T* + size for Span<T>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<int> DAWN_UNSAFE_BUFFERS(sp{&data[0], 3});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }
    // const T* + size for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int> DAWN_UNSAFE_BUFFERS(sp{&constData[0], 3});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData);
    }
    // T* + size for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int> DAWN_UNSAFE_BUFFERS(sp{&data[0], 3});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }

    // T* for Span<T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<int, 3> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }
    // const T* for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int, 3> DAWN_UNSAFE_BUFFERS(sp{&constData[0]});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData);
    }
    // T* for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int, 3> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
    }

    // T* + size for ityp::span<T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, int> DAWN_UNSAFE_BUFFERS(sp{&data[0], Index{3u}});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }
    // const T* + size for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int> DAWN_UNSAFE_BUFFERS(sp{&constData[0], Index{3u}});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData);
    }
    // T* + size for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int> DAWN_UNSAFE_BUFFERS(sp{&data[0], Index{3u}});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }

    // T* for ityp::span<T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }
    // const T* for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{&constData[0]});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData);
    }
    // T* for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{&data[0]});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data);
    }
}

TYPED_TEST(SpanDeathTest, Constructor_PointerAndSizeOversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    constexpr Index64 kHugeSize{0x1'0000'0000LLU};
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED(
        (ITYP_SPAN<Index64, const int>(kSpanData.data(), kHugeSize)), ""));
}

TYPED_TEST(SpanDeathTest, Constructor_PointerAndSizeDynamicExtent) {
    // DynamicExtent is invalid because it's reserved as a sentinel value.
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED(
        (ITYP_SPAN<Index8, const int>(kSpanData.data(), detail::DynamicExtent<Index8>)), ""));
}

TYPED_TEST(SpanTest, Constructor_TwoIterators) {
    std::array<int, 3> data = {1, 2, 3};
    const std::array<int, 3> constData = {1, 2, 3};

    // 2 x iterator for Span<T>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int> DAWN_UNSAFE_BUFFERS(sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for Span<const T>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }

    // 2 x iterator for Span<T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<int, 3> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int, 3> DAWN_UNSAFE_BUFFERS(sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for Span<const T, N>
    {
        // SAFETY: Test for the unsafe constructor.
        SPAN<const int, 3> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data.data());
    }

    // 2 x iterator for ityp::span<T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int> DAWN_UNSAFE_BUFFERS(sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for ityp::span<const T, ...>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }

    // 2 x iterator for ityp::span<T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }
    // 2 x const_iterator for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(
            sp{constData.begin(), constData.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), constData.data());
    }
    // 2 x iterator for ityp::span<const T, ..., N>
    {
        // SAFETY: Test for the unsafe constructor.
        ITYP_SPAN<Index, const int, Index{3u}> DAWN_UNSAFE_BUFFERS(sp{data.begin(), data.end()});
        EXPECT_EQ(sp.size(), Index{3u});
        EXPECT_EQ(sp.data(), data.data());
    }
}

TYPED_TEST(SpanDeathTest, Constructor_TwoIteratorsInverted) {
    std::array<int, 3> data = {1, 2, 3};
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED((SPAN<int>{data.end(), data.begin()}), ""));
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(EXPECT_DEATH_IF_SUPPORTED((SPAN<int, 3>{data.end(), data.begin()}), ""));
}

TYPED_TEST(SpanDeathTest, Constructor_TwoIteratorsLargerThanIndexType) {
    std::vector<int> data;
    data.resize(256);  // Larger than indices that can be store in a uint8_t.

    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(
        EXPECT_DEATH_IF_SUPPORTED((ITYP_SPAN<Index8, int>(data.begin(), data.end())), ""));

    // 255 is invalid because it's reserved as a sentinel value.
    data.resize(255);
    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS(
        EXPECT_DEATH_IF_SUPPORTED((ITYP_SPAN<Index8, int>(data.begin(), data.end())), ""));

    // Fits in uint8_t for indexing without matching DynamicExtent.
    data.resize(254);

    // SAFETY: Test for the unsafe constructor.
    DAWN_UNSAFE_BUFFERS((ITYP_SPAN<Index8, int>(data.begin(), data.end())));
}

std::span<std::byte> GetByteSpan() {
    return {};
}

TYPED_TEST(SpanTest, Constructor_CompatibleRange) {
    {
        std::array<int, 3> data = {1, 2, 3};
        SPAN<int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        const std::array<int, 3> data = {1, 2, 3};
        SPAN<const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        std::array<int, 3> data = {1, 2, 3};
        SPAN<int, 3> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        const std::array<int, 3> data = {1, 2, 3};
        SPAN<const int, 3> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        int data[] = {1, 2, 3};
        SPAN<int, 3> sp{data};
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
        if constexpr (std::is_same_v<TypeParam, SpanTypes>) {
            static_assert(sizeof(sp) == sizeof(SPAN<int, 3>::pointer));
        }
    }
    {
        const int data[] = {1, 2, 3};
        SPAN<const int, 3> sp{data};
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), data);
        if constexpr (std::is_same_v<TypeParam, SpanTypes>) {
            static_assert(sizeof(sp) == sizeof(SPAN<const int, 3>::pointer));
        }
    }
    {
        std::vector<int> data{{1, 2, 3}};
        SPAN<const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        std::string data = "foo";
        SPAN<char> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        std::string_view data = "foo";
        SPAN<const char> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        FakeRange data;
        SPAN<const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
    {
        // Fails to compile if the constructor from a range is written to take a reference
        // (R& range) and not an rvalue reference (R&& range).
        SPAN<std::byte> sp;
        sp = GetByteSpan();
    }

    {
        FakeTypedRange data;
        ITYP_SPAN<Index, const int> sp{data};
        EXPECT_EQ(sp.size(), data.size());
        EXPECT_EQ(sp.data(), data.data());
    }
}

TYPED_TEST(SpanTest, Constructor_DynamicToFixed) {
    // Span<T> -> Span<T, N>
    {
        std::array<int, 3> data = {1, 2, 3};
        SPAN<int> sp(data);
        SPAN<int, 3> fixed(sp);
        EXPECT_EQ(fixed.size(), 3u);
        EXPECT_EQ(fixed.data(), data.data());
        EXPECT_EQ(fixed[0], 1);
        EXPECT_EQ(fixed[1], 2);
        EXPECT_EQ(fixed[2], 3);
    }
    // Span<const T> -> Span<const T, N>
    {
        FakeRange data;
        SPAN<const int> sp(data);
        SPAN<const int, 5> fixed(sp);
        EXPECT_EQ(fixed.size(), 5u);
        EXPECT_EQ(fixed.data(), data.data());
        EXPECT_EQ(fixed[0], 1);
        EXPECT_EQ(fixed[1], 2);
        EXPECT_EQ(fixed[2], 3);
    }
    // ityp::span<Index, const T> -> ityp::span<Index, const T, N>
    {
        FakeTypedRange data;
        ITYP_SPAN<Index, const int> sp(data);
        ITYP_SPAN<Index, const int, Index{5u}> fixed(sp);
        EXPECT_EQ(fixed.size(), Index{5u});
        EXPECT_EQ(fixed.data(), data.data());
        EXPECT_EQ(fixed[Index{0u}], 1);
        EXPECT_EQ(fixed[Index{1u}], 2);
        EXPECT_EQ(fixed[Index{2u}], 3);
    }
    // Empty dynamic span -> Span<T, 0>
    {
        SPAN<int> sp;
        SPAN<int, 0> fixed(sp);
        EXPECT_EQ(fixed.size(), 0u);
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
    // Empty dynamic const span -> Span<const T, 0>
    {
        SPAN<const int> sp;
        SPAN<const int, 0> fixed(sp);
        EXPECT_EQ(fixed.size(), 0u);
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
    // Empty dynamic typed span -> ityp::span<Index, T, 0>
    {
        ITYP_SPAN<Index, int> sp;
        ITYP_SPAN<Index, int, Index{0u}> fixed(sp);
        EXPECT_EQ(fixed.size(), Index{0u});
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
    // Empty dynamic const typed span -> ityp::span<Index, const T, 0>
    {
        ITYP_SPAN<Index, const int> sp;
        ITYP_SPAN<Index, const int, Index{0u}> fixed(sp);
        EXPECT_EQ(fixed.size(), Index{0u});
        EXPECT_EQ(fixed.data(), nullptr);
        EXPECT_TRUE(fixed.empty());
    }
}

TYPED_TEST(SpanDeathTest, Constructor_DynamicToFixedSizeMismatch) {
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        FakeRange constData;
        SPAN<int> sp(data);
        SPAN<const int> constSp(constData);

        // Dynamic span larger than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((SPAN<int, 2>{sp}), "");
        EXPECT_DEATH_IF_SUPPORTED((SPAN<const int, 2>{constSp}), "");

        // Dynamic span smaller than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((SPAN<int, 10>{sp}), "");
        EXPECT_DEATH_IF_SUPPORTED((SPAN<const int, 10>{constSp}), "");

        // Empty dynamic span to non-zero fixed extent.
        SPAN<int> emptySp;
        SPAN<const int> emptyConstSp;
        EXPECT_DEATH_IF_SUPPORTED((SPAN<int, 3>{emptySp}), "");
        EXPECT_DEATH_IF_SUPPORTED((SPAN<const int, 3>{emptyConstSp}), "");
    }

    // Typed integer index tests.
    {
        FakeTypedRange data;
        ITYP_SPAN<Index, const int> constSp(data);

        // Dynamic span larger than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((ITYP_SPAN<Index, const int, Index{2u}>{constSp}), "");

        // Dynamic span smaller than fixed extent.
        EXPECT_DEATH_IF_SUPPORTED((ITYP_SPAN<Index, const int, Index{10u}>{constSp}), "");

        // Empty dynamic span to non-zero fixed extent.
        ITYP_SPAN<Index, int> emptySp;
        ITYP_SPAN<Index, const int> emptyConstSp;
        EXPECT_DEATH_IF_SUPPORTED((ITYP_SPAN<Index, int, Index{3u}>{emptySp}), "");
        EXPECT_DEATH_IF_SUPPORTED((ITYP_SPAN<Index, const int, Index{3u}>{emptyConstSp}), "");
    }
}

TYPED_TEST(SpanTest, CopyConstructor) {
    std::array<int, 3> data = {1, 2, 3};
    SPAN<int, 3> sp{data};

    SPAN<int> sp2{sp};
    ASSERT_EQ(sp.data(), sp2.data());
    ASSERT_EQ(sp.size(), sp2.size());

    // Actually calls the constructor from a range.
    SPAN<const int> sp3{sp2};
    ASSERT_EQ(sp.data(), sp3.data());
    ASSERT_EQ(sp.size(), sp3.size());

    // Actually calls the constructor from a range.
    SPAN<const int, 3> sp4{sp};
    ASSERT_EQ(sp.data(), sp4.data());
    ASSERT_EQ(sp.size(), sp4.size());
}

TYPED_TEST(SpanTest, CopyAssignment) {
    std::array<int, 3> data = {1, 2, 3};
    SPAN<int, 3> sp{data};

    SPAN<int> sp2;
    sp2 = sp;
    ASSERT_EQ(sp.data(), sp2.data());
    ASSERT_EQ(sp.size(), sp2.size());

    // Actually calls the constructor from a range and then assigns.
    SPAN<const int> sp3;
    sp3 = sp2;
    ASSERT_EQ(sp.data(), sp3.data());
    ASSERT_EQ(sp.size(), sp3.size());
}

TYPED_TEST(SpanTest, MoveConstructor) {
    std::array<int, 3> data = {1, 2, 3};
    SPAN<int> sp{data};

    SPAN<int> sp2{std::move(sp)};
    ASSERT_EQ(data.data(), sp2.data());
    ASSERT_EQ(data.size(), sp2.size());

    // "Default move constructor" copies so sp stays the same.
    ASSERT_EQ(data.data(), sp.data());
    ASSERT_EQ(data.size(), sp.size());
}

TYPED_TEST(SpanTest, MoveAssignment) {
    std::array<int, 3> data = {1, 2, 3};
    SPAN<int> sp{data};

    SPAN<int> sp2;
    sp2 = std::move(sp);
    ASSERT_EQ(data.data(), sp2.data());
    ASSERT_EQ(data.size(), sp2.size());

    // "Default move assignment" copies so sp stays the same.
    ASSERT_EQ(data.data(), sp.data());
    ASSERT_EQ(data.size(), sp.size());
}

TYPED_TEST(SpanTest, BeginEnd) {
    {
        SPAN<const int> sp(FakeRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
    {
        SPAN<const int, 5> sp(FakeRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
    {
        ITYP_SPAN<Index, const int> sp(FakeTypedRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
    {
        ITYP_SPAN<Index, const int, Index{5u}> sp(FakeTypedRange{});
        ASSERT_EQ(&*sp.begin(), &*kSpanData.begin());
        ASSERT_EQ(&*sp.end(), &*kSpanData.end());
    }
}

TYPED_TEST(SpanTest, BeginEndForIteration) {
    // Uses begin/end
    {
        SPAN<const int> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        SPAN<const int, 5> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }

    // Uses cbegin/cend
    {
        const SPAN<const int> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        const SPAN<const int, 5> sp(FakeRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }

    // ityp, uses begin/end
    {
        ITYP_SPAN<Index, const int> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        ITYP_SPAN<Index, const int, Index{5u}> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }

    // ityp, uses cbegin/cend
    {
        const ITYP_SPAN<Index, const int> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
    {
        const ITYP_SPAN<Index, const int, Index{5u}> sp(FakeTypedRange{});

        int expected = 1;
        for (const int& i : sp) {
            EXPECT_EQ(i, expected);
            expected++;
        }
    }
}

TYPED_TEST(SpanDeathTest, IteratorsAreHardened) {
    std::array<int, 4> src4{};
    std::array<int, 3> src3{};

    std::array<int, 4> dst{};
    SPAN<int> dstSpan = SPAN<int>(dst).first(3u);

    // Control case, copying 3 elements to dst is valid.
    std::ranges::copy(src3, dstSpan.begin());

    // We check specifically that libc++'s bounded iterators are used as iterators for dawn::Span.
#if defined(_LIBCPP_ABI_BOUNDED_ITERATORS)
    // Error case, the hardened iterator will fail when we try to write a non-existent 4th element.
    EXPECT_DEATH_IF_SUPPORTED(std::ranges::copy(src4, dstSpan.begin()), "");
#else
    // Still a success case because we don't have bounded iterators.
    std::ranges::copy(src4, dstSpan.begin());
#endif
}

TYPED_TEST(SpanTest, FrontBack) {
    {
        SPAN<const int> sp(FakeRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
    {
        SPAN<const int, 5> sp(FakeRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
    {
        ITYP_SPAN<Index, const int> sp(FakeTypedRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
    {
        ITYP_SPAN<Index, const int, Index{5u}> sp(FakeTypedRange{});
        EXPECT_EQ(&sp.front(), &kSpanData.front());
        EXPECT_EQ(&sp.back(), &kSpanData.back());
    }
}

TYPED_TEST(SpanDeathTest, FrontBackOfEmpty) {
    {
        SPAN<const int> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
    }
    {
        SPAN<const int, 0> sp;
        EXPECT_DEATH_IF_SUPPORTED(sp.front(), "");
        EXPECT_DEATH_IF_SUPPORTED(sp.back(), "");
    }
}

TYPED_TEST(SpanTest, Indexing) {
    {
        SPAN<const int> sp(FakeRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            EXPECT_EQ(&sp.at(i), &kSpanData[i]);
            EXPECT_EQ(&sp[i], &kSpanData[i]);
        }
    }
    {
        SPAN<const int, 5> sp(FakeRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            EXPECT_EQ(&sp.at(i), &kSpanData[i]);
            EXPECT_EQ(&sp[i], &kSpanData[i]);
        }
    }
    {
        ITYP_SPAN<Index, const int> sp(FakeTypedRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            Index id{static_cast<uint32_t>(i)};
            EXPECT_EQ(&sp.at(id), &kSpanData[i]);
            EXPECT_EQ(&sp[id], &kSpanData[i]);
        }
    }
    {
        ITYP_SPAN<Index, const int, Index{5u}> sp(FakeTypedRange{});
        for (size_t i = 0; i < kSpanData.size(); i++) {
            Index id{static_cast<uint32_t>(i)};
            EXPECT_EQ(&sp.at(id), &kSpanData[i]);
            EXPECT_EQ(&sp[id], &kSpanData[i]);
        }
    }
}

TYPED_TEST(SpanDeathTest, IndexingOOB) {
    SPAN<const int> sp(FakeRange{});
    EXPECT_DEATH_IF_SUPPORTED(sp.at(sp.size()), "");
    EXPECT_DEATH_IF_SUPPORTED(sp[sp.size()], "");

    SPAN<const int> spEmpty;
    EXPECT_DEATH_IF_SUPPORTED(spEmpty.at(0u), "");
    EXPECT_DEATH_IF_SUPPORTED(spEmpty[0u], "");

    SPAN<const int, 0> spFixedEmpty;
    EXPECT_DEATH_IF_SUPPORTED(spFixedEmpty.at(0u), "");
    EXPECT_DEATH_IF_SUPPORTED(spFixedEmpty[0u], "");
}

TYPED_TEST(SpanDeathTest, IndexingOversizedIndex) {
    // These tests are only relevant on 32-bit builds.
    if constexpr (sizeof(size_t) > sizeof(uint32_t)) {
        GTEST_SKIP();
    }

    auto sp = ITYP_SPAN<Index64, const int>(FakeTyped64Range());

    // The narrowing to size_t would give 0 which is in bounds, so this checks that the cast to
    // size_t itself causes a crash.
    constexpr Index64 kHugeIndex{0x1'0000'0000LLU};
    EXPECT_DEATH_IF_SUPPORTED(sp[kHugeIndex], "");
}

// .data() and .size() are tested in every test essentially.

TEST(SpanTest, UntypedSize) {
    int data[1000];
    Span<const int> sp{data};

    auto* sp8 = reinterpret_cast<ityp::span<Index8, const int>*>(&sp);

    // size() should return an Index8 that happens to be clamped, but untyped_size() will give the
    // whole size stored in the span.
    ASSERT_EQ(sp8->size(), Index8{static_cast<uint8_t>(sp.size())});
    ASSERT_EQ(sp8->untyped_size(), sp.size());
}

TYPED_TEST(SpanTest, Empty) {
    ASSERT_FALSE(SPAN<const int>{FakeRange{}}.empty());
    // SAFETY: Test for the unsafe constructor.
    ASSERT_FALSE(DAWN_UNSAFE_BUFFERS((SPAN<const int>{static_cast<int*>(nullptr), 1u})).empty());
    ASSERT_TRUE(SPAN<const int>{}.empty());
    ASSERT_TRUE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((SPAN<const int>{kSpanData.data(), 0u})).empty());

    ASSERT_TRUE((SPAN<const int, 0>{}.empty()));
    ASSERT_TRUE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((SPAN<const int, 0>{kSpanData.data()})).empty());

    ASSERT_FALSE((ITYP_SPAN<Index, const int>{FakeTypedRange{}}.empty()));
    ASSERT_FALSE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((ITYP_SPAN<Index, const int>{static_cast<int*>(nullptr), Index{1u}}))
            .empty());
    ASSERT_TRUE((ITYP_SPAN<Index, const int>{}.empty()));
    ASSERT_TRUE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((ITYP_SPAN<Index, const int>{kSpanData.data(), Index{0u}})).empty());

    ASSERT_TRUE((ITYP_SPAN<Index, const int, Index{0u}>{}.empty()));
    ASSERT_TRUE(
        // SAFETY: Test for the unsafe constructor.
        DAWN_UNSAFE_BUFFERS((ITYP_SPAN<Index, const int, Index{0u}>{kSpanData.data(), Index{0u}}))
            .empty());
}

TYPED_TEST(SpanTest, SizeBytes) {
    ASSERT_EQ(SPAN<int>{}.size_bytes(), 0u);
    ASSERT_EQ((ITYP_SPAN<Index, int>{}.size_bytes()), 0u);
    ASSERT_EQ((ITYP_SPAN<Index, int, Index{0u}>{}.size_bytes()), 0u);

    std::array<int, 3> ints{};
    ASSERT_EQ(SPAN<int>{ints}.size_bytes(), 3 * sizeof(int));
    ASSERT_EQ((SPAN<int, 3>{ints}.size_bytes()), 3 * sizeof(int));

    std::array<double, 10> doubles{};
    ASSERT_EQ(SPAN<double>{doubles}.size_bytes(), 10 * sizeof(double));
    ASSERT_EQ((SPAN<double, 10>{doubles}.size_bytes()), 10 * sizeof(double));
}

TYPED_TEST(SpanTest, FirstLast) {
    {
        SPAN<const int> sp{FakeRange()};
        SPAN<const int> first0 = sp.first(0);
        SPAN<const int> first2 = sp.first(2);
        SPAN<const int> last0 = sp.last(0);
        SPAN<const int> last2 = sp.last(2);

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), 0u);
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), 2u);

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), 0u);
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - 2));
        EXPECT_EQ(last2.size(), 2u);
    }
    {
        SPAN<const int, 5> sp{FakeRange()};
        SPAN<const int> first0 = sp.first(0);
        SPAN<const int> first2 = sp.first(2);
        SPAN<const int> last0 = sp.last(0);
        SPAN<const int> last2 = sp.last(2);

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), 0u);
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), 2u);

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), 0u);
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - 2));
        EXPECT_EQ(last2.size(), 2u);
    }

    {
        ITYP_SPAN<Index, const int> sp{FakeTypedRange()};
        ITYP_SPAN<Index, const int> first0 = sp.first(Index{0u});
        ITYP_SPAN<Index, const int> first2 = sp.first(Index{2u});
        ITYP_SPAN<Index, const int> last0 = sp.last(Index{0u});
        ITYP_SPAN<Index, const int> last2 = sp.last(Index{2u});

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), Index{0u});
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), Index{2u});

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), Index{0u});
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - Index{2u}));
        EXPECT_EQ(last2.size(), Index{2u});
    }
    {
        ITYP_SPAN<Index, const int, Index{5u}> sp{FakeTypedRange()};
        ITYP_SPAN<Index, const int> first0 = sp.first(Index{0u});
        ITYP_SPAN<Index, const int> first2 = sp.first(Index{2u});
        ITYP_SPAN<Index, const int> last0 = sp.last(Index{0u});
        ITYP_SPAN<Index, const int> last2 = sp.last(Index{2u});

        EXPECT_EQ(first0.data(), sp.data());
        EXPECT_EQ(first0.size(), Index{0u});
        EXPECT_EQ(first2.data(), sp.data());
        EXPECT_EQ(first2.size(), Index{2u});

        EXPECT_EQ(last0.data(), &*sp.end());
        EXPECT_EQ(last0.size(), Index{0u});
        EXPECT_EQ(last2.data(), &sp.at(sp.size() - Index{2u}));
        EXPECT_EQ(last2.size(), Index{2u});
    }
}

TYPED_TEST(SpanDeathTest, FirstLastOOB) {
    SPAN<const int> sp{FakeRange()};

    sp.first(sp.size());
    EXPECT_DEATH_IF_SUPPORTED(sp.first(sp.size() + 1), "");

    sp.last(sp.size());
    EXPECT_DEATH_IF_SUPPORTED(sp.last(sp.size() + 1), "");
}

TYPED_TEST(SpanTest, Subspan1Arg) {
    {
        SPAN<const int> sp{FakeRange()};
        SPAN<const int> subspan0 = sp.subspan(0);
        SPAN<const int> subspan2 = sp.subspan(2);

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(2));
        EXPECT_EQ(subspan2.size(), sp.size() - 2);
    }
    {
        SPAN<const int, 5> sp{FakeRange()};
        SPAN<const int> subspan0 = sp.subspan(0);
        SPAN<const int> subspan2 = sp.subspan(2);

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(2));
        EXPECT_EQ(subspan2.size(), sp.size() - 2);
    }
    {
        ITYP_SPAN<Index, const int> sp{FakeTypedRange()};
        ITYP_SPAN<Index, const int> subspan0 = sp.subspan(Index{0u});
        ITYP_SPAN<Index, const int> subspan2 = sp.subspan(Index{2u});

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(Index{2u}));
        EXPECT_EQ(subspan2.size(), sp.size() - Index{2u});
    }
    {
        ITYP_SPAN<Index, const int, Index{5u}> sp{FakeTypedRange()};
        ITYP_SPAN<Index, const int> subspan0 = sp.subspan(Index{0u});
        ITYP_SPAN<Index, const int> subspan2 = sp.subspan(Index{2u});

        EXPECT_EQ(subspan0.data(), sp.data());
        EXPECT_EQ(subspan0.size(), sp.size());
        EXPECT_EQ(subspan2.data(), &sp.at(Index{2u}));
        EXPECT_EQ(subspan2.size(), sp.size() - Index{2u});
    }
}

TYPED_TEST(SpanDeathTest, Subspan1ArgOOB) {
    {
        SPAN<const int> sp{FakeRange()};
        sp.subspan(sp.size());
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(sp.size() + 1), "");
    }
    {
        SPAN<const int, 5> sp{FakeRange()};
        sp.subspan(sp.size());
        EXPECT_DEATH_IF_SUPPORTED(sp.subspan(sp.size() + 1), "");
    }
}

TYPED_TEST(SpanTest, Subspan2Args) {
    {
        SPAN<const int> sp{FakeRange()};
        SPAN<const int> subspan0_2 = sp.subspan(0, 2);
        SPAN<const int> subspan3_2 = sp.subspan(3, 2);

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), 2u);
        EXPECT_EQ(subspan3_2.data(), &sp.at(3));
        EXPECT_EQ(subspan3_2.size(), 2u);
    }
    {
        SPAN<const int, 5> sp{FakeRange()};
        SPAN<const int> subspan0_2 = sp.subspan(0, 2);
        SPAN<const int> subspan3_2 = sp.subspan(3, 2);

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), 2u);
        EXPECT_EQ(subspan3_2.data(), &sp.at(3));
        EXPECT_EQ(subspan3_2.size(), 2u);
    }

    {
        ITYP_SPAN<Index, const int> sp{FakeTypedRange()};
        ITYP_SPAN<Index, const int> subspan0_2 = sp.subspan(Index{0u}, Index{2u});
        ITYP_SPAN<Index, const int> subspan3_2 = sp.subspan(Index{3u}, Index{2u});

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), Index{2u});
        EXPECT_EQ(subspan3_2.data(), &sp.at(Index{3u}));
        EXPECT_EQ(subspan3_2.size(), Index{2u});
    }
    {
        ITYP_SPAN<Index, const int, Index{5u}> sp{FakeTypedRange()};
        ITYP_SPAN<Index, const int> subspan0_2 = sp.subspan(Index{0u}, Index{2u});
        ITYP_SPAN<Index, const int> subspan3_2 = sp.subspan(Index{3u}, Index{2u});

        EXPECT_EQ(subspan0_2.data(), sp.data());
        EXPECT_EQ(subspan0_2.size(), Index{2u});
        EXPECT_EQ(subspan3_2.data(), &sp.at(Index{3u}));
        EXPECT_EQ(subspan3_2.size(), Index{2u});
    }
}

TYPED_TEST(SpanDeathTest, Subspan2ArgOOB) {
    SPAN<const int> sp{FakeRange()};

    sp.subspan(2, sp.size() - 2);
    EXPECT_DEATH_IF_SUPPORTED(sp.subspan(2, sp.size() - 1), "");

    // Check that overflows of offset + count is handled.
    EXPECT_DEATH_IF_SUPPORTED(sp.subspan(std::numeric_limits<size_t>::max(), 1), "");
    EXPECT_DEATH_IF_SUPPORTED(sp.subspan(1, std::numeric_limits<size_t>::max()), "");

    // SAFETY: This is the same range as kSpanData, just viewed with a uint8_t index (which fits the
    // size of kSpanData since it is 5).
    auto sp8 = DAWN_UNSAFE_BUFFERS(
        ITYP_SPAN<Index8, const int>(kSpanData.data(), Index8{uint8_t{kSpanData.size()}}));

    Index8 kOne = Index8{uint8_t{1}};
    Index8 kTwo = Index8{uint8_t{2}};
    sp8.subspan(kTwo, sp8.size() - kTwo);
    EXPECT_DEATH_IF_SUPPORTED(sp8.subspan(kTwo, sp8.size() - kOne), "");

    // Check that overflows of offset + count is handled.
    EXPECT_DEATH_IF_SUPPORTED(sp8.subspan(std::numeric_limits<Index8>::max(), kOne), "");
    EXPECT_DEATH_IF_SUPPORTED(sp8.subspan(kOne, std::numeric_limits<Index8>::max()), "");
}

TYPED_TEST(SpanTest, SpanAsBytes) {
    // Empty spans.
    {
        SPAN<int> sp;
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<SPAN<const std::byte>, decltype(bsp)>);
        EXPECT_TRUE(bsp.empty());
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<SPAN<std::byte>, decltype(wbsp)>);
        EXPECT_TRUE(wbsp.empty());

        SPAN<volatile int> vsp;
        auto vbsp = SpanAsBytes(vsp);
        static_assert(std::is_same_v<SPAN<const volatile std::byte>, decltype(vbsp)>);
        EXPECT_TRUE(vbsp.empty());
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<SPAN<volatile std::byte>, decltype(vwbsp)>);
        EXPECT_TRUE(vwbsp.empty());
    }

    // Non-empty span.
    {
        std::array<int, 3> ints{};

        SPAN<int> sp{ints};
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<SPAN<const std::byte>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sp.size_bytes());
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(sp.data()));
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<SPAN<std::byte>, decltype(wbsp)>);
        EXPECT_EQ(wbsp.size(), sp.size_bytes());
        EXPECT_EQ(wbsp.data(), reinterpret_cast<std::byte*>(sp.data()));

        SPAN<volatile int> vsp{ints};
        auto vbsp = SpanAsBytes(vsp);
        static_assert(std::is_same_v<SPAN<const volatile std::byte>, decltype(vbsp)>);
        EXPECT_EQ(vbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vbsp.data(), reinterpret_cast<const volatile std::byte*>(vsp.data()));
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<SPAN<volatile std::byte>, decltype(vwbsp)>);
        EXPECT_EQ(vwbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vwbsp.data(), reinterpret_cast<volatile std::byte*>(vsp.data()));
    }

    // Fixed-extent span.
    {
        std::array<int, 3> ints{};

        SPAN<int, 3> sp{ints};
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<SPAN<const std::byte, sizeof(int) * 3u>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sp.size_bytes());
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(sp.data()));
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<SPAN<std::byte, sizeof(int) * 3u>, decltype(wbsp)>);
        EXPECT_EQ(wbsp.size(), sp.size_bytes());
        EXPECT_EQ(wbsp.data(), reinterpret_cast<std::byte*>(sp.data()));

        SPAN<volatile int, 3> vsp{ints};
        auto vbsp = SpanAsBytes(vsp);
        static_assert(
            std::is_same_v<SPAN<const volatile std::byte, sizeof(int) * 3u>, decltype(vbsp)>);
        EXPECT_EQ(vbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vbsp.data(), reinterpret_cast<const volatile std::byte*>(vsp.data()));
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<SPAN<volatile std::byte, sizeof(int) * 3u>, decltype(vwbsp)>);
        EXPECT_EQ(vwbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vwbsp.data(), reinterpret_cast<volatile std::byte*>(vsp.data()));
    }

    // Span with an index.
    {
        std::array<int, 3> ints{};

        // SAFETY: This is viewing ints, just with typed indices.
        ITYP_SPAN<Index, int> DAWN_UNSAFE_BUFFERS(sp{ints.data(), Index(3u)});
        auto bsp = SpanAsBytes(sp);
        static_assert(std::is_same_v<SPAN<const std::byte>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sp.size_bytes());
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(sp.data()));
        auto wbsp = SpanAsWritableBytes(sp);
        static_assert(std::is_same_v<SPAN<std::byte>, decltype(wbsp)>);
        EXPECT_EQ(wbsp.size(), sp.size_bytes());
        EXPECT_EQ(wbsp.data(), reinterpret_cast<std::byte*>(sp.data()));

        // SAFETY: This is viewing ints, just with typed indices.
        ITYP_SPAN<Index, volatile int> DAWN_UNSAFE_BUFFERS(vsp{ints.data(), Index(3u)});
        auto vbsp = SpanAsBytes(vsp);
        static_assert(std::is_same_v<SPAN<const volatile std::byte>, decltype(vbsp)>);
        EXPECT_EQ(vbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vbsp.data(), reinterpret_cast<const volatile std::byte*>(vsp.data()));
        auto vwbsp = SpanAsWritableBytes(vsp);
        static_assert(std::is_same_v<SPAN<volatile std::byte>, decltype(vwbsp)>);
        EXPECT_EQ(vwbsp.size(), vsp.size_bytes());
        EXPECT_EQ(vwbsp.data(), reinterpret_cast<volatile std::byte*>(vsp.data()));
    }
}

TYPED_TEST(SpanTest, ReinterpretEmptySpans) {
    // Non-const, non-volatile span.
    {
        SPAN<std::byte> bsp;
        auto sp = ReinterpretSpan<int>(bsp);
        static_assert(std::is_same_v<SPAN<int>, decltype(sp)>);
        EXPECT_TRUE(sp.empty());

        auto sp_const = ReinterpretSpan<const int>(bsp);
        static_assert(std::is_same_v<SPAN<const int>, decltype(sp_const)>);
        EXPECT_TRUE(sp_const.empty());

        auto sp_volatile = ReinterpretSpan<volatile int>(bsp);
        static_assert(std::is_same_v<SPAN<volatile int>, decltype(sp_volatile)>);
        EXPECT_TRUE(sp_volatile.empty());

        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<SPAN<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
    // Const span.
    {
        SPAN<const std::byte> bsp;
        auto sp_const = ReinterpretSpan<const int>(bsp);
        static_assert(std::is_same_v<SPAN<const int>, decltype(sp_const)>);
        EXPECT_TRUE(sp_const.empty());

        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<SPAN<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
    // Volatile span.
    {
        SPAN<volatile std::byte> bsp;
        auto sp_volatile = ReinterpretSpan<volatile int>(bsp);
        static_assert(std::is_same_v<SPAN<volatile int>, decltype(sp_volatile)>);
        EXPECT_TRUE(sp_volatile.empty());

        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<SPAN<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
    // Const and volatile span.
    {
        SPAN<const volatile std::byte> bsp;
        auto sp_const_volatile = ReinterpretSpan<const volatile int>(bsp);
        static_assert(std::is_same_v<SPAN<const volatile int>, decltype(sp_const_volatile)>);
        EXPECT_TRUE(sp_const_volatile.empty());
    }
}

TYPED_TEST(SpanTest, ReintepretSpans) {
    // Basic usages with varying const/volatile and type-ness.
    {
        alignas(uint32_t) std::array<std::byte, 8> bytes{};
        SPAN<std::byte> bsp{bytes};

        auto sp16 = ReinterpretSpan<uint16_t>(bsp);
        static_assert(std::is_same_v<SPAN<uint16_t>, decltype(sp16)>);
        EXPECT_EQ(sp16.size(), 4u);
        EXPECT_EQ(sp16.data(), reinterpret_cast<uint16_t*>(bsp.data()));

        auto sp32 = ReinterpretSpan<const uint32_t>(bsp);
        static_assert(std::is_same_v<SPAN<const uint32_t>, decltype(sp32)>);
        EXPECT_EQ(sp32.size(), 2u);
        EXPECT_EQ(sp32.data(), reinterpret_cast<const uint32_t*>(bsp.data()));

        auto sp16_typed = ReinterpretSpan<volatile uint16_t, Index>(bsp);
        static_assert(std::is_same_v<ITYP_SPAN<Index, volatile uint16_t>, decltype(sp16_typed)>);
        EXPECT_EQ(sp16_typed.size(), Index{4u});
        EXPECT_EQ(sp16_typed.data(), reinterpret_cast<volatile uint16_t*>(bsp.data()));

        auto sp32_typed = ReinterpretSpan<const volatile uint32_t, Index>(bsp);
        static_assert(
            std::is_same_v<ITYP_SPAN<Index, const volatile uint32_t>, decltype(sp32_typed)>);
        EXPECT_EQ(sp32_typed.size(), Index{2u});
        EXPECT_EQ(sp32_typed.data(), reinterpret_cast<const volatile uint32_t*>(bsp.data()));
    }
    // Round-trip data integrity with SpanAs*Bytes.
    {
        std::array<int, 3> ints{1, 2, 3};
        SPAN<int> sp{ints};
        {
            SPAN<const std::byte> bsp = SpanAsBytes(sp);
            SPAN<const int> sp2 = ReinterpretSpan<const int>(bsp);
            EXPECT_EQ(sp2.size(), sp.size());
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
        {
            SPAN<std::byte> wbsp = SpanAsWritableBytes(sp);
            SPAN<int> sp2 = ReinterpretSpan<int>(wbsp);
            EXPECT_EQ(sp2.size(), sp.size());
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
    }
}

TYPED_TEST(SpanTest, ReintepretFixedExtentSpans) {
    // Basic usages with varying const/volatile and type-ness.
    {
        alignas(uint32_t) std::array<std::byte, 8> bytes{};
        SPAN<std::byte, 8> bsp{bytes};

        auto sp16 = ReinterpretSpan<uint16_t>(bsp);
        static_assert(std::is_same_v<SPAN<uint16_t, 4>, decltype(sp16)>);
        EXPECT_EQ(sp16.data(), reinterpret_cast<uint16_t*>(bsp.data()));

        auto sp32 = ReinterpretSpan<const uint32_t>(bsp);
        static_assert(std::is_same_v<SPAN<const uint32_t, 2>, decltype(sp32)>);
        EXPECT_EQ(sp32.data(), reinterpret_cast<const uint32_t*>(bsp.data()));

        auto sp16_typed = ReinterpretSpan<volatile uint16_t, Index>(bsp);
        static_assert(
            std::is_same_v<ITYP_SPAN<Index, volatile uint16_t, Index{4u}>, decltype(sp16_typed)>);
        EXPECT_EQ(sp16_typed.data(), reinterpret_cast<volatile uint16_t*>(bsp.data()));

        auto sp32_typed = ReinterpretSpan<const volatile uint32_t, Index>(bsp);
        static_assert(std::is_same_v<ITYP_SPAN<Index, const volatile uint32_t, Index{2u}>,
                                     decltype(sp32_typed)>);
        EXPECT_EQ(sp32_typed.data(), reinterpret_cast<const volatile uint32_t*>(bsp.data()));
    }
    // Round-trip data integrity with SpanAs*Bytes.
    {
        std::array<int, 3> ints{1, 2, 3};
        SPAN<int, 3> sp{ints};
        {
            SPAN<const std::byte, sizeof(int) * 3u> bsp = SpanAsBytes(sp);
            SPAN<const int, 3> sp2 = ReinterpretSpan<const int>(bsp);
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
        {
            SPAN<std::byte, sizeof(int) * 3u> wbsp = SpanAsWritableBytes(sp);
            SPAN<int, 3> sp2 = ReinterpretSpan<int>(wbsp);
            EXPECT_EQ(sp2.data(), sp.data());
            EXPECT_TRUE(std::ranges::equal(sp2, sp));
        }
    }
}

TYPED_TEST(SpanDeathTest, ReinterpretSpan) {
    // Check unaligned empty span.
    // Empty slice (e.g. data() != nullptr, but size() == 0).
    {
        alignas(uint32_t) std::array<std::byte, 4> bytes{};
        auto bsp = SPAN<std::byte>(bytes).subspan(1u, 0);
        EXPECT_EQ(bsp.size(), 0u);
        EXPECT_NE(bsp.data(), nullptr);
        EXPECT_DEATH_IF_SUPPORTED(ReinterpretSpan<uint32_t>(bsp), "");
        EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint32_t, Index>(bsp)), "");
    }
    // Alignment check fails.
    {
        alignas(uint32_t) std::array<std::byte, 9> bytes{};
        auto bsp = SPAN<std::byte>(bytes).subspan(1u, 4u);
        if (alignof(uint32_t) > 1) {
            EXPECT_DEATH_IF_SUPPORTED(ReinterpretSpan<uint32_t>(bsp), "");
            EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint32_t, Index>(bsp)), "");
        }
    }
    // Size check fails.
    {
        alignas(uint32_t) std::array<std::byte, 8> bytes{};
        auto bsp = SPAN<std::byte>(bytes).first(7u);
        EXPECT_DEATH_IF_SUPPORTED(ReinterpretSpan<uint32_t>(bsp), "");
        EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint32_t, Index>(bsp)), "");
    }
    // Index check fails.
    {
        std::array<std::byte, 256> bytes{};
        auto bsp = SPAN<std::byte>(bytes);
        EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint8_t, Index8>(bsp)), "");
    }
    {
        std::array<std::byte, 255> bytes{};
        auto bsp = SPAN<std::byte>(bytes);
        EXPECT_DEATH_IF_SUPPORTED((ReinterpretSpan<uint8_t, Index8>(bsp)), "");
    }
}

TEST(SpanTest, SpanFromRef) {
    {
        uint32_t i = 0;

        auto sp = SpanFromRef(i);
        static_assert(std::is_same_v<Span<uint32_t, 1>, decltype(sp)>);
        EXPECT_EQ(sp.size(), 1u);
        EXPECT_EQ(sp.data(), &i);

        auto bsp = ByteSpanFromRef(i);
        static_assert(std::is_same_v<Span<std::byte, sizeof(uint32_t)>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sizeof(uint32_t));
        EXPECT_EQ(bsp.data(), reinterpret_cast<std::byte*>(&i));
    }
    {
        const uint32_t i = 0;

        auto sp = SpanFromRef(i);
        static_assert(std::is_same_v<Span<const uint32_t, 1>, decltype(sp)>);
        EXPECT_EQ(sp.size(), 1u);
        EXPECT_EQ(sp.data(), &i);

        auto bsp = ByteSpanFromRef(i);
        static_assert(std::is_same_v<Span<const std::byte, sizeof(uint32_t)>, decltype(bsp)>);
        EXPECT_EQ(bsp.size(), sizeof(uint32_t));
        EXPECT_EQ(bsp.data(), reinterpret_cast<const std::byte*>(&i));
    }
}

TEST(SpanTest, SpanFromRefTyped) {
    {
        uint32_t i = 0;

        auto sp = SpanFromRef<Index>(i);
        static_assert(std::is_same_v<ityp::span<Index, uint32_t, Index{1u}>, decltype(sp)>);
        EXPECT_EQ(sp.size(), Index{1u});
        EXPECT_EQ(sp.data(), &i);
    }
    {
        const uint32_t i = 0;

        auto sp = SpanFromRef<Index>(i);
        static_assert(std::is_same_v<ityp::span<Index, const uint32_t, Index{1u}>, decltype(sp)>);
        EXPECT_EQ(sp.size(), Index{1u});
        EXPECT_EQ(sp.data(), &i);
    }
}

TYPED_TEST(SpanTest, TakeFirst) {
    std::array<int, 3> ints{1, 2, 3};

    // Take only a part.
    {
        SPAN<int> sp{ints};
        auto taken = sp.TakeFirst(1);

        EXPECT_EQ(sp.data(), &ints[1]);
        EXPECT_EQ(sp.size(), 2u);

        static_assert(std::is_same_v<decltype(taken), SPAN<int>>);
        EXPECT_EQ(taken.data(), ints.data());
        EXPECT_EQ(taken.size(), 1u);
    }

    // Take none.
    {
        SPAN<int> sp{ints};
        auto taken = sp.TakeFirst(0);

        EXPECT_EQ(sp.data(), ints.data());
        EXPECT_EQ(sp.size(), 3u);

        static_assert(std::is_same_v<decltype(taken), SPAN<int>>);
        EXPECT_TRUE(taken.empty());
    }

    // Take all.
    {
        SPAN<int> sp{ints};
        auto taken = sp.TakeFirst(3);

        EXPECT_TRUE(sp.empty());

        static_assert(std::is_same_v<decltype(taken), SPAN<int>>);
        EXPECT_EQ(taken.data(), ints.data());
        EXPECT_EQ(taken.size(), 3u);
    }
}

TYPED_TEST(SpanDeathTest, TakeFirstOOB) {
    SPAN<const int> sp{FakeRange()};

    sp.TakeFirst(sp.size());
    EXPECT_DEATH_IF_SUPPORTED(sp.TakeFirst(sp.size() + 1), "");
}

TYPED_TEST(SpanTest, CopyFrom) {
    // Copy from implicitly constructed span (std::array)
    {
        std::array<int, 3> src = {1, 2, 3};
        std::array<int, 3> dst = {0, 0, 0};
        SPAN<int> dst_sp{dst};

        dst_sp.CopyFrom(src);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }

    // Copy from implicitly constructed span (std::array) with fixed extents
    {
        std::array<int, 3> src = {1, 2, 3};
        std::array<int, 3> dst = {0, 0, 0};
        SPAN<int, 3> dst_sp{dst};

        dst_sp.CopyFrom(src);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }

    // Copy from heap array (std::vector)
    {
        std::vector<int> src = {4, 5, 6};
        std::array<int, 3> dst = {0, 0, 0};
        SPAN<int> dst_sp{dst};

        dst_sp.CopyFrom(src);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }

    // Test different index types
    {
        std::array<int, 3> src = {1, 2, 3};
        std::array<int, 3> dst = {0, 0, 0};
        // SAFETY: This is viewing dst, just with typed indices.
        ITYP_SPAN<Index, int> DAWN_UNSAFE_BUFFERS(dst_sp(dst.data(), Index{3u}));
        // SAFETY: This is viewing src, just with typed indices.
        ITYP_SPAN<Index, const int> DAWN_UNSAFE_BUFFERS(src_sp(src.data(), Index{3u}));

        dst_sp.CopyFrom(src_sp);
        EXPECT_THAT(dst, ElementsAreArray(src));
    }
}

TYPED_TEST(SpanTest, CopyFromOverlapping) {
    // Forward copy (src before dst)
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        SPAN<int> sp{data};
        sp.subspan(1, 3).CopyFrom(sp.subspan(0, 3));
        EXPECT_THAT(data, testing::ElementsAre(1, 1, 2, 3, 5));
    }

    // Backward copy (dst before src)
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        SPAN<int> sp{data};
        sp.subspan(0, 3).CopyFrom(sp.subspan(1, 3));
        EXPECT_THAT(data, testing::ElementsAre(2, 3, 4, 4, 5));
    }
}

TYPED_TEST(SpanDeathTest, CopyFromSizeMismatch) {
    std::array<int, 3> src = {1, 2, 3};
    std::array<int, 2> dst = {0, 0};
    SPAN<int> dst_sp{dst};
    SPAN<const int> src_sp{src};

    EXPECT_DEATH_IF_SUPPORTED(dst_sp.CopyFrom(src_sp), "");
}

TYPED_TEST(SpanTest, CopyPrefixFrom) {
    // Copy from implicitly constructed span (std::array)
    {
        std::array<int, 2> src = {1, 2};
        std::array<int, 3> dst = {0, 0, 0};
        SPAN<int> dst_sp{dst};

        dst_sp.CopyPrefixFrom(src);
        EXPECT_THAT(dst, testing::ElementsAre(1, 2, 0));
    }

    // Copy from implicitly constructed span (std::array) with fixed extents
    {
        std::array<int, 2> src = {1, 2};
        std::array<int, 3> dst = {0, 0, 0};
        SPAN<int, 3> dst_sp{dst};

        dst_sp.CopyPrefixFrom(src);
        EXPECT_THAT(dst, testing::ElementsAre(1, 2, 0));
    }

    // Copy from heap array (std::vector)
    {
        std::vector<int> src = {4};
        std::array<int, 3> dst = {0, 0, 0};
        SPAN<int> dst_sp{dst};

        dst_sp.CopyPrefixFrom(src);
        EXPECT_THAT(dst, testing::ElementsAre(4, 0, 0));
    }

    // Test different index types
    {
        std::array<int, 2> src = {1, 2};
        std::array<int, 3> dst = {0, 0, 0};
        // SAFETY: This is viewing dst, just with typed indices.
        ITYP_SPAN<Index, int> DAWN_UNSAFE_BUFFERS(dst_sp(dst.data(), Index{3u}));
        // SAFETY: This is viewing src, just with typed indices.
        ITYP_SPAN<Index, const int> DAWN_UNSAFE_BUFFERS(src_sp(src.data(), Index{2u}));

        dst_sp.CopyPrefixFrom(src_sp);
        EXPECT_THAT(dst, testing::ElementsAre(1, 2, 0));
    }
}

TYPED_TEST(SpanTest, CopyPrefixFromOverlapping) {
    // Forward copy
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        SPAN<int> sp{data};
        sp.subspan(1, 4).CopyPrefixFrom(sp.subspan(0, 3));
        EXPECT_THAT(data, testing::ElementsAre(1, 1, 2, 3, 5));
    }

    // Backward copy
    {
        std::array<int, 5> data = {1, 2, 3, 4, 5};
        SPAN<int> sp{data};
        sp.subspan(0, 4).CopyPrefixFrom(sp.subspan(1, 3));
        EXPECT_THAT(data, testing::ElementsAre(2, 3, 4, 4, 5));
    }
}

TYPED_TEST(SpanDeathTest, CopyPrefixFromSizeMismatch) {
    std::array<int, 3> src = {1, 2, 3};
    std::array<int, 2> dst = {0, 0};
    SPAN<int> dst_sp{dst};
    SPAN<const int> src_sp{src};

    EXPECT_DEATH_IF_SUPPORTED(dst_sp.CopyPrefixFrom(src_sp), "");
}

TYPED_TEST(SpanTest, FillBytes) {
    // Fill dynamic extent span (std::byte)
    {
        std::array<std::byte, 5> data = {};
        SPAN<std::byte> sp{data};
        sp.FillBytes(std::byte{0x7F});
        EXPECT_THAT(data, testing::ElementsAre(std::byte{0x7F}, std::byte{0x7F}, std::byte{0x7F},
                                               std::byte{0x7F}, std::byte{0x7F}));
    }

    // Fill fixed extent span
    {
        std::array<std::byte, 3> data = {};
        SPAN<std::byte, 3> sp{data};
        sp.FillBytes(std::byte{0xAB});
        EXPECT_THAT(data, testing::ElementsAre(std::byte{0xAB}, std::byte{0xAB}, std::byte{0xAB}));
    }

    // Fill ityp::span with custom index
    {
        std::array<std::byte, 3> data = {};
        // SAFETY: This is viewing data, just with typed indices.
        ITYP_SPAN<Index, std::byte> DAWN_UNSAFE_BUFFERS(sp(data.data(), Index{3u}));
        sp.FillBytes(std::byte{0x42});
        EXPECT_THAT(data, testing::ElementsAre(std::byte{0x42}, std::byte{0x42}, std::byte{0x42}));
    }

    // Fill subspan
    {
        std::array<std::byte, 5> data = {std::byte{1}, std::byte{2}, std::byte{3}, std::byte{4},
                                         std::byte{5}};
        SPAN<std::byte> sp{data};
        sp.subspan(1, 3).FillBytes(std::byte{0});
        EXPECT_THAT(data, testing::ElementsAre(std::byte{1}, std::byte{0}, std::byte{0},
                                               std::byte{0}, std::byte{5}));
    }

    // Fill empty span
    {
        SPAN<std::byte> empty_sp;
        empty_sp.FillBytes(std::byte{0xFF});
        EXPECT_TRUE(empty_sp.empty());
    }
}

TYPED_TEST(SpanTest, Constructor_CArray) {
    int arr[] = {1, 2, 3, 4};
    const int constArr[] = {5, 6, 7};

    {
        SPAN<int> sp(arr);
        EXPECT_EQ(sp.size(), 4u);
        EXPECT_EQ(sp.data(), arr);
        EXPECT_EQ(sp[0], 1);
    }
    {
        SPAN<int, 4> sp(arr);
        EXPECT_EQ(sp.data(), arr);
        EXPECT_EQ(sp[0], 1);
    }
    {
        SPAN<const int> sp(arr);
        EXPECT_EQ(sp.size(), 4u);
        EXPECT_EQ(sp.data(), arr);
    }
    {
        SPAN<const int> sp(constArr);
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constArr);
    }
    {
        SPAN<const int, 3> sp(constArr);
        EXPECT_EQ(sp.data(), constArr);
    }
}

TYPED_TEST(SpanTest, Constructor_InitializeList) {
    std::initializer_list<int> list = {1, 2, 3, 4};
    std::initializer_list<const int> constList = {5, 6, 7};

    {
        SPAN<const int> sp(list);
        EXPECT_EQ(sp.size(), 4u);
        EXPECT_EQ(sp.data(), list.begin());
    }
    {
        SPAN<const int> sp(constList);
        EXPECT_EQ(sp.size(), 3u);
        EXPECT_EQ(sp.data(), constList.begin());
    }
}

TEST(SpanTest, RawSpanToSpan) {
    std::array<int, 3> arr = {1, 2, 3};

    // Dynamic extent span
    {
        RawSpan<int> raw(arr);

        auto TakesSpan = [](Span<int> s) { return s.size(); };
        EXPECT_EQ(TakesSpan(raw), 3u);
        auto TakesConstSpan = [](Span<const int> s) { return s.size(); };
        EXPECT_EQ(TakesConstSpan(raw), 3u);
        {
            Span<int> s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            Span<int> s;
            s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            Span<const int> s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            Span<const int> s;
            s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
    }

    // Fixed extent span
    {
        RawSpan<int, 3> raw(arr);

        auto TakesFixedSpan = [](Span<int, 3> s) { return s.size(); };
        EXPECT_EQ(TakesFixedSpan(raw), 3u);
        auto TakesFixedConstSpan = [](Span<const int, 3> s) { return s.size(); };
        EXPECT_EQ(TakesFixedConstSpan(raw), 3u);

        {
            Span<int, 3> s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            Span<int, 3> s;
            s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            Span<const int, 3> s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            Span<const int, 3> s;
            s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
    }

    // Typed index span
    {
        // SAFETY: This is viewing arr, just with typed indices.
        ityp::raw_span<Index, int> DAWN_UNSAFE_BUFFERS(raw(arr.data(), Index{3u}));

        auto TakesItypSpan = [](ityp::span<Index, int> s) { return s.size(); };
        EXPECT_EQ(TakesItypSpan(raw), Index{3u});
        auto TakesItypConstSpan = [](ityp::span<Index, const int> s) { return s.size(); };
        EXPECT_EQ(TakesItypConstSpan(raw), Index{3u});
        {
            ityp::span<Index, int> s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            ityp::span<Index, int> s;
            s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            ityp::span<Index, const int> s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            ityp::span<Index, const int> s;
            s = raw;
            EXPECT_EQ(s.data(), arr.data());
        }
    }
}

TEST(SpanTest, SpanToRawSpan) {
    std::array<int, 3> arr = {1, 2, 3};

    // Dynamic extent span
    {
        Span<int> sp(arr);

        auto TakesRawSpan = [](RawSpan<int> s) { return s.size(); };
        EXPECT_EQ(TakesRawSpan(sp), 3u);
        auto TakesConstRawSpan = [](RawSpan<const int> s) { return s.size(); };
        EXPECT_EQ(TakesConstRawSpan(sp), 3u);
        {
            RawSpan<int> s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            RawSpan<int> s;
            s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            RawSpan<const int> s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            RawSpan<const int> s;
            s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
    }

    // Fixed extent span
    {
        Span<int, 3> sp(arr);

        auto TakesFixedRawSpan = [](RawSpan<int, 3> s) { return s.size(); };
        EXPECT_EQ(TakesFixedRawSpan(sp), 3u);
        auto TakesFixedConstRawSpan = [](RawSpan<const int, 3> s) { return s.size(); };
        EXPECT_EQ(TakesFixedConstRawSpan(sp), 3u);

        {
            RawSpan<int, 3> s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            RawSpan<int, 3> s;
            s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            RawSpan<const int, 3> s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            RawSpan<const int, 3> s;
            s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
    }

    // Typed index span
    {
        // SAFETY: This is viewing arr, just with typed indices.
        ityp::span<Index, int> DAWN_UNSAFE_BUFFERS(sp(arr.data(), Index{3u}));

        auto TakesItypRawSpan = [](ityp::raw_span<Index, int> s) { return s.size(); };
        EXPECT_EQ(TakesItypRawSpan(sp), Index{3u});
        auto TakesItypConstRawSpan = [](ityp::raw_span<Index, const int> s) { return s.size(); };
        EXPECT_EQ(TakesItypConstRawSpan(sp), Index{3u});
        {
            ityp::raw_span<Index, int> s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            ityp::raw_span<Index, int> s;
            s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            ityp::raw_span<Index, const int> s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
        {
            ityp::raw_span<Index, const int> s;
            s = sp;
            EXPECT_EQ(s.data(), arr.data());
        }
    }
}

}  // anonymous namespace
}  // namespace dawn
