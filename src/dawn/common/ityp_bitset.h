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

#ifndef SRC_DAWN_COMMON_ITYP_BITSET_H_
#define SRC_DAWN_COMMON_ITYP_BITSET_H_

#include <bitset>
#include <limits>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn {
namespace ityp {

// ityp::bitset is a helper class that wraps std::bitset with the restriction that
// indices must be a particular type |Index|.
template <typename Index, size_t N>
class bitset : private ::std::bitset<N> {
    using I = UnderlyingType<Index>;
    using Base = ::std::bitset<N>;

    static_assert(sizeof(I) <= sizeof(size_t));

    explicit constexpr bitset(const Base& rhs) : Base(rhs) {}

  public:
    class Iterator final {
      public:
        explicit Iterator(const std::bitset<N>& bits);
        Iterator& operator++();

        bool operator==(const Iterator& other) const;
        bool operator!=(const Iterator& other) const;

        Index operator*() const;

      private:
        uint32_t getNextBit();

        static constexpr size_t kBitsPerWord = sizeof(uint32_t) * 8;
        std::bitset<N> mBits;
        uint32_t mCurrentBit{0};
        uint32_t mOffset{0};
    };

    Iterator begin() const { return Iterator(*this); }
    Iterator end() const { return Iterator(std::bitset<N>(0)); }

    const Base& AsBase() const { return static_cast<const Base&>(*this); }
    Base& AsBase() { return static_cast<Base&>(*this); }

    using reference = typename Base::reference;

    constexpr bitset() noexcept : Base() {}

    // NOLINTNEXTLINE(runtime/explicit)
    constexpr bitset(uint64_t value) noexcept : Base(value) {}

    constexpr bool operator[](Index i) const { return Base::operator[](static_cast<I>(i)); }

    typename Base::reference operator[](Index i) { return Base::operator[](static_cast<I>(i)); }

    bool test(Index i) const { return Base::test(static_cast<I>(i)); }

    using Base::all;
    using Base::any;
    using Base::count;
    using Base::none;
    using Base::size;

    bool operator==(const bitset& other) const noexcept {
        return Base::operator==(static_cast<const Base&>(other));
    }

    bool operator!=(const bitset& other) const noexcept {
        return !Base::operator==(static_cast<const Base&>(other));
    }

    bitset& operator&=(const bitset& other) noexcept {
        return static_cast<bitset&>(Base::operator&=(static_cast<const Base&>(other)));
    }

    bitset& operator|=(const bitset& other) noexcept {
        return static_cast<bitset&>(Base::operator|=(static_cast<const Base&>(other)));
    }

    bitset& operator^=(const bitset& other) noexcept {
        return static_cast<bitset&>(Base::operator^=(static_cast<const Base&>(other)));
    }

    bitset operator~() const noexcept { return bitset(*this).flip(); }

    bitset& set() noexcept { return static_cast<bitset&>(Base::set()); }

    bitset& set(Index i, bool value = true) {
        return static_cast<bitset&>(Base::set(static_cast<I>(i), value));
    }

    bitset& reset() noexcept { return static_cast<bitset&>(Base::reset()); }

    bitset& reset(Index i) { return static_cast<bitset&>(Base::reset(static_cast<I>(i))); }

    bitset& flip() noexcept { return static_cast<bitset&>(Base::flip()); }

    bitset& flip(Index i) { return static_cast<bitset&>(Base::flip(static_cast<I>(i))); }

    using Base::to_string;
    using Base::to_ullong;
    using Base::to_ulong;

    friend bitset operator&(const bitset& lhs, const bitset& rhs) noexcept {
        return bitset(static_cast<const Base&>(lhs) & static_cast<const Base&>(rhs));
    }

    friend bitset operator|(const bitset& lhs, const bitset& rhs) noexcept {
        return bitset(static_cast<const Base&>(lhs) | static_cast<const Base&>(rhs));
    }

    friend bitset operator^(const bitset& lhs, const bitset& rhs) noexcept {
        return bitset(static_cast<const Base&>(lhs) ^ static_cast<const Base&>(rhs));
    }

    friend BitSetIterator<N, Index> IterateBitSet(const bitset& bitset) {
        return BitSetIterator<N, Index>(static_cast<const Base&>(bitset));
    }

