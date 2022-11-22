// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_BITSET_H_
#define SRC_TINT_UTILS_BITSET_H_

#include <stdint.h>

#include "src/tint/utils/vector.h"

namespace tint::utils {

/// Bitset is a dynamically sized, vector of bits, packed into integer words.
/// Bits can be individually read and written using the index operator.
///
/// Bitset will fit at least `N` bits internally before spilling to heap allocations.
template <size_t N = 0>
class Bitset {
    /// The integer word type used to hold the bits
    using Word = size_t;
    /// Number of bits per word
    static constexpr size_t kWordBits = sizeof(Word) * 8;

    /// Number of words required to hold the number of bits
    static constexpr size_t NumWords(size_t num_bits) {
        return ((num_bits + kWordBits - 1) / kWordBits);
    }

  public:
    /// Constructor
    Bitset() = default;

    /// Destructor
    ~Bitset() = default;

    /// Accessor for a single bit
    struct Bit {
        /// The word that contains the bit
        Word& word;
        /// A word with a single bit set, which masks the targetted bit
        Word const mask;

        /// Assignment operator
        /// @param value the new value for the bit
        /// @returns this Bit so calls can be chained
        const Bit& operator=(bool value) const {
            if (value) {
                word = word | mask;
            } else {
                word = word & ~mask;
            }
            return *this;
        }

        /// Conversion operator
        /// @returns the bit value
        operator bool() const { return (word & mask) != 0; }
    };

    /// @param new_len the new size of the bitmap, in bits.
    void Resize(size_t new_len) {
        vec_.Resize(NumWords(new_len));

        // Clear any potentially set bits that are in the top part of the word
        if (size_t high_bit = new_len % kWordBits; high_bit > 0) {
            vec_.Back() &= (static_cast<Word>(1) << high_bit) - 1;
        }

        len_ = new_len;
    }

    /// @return the number of bits in the bitset.
    size_t Length() const { return len_; }

    /// Index operator
    /// @param index the index of the bit to access
    /// @return the accessor for the indexed bit
    Bit operator[](size_t index) {
        auto& word = vec_[index / kWordBits];
        auto mask = static_cast<Word>(1) << (index % kWordBits);
        return Bit{word, mask};
    }

    /// Const index operator
    /// @param index the index of the bit to access
    /// @return bool value of the indexed bit
    bool operator[](size_t index) const {
        const auto& word = vec_[index / kWordBits];
        auto mask = static_cast<Word>(1) << (index % kWordBits);
        return word & mask;
    }

    /// @returns true iff the all bits are unset (0)
    bool AllBitsZero() const {
        for (auto word : vec_) {
            if (word) {
                return false;
            }
        }
        return true;
    }

  private:
    Vector<size_t, NumWords(N)> vec_;
    size_t len_ = 0;
};

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_BITSET_H_
