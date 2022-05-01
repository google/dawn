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

#ifndef SRC_DAWN_COMMON_ITYP_SPAN_H_
#define SRC_DAWN_COMMON_ITYP_SPAN_H_

#include <type_traits>

#include "dawn/common/TypedInteger.h"
#include "dawn/common/UnderlyingType.h"

namespace ityp {

// ityp::span is a helper class that wraps an unowned packed array of type |Value|.
// It stores the size and pointer to first element. It has the restriction that
// indices must be a particular type |Index|. This provides a type-safe way to index
// raw pointers.
template <typename Index, typename Value>
class span {
    using I = UnderlyingType<Index>;

  public:
    constexpr span() : mData(nullptr), mSize(0) {}
    constexpr span(Value* data, Index size) : mData(data), mSize(size) {}

    constexpr Value& operator[](Index i) const {
        ASSERT(i < mSize);
        return mData[static_cast<I>(i)];
    }

    Value* data() noexcept { return mData; }

    const Value* data() const noexcept { return mData; }

    Value* begin() noexcept { return mData; }

    const Value* begin() const noexcept { return mData; }

    Value* end() noexcept { return mData + static_cast<I>(mSize); }

    const Value* end() const noexcept { return mData + static_cast<I>(mSize); }

    Value& front() {
        ASSERT(mData != nullptr);
        ASSERT(static_cast<I>(mSize) >= 0);
        return *mData;
    }

    const Value& front() const {
        ASSERT(mData != nullptr);
        ASSERT(static_cast<I>(mSize) >= 0);
        return *mData;
    }

    Value& back() {
        ASSERT(mData != nullptr);
        ASSERT(static_cast<I>(mSize) >= 0);
        return *(mData + static_cast<I>(mSize) - 1);
    }

    const Value& back() const {
        ASSERT(mData != nullptr);
        ASSERT(static_cast<I>(mSize) >= 0);
        return *(mData + static_cast<I>(mSize) - 1);
    }

    Index size() const { return mSize; }

  private:
    Value* mData;
    Index mSize;
};

}  // namespace ityp

#endif  // SRC_DAWN_COMMON_ITYP_SPAN_H_
