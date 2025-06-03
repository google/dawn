// Copyright 2025 The Dawn & Tint Authors
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

#ifndef SRC_DAWN_COMMON_BITSETRANGEITERATOR_H_
#define SRC_DAWN_COMMON_BITSETRANGEITERATOR_H_

#include <bit>
#include <bitset>
#include <limits>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Math.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn {

// Similar to BitSetIterator but returns ranges of consecutive bits as (offset, size) pairs
// TODO(crbug.com/366291600): // Specialization  for bitset size fits in uint64_t to skip
// loops for bits across words boundary.
template <size_t N, typename T>
class BitSetRangeIterator final {
  public:
    explicit BitSetRangeIterator(const std::bitset<N>& bitset);
    BitSetRangeIterator(const BitSetRangeIterator& other);
    BitSetRangeIterator& operator=(const BitSetRangeIterator& other);

    class Iterator final {
      public:
        explicit Iterator(const std::bitset<N>& bits, uint32_t offset = 0, uint32_t size = 0);
        Iterator& operator++();

        bool operator==(const Iterator& other) const = default;

        // Returns a pair of offset and size of the current range
        std::pair<T, size_t> operator*() const {
            using U = UnderlyingType<T>;
            DAWN_ASSERT(static_cast<U>(mOffset) <= std::numeric_limits<U>::max());
            DAWN_ASSERT(static_cast<size_t>(mSize) <= std::numeric_limits<size_t>::max());
            return std::make_pair(static_cast<T>(static_cast<U>(mOffset)),
                                  static_cast<size_t>(mSize));
        }

      private:
        void Advance();
        size_t ScanForwardAndShiftBits();

        static constexpr size_t kBitsPerWord = sizeof(uint64_t) * 8;
        std::bitset<N> mBits;
        uint32_t mOffset{0};
        uint32_t mSize{0};
    };

    Iterator begin() const { return Iterator(mBits); }
    Iterator end() const { return Iterator(std::bitset<N>(0), N, 0); }

  private:
    const std::bitset<N> mBits;
};

template <size_t N, typename T>
BitSetRangeIterator<N, T>::BitSetRangeIterator(const std::bitset<N>& bitset) : mBits(bitset) {}

template <size_t N, typename T>
BitSetRangeIterator<N, T>::BitSetRangeIterator(const BitSetRangeIterator& other)
    : mBits(other.mBits) {}

template <size_t N, typename T>
BitSetRangeIterator<N, T>& BitSetRangeIterator<N, T>::operator=(const BitSetRangeIterator& other) {
    mBits = other.mBits;
    return *this;
}

template <size_t N, typename T>
BitSetRangeIterator<N, T>::Iterator::Iterator(const std::bitset<N>& bits,
                                              uint32_t offset,
                                              uint32_t size)
    : mBits(bits), mOffset(offset), mSize(size) {
    Advance();
}

template <size_t N, typename T>
typename BitSetRangeIterator<N, T>::Iterator& BitSetRangeIterator<N, T>::Iterator::operator++() {
    Advance();
    return *this;
}

template <size_t N, typename T>
size_t BitSetRangeIterator<N, T>::Iterator::ScanForwardAndShiftBits() {
    if (mBits.none()) {
        return N;  // Or some other indicator that there are no bits.
    }

    constexpr std::bitset<N> wordMask(std::numeric_limits<uint64_t>::max());
    size_t offset = 0;
    while ((mBits & wordMask).to_ullong() == 0) {
        offset += kBitsPerWord;
        mBits >>= kBitsPerWord;
    }

    size_t nextBit = static_cast<size_t>(
        std::countr_zero(static_cast<uint64_t>((mBits & wordMask).to_ullong())));
    mBits >>= nextBit;
    return offset + nextBit;
}

template <size_t N, typename T>
void BitSetRangeIterator<N, T>::Iterator::Advance() {
    // Bits are currently shifted to mOffset + mSize.
    mOffset += mSize;

    size_t rangeStart = ScanForwardAndShiftBits();
    if (rangeStart == N) {
        // Reached the end, no more ranges.
        mOffset = N;
        mSize = 0;
        return;
    }

    mOffset += rangeStart;
    mBits = ~mBits;

    size_t rangeCount = ScanForwardAndShiftBits();
    if (rangeCount == N) {
        // All bits until the end of the set are set.
        rangeCount = N - mOffset;
    }

    mSize = rangeCount;
    mBits = ~mBits;

    // Clear the bits for the current range.
    mBits <<= rangeCount;
    mBits >>= rangeCount;
}

// Helper to avoid needing to specify the template parameter size
template <size_t N>
BitSetRangeIterator<N, uint32_t> IterateBitSetRanges(const std::bitset<N>& bitset) {
    return BitSetRangeIterator<N, uint32_t>(bitset);
}

}  // namespace dawn

#endif  // SRC_DAWN_COMMON_BITSETRANGEITERATOR_H_
