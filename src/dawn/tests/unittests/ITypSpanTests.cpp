// Copyright 2020 The Dawn & Tint Authors
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

#include <array>

#include "gtest/gtest.h"
#include "src/dawn/common/ityp_span.h"
#include "src/utils/typed_integer.h"

namespace dawn {
namespace {

class ITypSpanTest : public testing::Test {
  protected:
    using Key = TypedInteger<struct KeyT, size_t>;
    using Val = TypedInteger<struct ValT, uint32_t>;
    using Span = ityp::span<Key, Val>;
};

// Test the utility SpanFromUntyped
TEST_F(ITypSpanTest, SpanFromUntyped) {
    // Test creating an empty span.
    {
        Val* values = nullptr;
        Span span = ityp::SpanFromUntyped<Key>(values, 0);
        ASSERT_EQ(nullptr, span.data());
        ASSERT_EQ(Key(0u), span.size());
    }
    // Test creating a one element span.
    {
        Val value = Val(25u);
        Span span = ityp::SpanFromUntyped<Key>(&value, 1);
        ASSERT_EQ(&value, span.data());
        ASSERT_EQ(Key(1u), span.size());
        ASSERT_EQ(value, span[Key(0u)]);
    }
    // Test creating a multi-element span.
    {
        std::array<Val, 10> arr = {};
        Span span = ityp::SpanFromUntyped<Key>(arr.data(), arr.size());
        ASSERT_EQ(arr.data(), span.data());
        ASSERT_EQ(arr.size(), static_cast<size_t>(span.size()));
    }
}

}  // anonymous namespace
}  // namespace dawn
