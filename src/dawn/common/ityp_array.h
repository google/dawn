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

#ifndef SRC_DAWN_COMMON_ITYP_ARRAY_H_
#define SRC_DAWN_COMMON_ITYP_ARRAY_H_

#include <array>
#include <cstddef>
#include <limits>

#include "src/utils/numeric.h"
#include "src/utils/underlying_type.h"

namespace dawn::ityp {

// ityp::array is a helper class that wraps std::array with the restriction that
// indices must be a particular type |Index|. Dawn uses multiple flat maps of
// index-->data, and this class helps ensure an indices cannot be passed interchangably
// to a flat map of a different type.
template <typename Index, typename Value, size_t Size>
class array {
    using I = UnderlyingType<Index>;

    static_assert(HasUnsignedUnderlyingType<Index>, "Index type must be unsigned");
    static_assert(Size <= std::numeric_limits<I>::max());
    static_assert(Size <= std::numeric_limits<size_t>::max());

  public:
    // In order to have the same initialization rules as std::array, ityp::array needs to be an
    // "aggregate". This requires it to have no constructors, and thus the underlying data member
    // needs to be public in order to be initializable. (std::array is the same way but with a T[].)
    //
    // In particular, this allows us to delegate adherence to cppcoreguidelines-pro-type-member-init
    // (or NOLINTing) to instantiators of ityp::array, rather than to the definition of ityp::array.
    // In order to get good static analysis without this, ityp::array would have to deviate from the
    // interface of std::array by initializing its memory by default, and having an explicit opt-out
    // like HeapArray::Uninit().
    ::std::array<Value, Size> mPrivate;

    // Methods that are exactly like std::array

    constexpr bool operator==(const array<Index, Value, Size>& other) const {
        return mPrivate == other.mPrivate;
    }

    constexpr auto begin() { return mPrivate.begin(); }
    constexpr auto begin() const { return mPrivate.begin(); }
    constexpr auto end() { return mPrivate.end(); }
    constexpr auto end() const { return mPrivate.end(); }

    constexpr auto& front() { return mPrivate.front(); }
    constexpr const auto& front() const { return mPrivate.front(); }
    constexpr auto& back() { return mPrivate.back(); }
    constexpr const auto& back() const { return mPrivate.back(); }

    constexpr auto data() { return mPrivate.data(); }
    constexpr auto data() const { return mPrivate.data(); }
    constexpr auto empty() { return mPrivate.empty(); }
    constexpr auto empty() const { return mPrivate.empty(); }
    constexpr void fill(const Value& value) { mPrivate.fill(value); }

    // Methods that are like std::array but with typed indices

    constexpr Value& operator[](Index i) { return mPrivate[checked_cast<size_t>(i)]; }
    constexpr const Value& operator[](Index i) const { return mPrivate[checked_cast<size_t>(i)]; }

    constexpr Value& at(Index i) { return mPrivate.at(checked_cast<size_t>(i)); }
    constexpr const Value& at(Index i) const { return mPrivate.at(checked_cast<size_t>(i)); }

    constexpr Index size() const { return Index(static_cast<I>(mPrivate.size())); }
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_ARRAY_H_
