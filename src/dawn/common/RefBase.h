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

#ifndef SRC_DAWN_COMMON_REFBASE_H_
#define SRC_DAWN_COMMON_REFBASE_H_

#include <type_traits>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/Compiler.h"

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
  public:
    // Default constructor and destructor.
    RefBase() : mValue(Traits::kNullValue) {}

    ~RefBase() { Release(mValue); }

    // Constructors from nullptr.
    // NOLINTNEXTLINE(runtime/explicit)
    constexpr RefBase(std::nullptr_t) : RefBase() {}

    RefBase<T, Traits>& operator=(std::nullptr_t) {
        Set(Traits::kNullValue);
        return *this;
    }

    // Constructors from a value T.
    // NOLINTNEXTLINE(runtime/explicit)
    RefBase(T value) : mValue(value) { Reference(value); }

    RefBase<T, Traits>& operator=(const T& value) {
        Set(value);
        return *this;
    }

    // Constructors from a RefBase<T>
    RefBase(const RefBase<T, Traits>& other) : mValue(other.mValue) { Reference(other.mValue); }

    RefBase<T, Traits>& operator=(const RefBase<T, Traits>& other) {
        Set(other.mValue);
        return *this;
    }

    RefBase(RefBase<T, Traits>&& other) { mValue = other.Detach(); }

    RefBase<T, Traits>& operator=(RefBase<T, Traits>&& other) {
        if (&other != this) {
            Release(mValue);
            mValue = other.Detach();
        }
        return *this;
    }

    // Constructors from a RefBase<U>. Note that in the *-assignment operators this cannot be the
    // same as `other` because overload resolution rules would have chosen the *-assignement
    // operators defined with `other` == RefBase<T, Traits>.
    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase(const RefBase<U, UTraits>& other) : mValue(other.mValue) {
        Reference(other.mValue);
    }

    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase<T, Traits>& operator=(const RefBase<U, UTraits>& other) {
        Set(other.mValue);
        return *this;
    }

    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase(RefBase<U, UTraits>&& other) {
        mValue = other.Detach();
    }

    template <typename U, typename UTraits, typename = typename std::is_convertible<U, T>::type>
    RefBase<T, Traits>& operator=(RefBase<U, UTraits>&& other) {
        Release(mValue);
        mValue = other.Detach();
        return *this;
    }

    // Comparison operators.
    bool operator==(const T& other) const { return mValue == other; }

    bool operator!=(const T& other) const { return mValue != other; }

    const T operator->() const { return mValue; }
    T operator->() { return mValue; }

    bool operator<(const RefBase<T, Traits>& other) const { return mValue < other.mValue; }

    // Smart pointer methods.
    const T& Get() const { return mValue; }
    T& Get() { return mValue; }

    [[nodiscard]] T Detach() {
        T value{std::move(mValue)};
        mValue = Traits::kNullValue;
        return value;
    }

    void Acquire(T value) {
        Release(mValue);
        mValue = value;
    }

    [[nodiscard]] T* InitializeInto() {
        ASSERT(mValue == Traits::kNullValue);
        return &mValue;
    }

  private:
    // Friend is needed so that instances of RefBase<U> can call Reference and Release on
    // RefBase<T>.
    template <typename U, typename UTraits>
    friend class RefBase;

    static void Reference(T value) {
        if (value != Traits::kNullValue) {
            Traits::Reference(value);
        }
    }
    static void Release(T value) {
        if (value != Traits::kNullValue) {
            Traits::Release(value);
        }
    }

    void Set(T value) {
        if (mValue != value) {
            // Ensure that the new value is referenced before the old is released to prevent any
            // transitive frees that may affect the new value.
            Reference(value);
            Release(mValue);
            mValue = value;
        }
    }

    T mValue;
};

#endif  // SRC_DAWN_COMMON_REFBASE_H_
