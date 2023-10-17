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

#ifndef SRC_DAWN_COMMON_ITYP_STACK_VEC_H_
#define SRC_DAWN_COMMON_ITYP_STACK_VEC_H_

#include <limits>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/StackContainer.h"
#include "dawn/common/UnderlyingType.h"

namespace dawn::ityp {

template <typename Index, typename Value, size_t StaticCapacity>
class stack_vec : private StackVector<Value, StaticCapacity> {
    using I = UnderlyingType<Index>;
    using Base = StackVector<Value, StaticCapacity>;
    using VectorBase = std::vector<Value, StackAllocator<Value, StaticCapacity>>;
    static_assert(StaticCapacity <= std::numeric_limits<I>::max());

  public:
    stack_vec() : Base() {}
    explicit stack_vec(Index size) : Base() { this->container().resize(static_cast<I>(size)); }

    Value& operator[](Index i) {
        DAWN_ASSERT(i < size());
        return Base::operator[](static_cast<I>(i));
    }

    constexpr const Value& operator[](Index i) const {
        DAWN_ASSERT(i < size());
        return Base::operator[](static_cast<I>(i));
    }

    void resize(Index size) { this->container().resize(static_cast<I>(size)); }

    void reserve(Index size) { this->container().reserve(static_cast<I>(size)); }

    Value* data() { return this->container().data(); }

    const Value* data() const { return this->container().data(); }

    typename VectorBase::iterator begin() noexcept { return this->container().begin(); }

    typename VectorBase::const_iterator begin() const noexcept { return this->container().begin(); }

    typename VectorBase::iterator end() noexcept { return this->container().end(); }

    typename VectorBase::const_iterator end() const noexcept { return this->container().end(); }

    typename VectorBase::reference front() { return this->container().front(); }

    typename VectorBase::const_reference front() const { return this->container().front(); }

    typename VectorBase::reference back() { return this->container().back(); }

    typename VectorBase::const_reference back() const { return this->container().back(); }

    Index size() const { return Index(static_cast<I>(this->container().size())); }
};

}  // namespace dawn::ityp

#endif  // SRC_DAWN_COMMON_ITYP_STACK_VEC_H_
