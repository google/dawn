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

#ifndef COMMON_REFBASE_H_
#define COMMON_REFBASE_H_

#include "common/Assert.h"
#include "common/Compiler.h"

#include <type_traits>

// A common class for various smart-pointers acting on referenceable/releasable pointer-like
// objects. Logic for each specialization can be customized using a Traits type that looks
// like the following:
//
//   struct {
//      static constexpr T kNullValue = ...;
//      static void Reference(T value) { ... }
//      static void Release(T value) { ... }
//   };
//
// RefBase supports
template <typename T, typename Traits>
class RefBase {
  private:
    static constexpr T kNullValue = Traits::kNullValue;

  public:
    // Default constructor and destructor.
    RefBase() : mValue(kNullValue) {
    }

    ~RefBase() {
        Release();
        mValue = kNullValue;
    }

    // Constructors from nullptr.
    constexpr RefBase(std::nullptr_t) : RefBase() {
    }

    RefBase<T, Traits>& operator=(std::nullptr_t) {
        Release();
        mValue = kNullValue;
        return *this;
    }

    // Constructors from a value T.
    RefBase(T value) : mValue(value) {
        Reference();
    }

    RefBase<T, Traits>& operator=(const T& value) {
        mValue = value;
        Reference();
        return *this;
    }

    // Constructors from a RefBase<T>
    RefBase(const RefBase<T, Traits>& other) : mValue(other.mValue) {
        Reference();
    }

    RefBase<T, Traits>& operator=(const RefBase<T, Traits>& other) {
        if (&other != this) {
            other.Reference();
            Release();
            mValue = other.mValue;
        }

        return *this;
    }

    RefBase(RefBase<T, Traits>&& other) {
        mValue = other.mValue;
        other.mValue = kNullValue;
    }

    RefBase<T, Traits>& operator=(RefBase<T, Traits>&& other) {
        if (&other != this) {
            Release();
            mValue = other.mValue;
            other.mValue = kNullValue;
        }

        return *this;
    }

    // Constructors from a RefBase<U>. Note that in the *-assignment operators this cannot be the
    // same as `other` because overload resolution rules would have chosen the *-assignement
    // operators defined with `other` == RefBase<T, Traits>.
    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase(const RefBase<U, UTraits>& other) : mValue(other.mValue) {
        Reference();
    }

    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase<T, Traits>& operator=(const RefBase<U, UTraits>& other) {
        other.Reference();
        Release();
        mValue = other.mValue;
        return *this;
    }

    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase(RefBase<U, UTraits>&& other) {
        mValue = other.Detach();
    }

    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase<T, Traits>& operator=(RefBase<U, UTraits>&& other) {
        Release();
        mValue = other.Detach();
        return *this;
    }

    // Comparison operators.
    bool operator==(const T& other) const {
        return mValue == other;
    }

    bool operator!=(const T& other) const {
        return mValue != other;
    }

    const T operator->() const {
        return mValue;
    }
    T operator->() {
        return mValue;
    }

    // Smart pointer methods.
    const T& Get() const {
        return mValue;
    }
    T& Get() {
        return mValue;
    }

    T Detach() DAWN_NO_DISCARD {
        T value = mValue;
        mValue = kNullValue;
        return value;
    }

    void Acquire(T value) {
        Release();
        mValue = value;
    }

    T* InitializeInto() DAWN_NO_DISCARD {
        ASSERT(mValue == kNullValue);
        return &mValue;
    }

  private:
    // Friend is needed so that instances of RefBase<U> can call Reference and Release on
    // RefBase<T>.
    template <typename U, typename UTraits>
    friend class RefBase;

    void Reference() const {
        if (mValue != kNullValue) {
            Traits::Reference(mValue);
        }
    }
    void Release() const {
        if (mValue != kNullValue) {
            Traits::Release(mValue);
        }
    }

    T mValue;
};

#endif  // COMMON_REFBASE_H_
