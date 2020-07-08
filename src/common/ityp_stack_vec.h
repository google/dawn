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

#ifndef COMMON_ITYP_STACK_VEC_H_
#define COMMON_ITYP_STACK_VEC_H_

#include "common/Assert.h"
#include "common/StackContainer.h"
#include "common/UnderlyingType.h"

namespace ityp {

    template <typename Index, typename Value, size_t StaticCapacity = 0>
    class stack_vec : private StackVector<Value, StaticCapacity> {
        using I = UnderlyingType<Index>;
        using Base = StackVector<Value, StaticCapacity>;
        static_assert(StaticCapacity <= std::numeric_limits<I>::max(), "");

      public:
        stack_vec() : Base() {
        }
        stack_vec(Index size) : Base() {
            this->container().resize(static_cast<I>(size));
        }

        Value& operator[](Index i) {
            ASSERT(i < size());
            return Base::operator[](static_cast<I>(i));
        }

        constexpr const Value& operator[](Index i) const {
            ASSERT(i < size());
            return Base::operator[](static_cast<I>(i));
        }

        void resize(Index size) {
            this->container().resize(static_cast<I>(size));
        }

        void reserve(Index size) {
            this->container().reserve(static_cast<I>(size));
        }

        Value* data() {
            return this->container().data();
        }

        const Value* data() const {
            return this->container().data();
        }

        Value* begin() noexcept {
            return this->container().begin();
        }

        const Value* begin() const noexcept {
            return this->container().begin();
        }

        Value* end() noexcept {
            return this->container().end();
        }

        const Value* end() const noexcept {
            return this->container().end();
        }

        Value& front() {
            return this->container().front();
        }

        const Value& front() const {
            return this->container().front();
        }

        Value& back() {
            return this->container().back();
        }

        const Value& back() const {
            return this->container().back();
        }

        Index size() const {
            return Index(static_cast<I>(this->container().size()));
        }
    };

}  // namespace ityp

#endif  // COMMON_ITYP_STACK_VEC_H_
