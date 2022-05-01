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

#ifndef SRC_DAWN_NATIVE_ENUMMASKITERATOR_H_
#define SRC_DAWN_NATIVE_ENUMMASKITERATOR_H_

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/EnumClassBitmasks.h"

namespace dawn::native {

template <typename T>
class EnumMaskIterator final {
    static constexpr size_t N = EnumBitmaskSize<T>::value;
    static_assert(N > 0);

    using U = std::underlying_type_t<T>;

  public:
    explicit EnumMaskIterator(const T& mask)
        : mBitSetIterator(std::bitset<N>(static_cast<U>(mask))) {
        // If you hit this ASSERT it means that you forgot to update EnumBitmaskSize<T>::value;
        ASSERT(U(mask) == 0 || Log2(uint64_t(U(mask))) < N);
    }

    class Iterator final {
      public:
        explicit Iterator(const typename BitSetIterator<N, U>::Iterator& iter) : mIter(iter) {}

        Iterator& operator++() {
            ++mIter;
            return *this;
        }

        bool operator==(const Iterator& other) const { return mIter == other.mIter; }

        bool operator!=(const Iterator& other) const { return mIter != other.mIter; }

        T operator*() const {
            U value = *mIter;
            return static_cast<T>(U(1) << value);
        }

      private:
        typename BitSetIterator<N, U>::Iterator mIter;
    };

    Iterator begin() const { return Iterator(mBitSetIterator.begin()); }

    Iterator end() const { return Iterator(mBitSetIterator.end()); }

  private:
    BitSetIterator<N, U> mBitSetIterator;
};

template <typename T>
EnumMaskIterator<T> IterateEnumMask(const T& mask) {
    return EnumMaskIterator<T>(mask);
}

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_ENUMMASKITERATOR_H_
