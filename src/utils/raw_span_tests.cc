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

#include <memory>
#include <vector>

#include "src/utils/gtest.h"
#include "src/utils/span.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

using Index = TypedInteger<struct IndexT, uint32_t>;

TEST(RawSpanDanglingDeathTest, Constructor) {
    auto owner = std::make_unique<std::vector<int>>(std::initializer_list<int>{1, 2, 3});
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sp = RawSpan<int>{*owner};
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sub = RawSpan<int>{*owner}.subspan(1u);
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sp =
                // SAFETY: This is viewing owner, just with typed indices.
                DAWN_UNSAFE_BUFFERS((ityp::raw_span<Index, int>{owner->data(), Index{3u}}));
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sub =
                // SAFETY: This is viewing owner, just with typed indices.
                DAWN_UNSAFE_BUFFERS((ityp::raw_span<Index, int>{owner->data(), Index{3u}}))
                    .subspan(Index{1u});
            owner.reset();
        },
        "");
}

TEST(RawSpanDanglingDeathTest, SpanAsBytes) {
    auto owner = std::make_unique<std::vector<int>>(std::initializer_list<int>{1, 2, 3});
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto bsp = SpanAsBytes(RawSpan<int>{*owner});
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sub = SpanAsBytes(RawSpan<int>{*owner}).subspan(1u);
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto wbsp = SpanAsWritableBytes(RawSpan<int>{*owner});
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sub = SpanAsWritableBytes(RawSpan<int>{*owner}).subspan(1u);
            owner.reset();
        },
        "");
}

TEST(RawSpanDanglingDeathTest, ReinterpretSpan) {
    auto owner = std::make_unique<std::vector<std::byte>>(4 * sizeof(int));
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sp = ReinterpretSpan<int>(RawSpan<std::byte>{*owner});
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sub =
                ReinterpretSpan<int>(RawSpan<std::byte>{*owner}).subspan(1u);
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sp = (ReinterpretSpan<int, Index>(RawSpan<std::byte>{*owner}));
            owner.reset();
        },
        "");
    DAWN_EXPECT_DEATH_IF_SUPPORTED(
        {
            [[maybe_unused]] auto sub =
                (ReinterpretSpan<int, Index>(RawSpan<std::byte>{*owner})).subspan(Index{1u});
            owner.reset();
        },
        "");
}

}  // anonymous namespace
}  // namespace dawn
