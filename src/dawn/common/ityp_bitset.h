// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_COMMON_ITYP_BITSET_H_
#define SRC_DAWN_COMMON_ITYP_BITSET_H_

#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Platform.h"
#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"

namespace ityp {

// ityp::bitset is a helper class that wraps std::bitset with the restriction that
// indices must be a particular type |Index|.
template <typename Index, size_t N>
class bitset : private std::bitset<N> {
    using I = UnderlyingType<Index>;
    using Base = std::bitset<N>;

    static_assert(sizeof(I) <= sizeof(size_t));

    explicit constexpr bitset(const Base& rhs) : Base(rhs) {}

  public:
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
        UNREACHABLE();
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

#endif  // SRC_DAWN_COMMON_ITYP_BITSET_H_