    friend struct std::hash<bitset>;
};

template <typename Index, size_t N>
bitset<Index, N>::Iterator::Iterator(const std::bitset<N>& bits) : mBits(bits) {
    if (bits.any()) {
        mCurrentBit = getNextBit();
    } else {
        mOffset = static_cast<uint32_t>(RoundUp(N, kBitsPerWord));
    }
}

template <typename Index, size_t N>
typename bitset<Index, N>::Iterator& bitset<Index, N>::Iterator::operator++() {
    DAWN_ASSERT(mBits.any());
    mBits.set(mCurrentBit - mOffset, 0);
    mCurrentBit = getNextBit();
    return *this;
}

template <typename Index, size_t N>
bool bitset<Index, N>::Iterator::operator==(const Iterator& other) const {
    return mOffset == other.mOffset && mBits == other.mBits;
}

template <typename Index, size_t N>
bool bitset<Index, N>::Iterator::operator!=(const Iterator& other) const {
    return !(*this == other);
}

template <typename Index, size_t N>
Index bitset<Index, N>::Iterator::operator*() const {
    using U = UnderlyingType<Index>;
    DAWN_ASSERT(static_cast<U>(mCurrentBit) <= std::numeric_limits<U>::max());
    return static_cast<Index>(static_cast<U>(mCurrentBit));
}

template <typename Index, size_t N>
uint32_t bitset<Index, N>::Iterator::getNextBit() {
    static std::bitset<N> wordMask(std::numeric_limits<uint32_t>::max());

    while (mOffset < N) {
        uint32_t wordBits = static_cast<uint32_t>((mBits & wordMask).to_ulong());
        if (wordBits != 0ul) {
            return ScanForward(wordBits) + mOffset;
        }

        mBits >>= kBitsPerWord;
        mOffset += kBitsPerWord;
    }
    return 0;
}

}  // namespace ityp

// Assume we have bitset of at most 64 bits
// Returns i which is the next integer of the index of the highest bit
// i == 0 if there is no bit set to true
// i == 1 if only the least significant bit (at index 0) is the bit set to true with the
// highest index
// ...
// i == 64 if the most significant bit (at index 64) is the bit set to true with the highest
// index
template <typename Index, size_t N>
Index GetHighestBitIndexPlusOne(const ityp::bitset<Index, N>& bitset) {
    using I = UnderlyingType<Index>;
#if DAWN_COMPILER_IS(MSVC)
    if constexpr (N > 32) {
#if DAWN_PLATFORM_IS(64_BIT)
        // NOLINTNEXTLINE(runtime/int)
        unsigned long firstBitIndex = 0ul;
        unsigned char ret = _BitScanReverse64(&firstBitIndex, bitset.to_ullong());
        if (ret == 0) {
            return Index(static_cast<I>(0));
        }
        return Index(static_cast<I>(firstBitIndex + 1));
#else   // DAWN_PLATFORM_IS(64_BIT)
        if (bitset.none()) {
            return Index(static_cast<I>(0));
        }
        for (size_t i = 0u; i < N; i++) {
            if (bitset.test(Index(static_cast<I>(N - 1 - i)))) {
                return Index(static_cast<I>(N - i));
            }
        }
        DAWN_UNREACHABLE();
#endif  // DAWN_PLATFORM_IS(64_BIT)
    } else {
        // NOLINTNEXTLINE(runtime/int)
        unsigned long firstBitIndex = 0ul;
        unsigned char ret = _BitScanReverse(&firstBitIndex, bitset.to_ulong());
        if (ret == 0) {
            return Index(static_cast<I>(0));
        }
        return Index(static_cast<I>(firstBitIndex + 1));
    }
#else   // DAWN_COMPILER_IS(MSVC)
    if (bitset.none()) {
        return Index(static_cast<I>(0));
    }
    if constexpr (N > 32) {
        return Index(
            static_cast<I>(64 - static_cast<uint32_t>(__builtin_clzll(bitset.to_ullong()))));
    } else {
        return Index(static_cast<I>(32 - static_cast<uint32_t>(__builtin_clz(bitset.to_ulong()))));
    }
#endif  // DAWN_COMPILER_IS(MSVC)
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_ITYP_BITSET_H_
