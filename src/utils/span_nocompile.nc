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

// Test the type checks of dawn::Span.
//
// Note: All unsafe buffer warning tests are in warning_nocompile.nc.
// Warnings don't trigger in this file because it contains errors. Because of this,
// DAWN_UNSAFE_BUFFERS() annotations also don't need be to used in this file - they're ignored.

#include <array>

#include "src/utils/span.h"
#include "src/utils/typed_integer.h"

namespace dawn {

static constexpr std::array<int, 5> kSpanData = {1, 2, 3, 4, 5};

using Index = TypedInteger<struct IndexT, uint32_t>;
using IndexSizeT = TypedInteger<struct IndexT, size_t>;

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

void TestConstPointerToNonConstSpan() {
    Span<const int>(kSpanData.data(), kSpanData.size()); // Control case.
    Span<int>(kSpanData.data(), kSpanData.size()); // expected-error {{no matching constructor for initialization}}

    Span<const int>(kSpanData.begin(), kSpanData.end()); // Control case.
    Span<int>(kSpanData.begin(), kSpanData.end()); // expected-error {{no matching constructor for initialization}}

    Span<const int>{FakeRange()}; // Control case.
    Span<int>{FakeRange()}; // expected-error {{no matching constructor for initialization}}
}

void TestConstructorWithRangeRequirements() {
    Span<const int>{FakeRange()}; // Control case.

    struct FakeRangeBadSize {
        uint8_t size() const {
            return uint8_t{kSpanData.size()};
        }
        const int* data() const { return kSpanData.data(); }
    };
    Span<const int>{FakeRangeBadSize()}; // expected-error {{no matching constructor for initialization of}}

    struct FakeRangeTypedSize {
         IndexSizeT size() const {
            return IndexSizeT{kSpanData.size()};
        }
        const int* data() const { return kSpanData.data(); }
    };
    Span<const int>{FakeRangeTypedSize()}; // expected-error {{no matching constructor for initialization of}}

    struct FakeRangeBadData {
        size_t size() const {
            return kSpanData.size();
        }
        const int& data() const { return *kSpanData.data(); }
    };
    Span<const int>{FakeRangeBadData()}; // expected-error {{no matching constructor for initialization of}}
}

void TestTypedIntegerArguments() {
    ityp::span<Index, const int> sp{FakeTypedRange()};

    ityp::span<Index, const int>(kSpanData.data(), kSpanData.size()); // expected-error {{no matching constructor for initialization}}
    (void) sp.at(2); // expected-error {{no viable conversion from}}
    (void) sp[2]; // expected-error {{no viable overloaded operator[]}}
    (void) sp.first(2); // expected-error {{no viable conversion from}}
    (void) sp.last(2); // expected-error {{no viable conversion from}}
    (void) sp.subspan(2); // expected-error {{no matching member function for call to}}
    (void) sp.subspan(2, 2); // expected-error {{no matching member function for call to}}
    (void) sp.SplitAt(2); // expected-error {{no viable conversion}}
    (void) sp.TakeFirst(2); // expected-error {{no viable conversion}}
}

void TestAsWriteableBytesRequiresNonConst() {
    auto sp = Span<const int>{FakeRange()};

    SpanAsBytes(sp); // Control case
    SpanAsWritableBytes(sp); // expected-error {{no matching function for call}}
}

void TestAsBytesRetainsVolatile() {
    std::array<int, 3> ints{};
    auto sp = Span<volatile int>{ints};

    {
        // Control case
        [[maybe_unused]] Span<const volatile std::byte> vbsp = SpanAsBytes(sp);
        [[maybe_unused]] Span<volatile std::byte> vwbsp = SpanAsWritableBytes(sp);
    }

    {
        Span<const std::byte> vbsp = SpanAsBytes(sp); // expected-error {{no viable conversion from}}
        Span<std::byte> vwbsp = SpanAsWritableBytes(sp); // expected-error {{no viable conversion from}}
    }
}

void TestReinterpretSpan() {
    {
        // Non-byte source.
        std::array<int, 3> ints{};
        auto s = Span<int>{ints};
        auto r1 = ReinterpretSpan<int>(s);  // expected-error {{no matching function for call}}
        auto r2 = ReinterpretSpan<int, Index>(s);  // expected-error {{no matching function for call}}
    }
    {
        // Casting away const or volatile.
        std::array<std::byte, 4> bytes;
        {
            auto const_s = Span<const std::byte>{bytes};
            auto r1 = ReinterpretSpan<char>(const_s); // expected-error {{no matching function for call}}
            auto r2 = ReinterpretSpan<volatile char>(const_s); // expected-error {{no matching function for call}}
            auto r3 = ReinterpretSpan<char, Index>(const_s); // expected-error {{no matching function for call}}
            auto r4 = ReinterpretSpan<volatile char, Index>(const_s); // expected-error {{no matching function for call}}
        }
        {
            auto volatile_s = Span<volatile std::byte>{bytes};
            auto r1 = ReinterpretSpan<char>(volatile_s); // expected-error {{no matching function for call}}
            auto r2 = ReinterpretSpan<const char>(volatile_s); // expected-error {{no matching function for call}}
            auto r3 = ReinterpretSpan<char, Index>(volatile_s); // expected-error {{no matching function for call}}
            auto r4 = ReinterpretSpan<const char, Index>(volatile_s); // expected-error {{no matching function for call}}
        }
        {
            auto cv_s = Span<const volatile std::byte>{bytes};
            auto r1 = ReinterpretSpan<char>(cv_s); // expected-error {{no matching function for call}}
            auto r2 = ReinterpretSpan<const char>(cv_s); // expected-error {{no matching function for call}}
            auto r3 = ReinterpretSpan<volatile char>(cv_s); // expected-error {{no matching function for call}}
            auto r4 = ReinterpretSpan<char, Index>(cv_s); // expected-error {{no matching function for call}}
            auto r5 = ReinterpretSpan<const char, Index>(cv_s); // expected-error {{no matching function for call}}
            auto r6 = ReinterpretSpan<volatile char, Index>(cv_s); // expected-error {{no matching function for call}}
        }
    }
    {
        // Unless type is volatile, it needs to be trivially copyable.
        struct S {
            S() {}
            ~S() {}
            S(const volatile S&) {}
            S(volatile S&) {}
            S(const volatile S&&) {}
            char i;
        };
        std::array<std::byte, 4> bytes;
        auto s = Span<std::byte>{bytes};

        // Allowed if the type is volatile.
        [[maybe_unused]] auto r1 = ReinterpretSpan<volatile S>(s);
        [[maybe_unused]] auto r2 = ReinterpretSpan<const volatile S>(s);
        [[maybe_unused]] auto r3 = ReinterpretSpan<volatile S, Index>(s);
        [[maybe_unused]] auto r4 = ReinterpretSpan<const volatile S, Index>(s);

        // TODO(https://crbug.com/528027992): Re-enable these tests once wire serialization is volatile
        // in both directions. These should all eventually fail to compile.
        [[maybe_unused]] auto r5 = ReinterpretSpan<S>(s);
        [[maybe_unused]] auto r6 = ReinterpretSpan<const S>(s);
        [[maybe_unused]] auto r7 = ReinterpretSpan<S, Index>(s);
        [[maybe_unused]] auto r8 = ReinterpretSpan<const S, Index>(s);
    }
}

}  // namespace dawn
