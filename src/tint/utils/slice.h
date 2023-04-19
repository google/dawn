// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_UTILS_SLICE_H_
#define SRC_TINT_UTILS_SLICE_H_

#include <cstdint>
#include <iterator>

#include "src/tint/utils/bitcast.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/traits.h"

namespace tint::utils {

/// A type used to indicate an empty array.
struct EmptyType {};

/// An instance of the EmptyType.
static constexpr EmptyType Empty;

/// Mode enumerator for ReinterpretSlice
enum class ReinterpretMode {
    /// Only upcasts of pointers are permitted
    kSafe,
    /// Potentially unsafe downcasts of pointers are also permitted
    kUnsafe,
};

namespace detail {

template <typename TO, typename FROM>
static constexpr bool ConstRemoved = std::is_const_v<FROM> && !std::is_const_v<TO>;

/// Private implementation of tint::utils::CanReinterpretSlice.
/// Specialized for the case of TO equal to FROM, which is the common case, and avoids inspection of
/// the base classes, which can be troublesome if the slice is of an incomplete type.
template <ReinterpretMode MODE, typename TO, typename FROM>
struct CanReinterpretSlice {
  private:
    using TO_EL = std::remove_pointer_t<std::decay_t<TO>>;
    using FROM_EL = std::remove_pointer_t<std::decay_t<FROM>>;

  public:
    /// @see utils::CanReinterpretSlice
    static constexpr bool value =
        // const can only be applied, not removed
        !ConstRemoved<TO, FROM> &&

        // Both TO and FROM are the same type (ignoring const)
        (std::is_same_v<std::remove_const_t<TO>, std::remove_const_t<FROM>> ||

         // Both TO and FROM are pointers...
         ((std::is_pointer_v<TO> && std::is_pointer_v<FROM>)&&

          // const can only be applied to element type, not removed
          !ConstRemoved<TO_EL, FROM_EL> &&

          // Either:
          // * Both the pointer elements are of the same type (ignoring const)
          // * Both the pointer elements are both Castable, and MODE is kUnsafe, or FROM is of,
          // or
          //   derives from TO
          (std::is_same_v<std::remove_const_t<FROM_EL>, std::remove_const_t<TO_EL>> ||
           (IsCastable<FROM_EL, TO_EL> && (MODE == ReinterpretMode::kUnsafe ||
                                           utils::traits::IsTypeOrDerived<FROM_EL, TO_EL>)))));
};

/// Specialization of 'CanReinterpretSlice' for when TO and FROM are equal types.
template <typename T, ReinterpretMode MODE>
struct CanReinterpretSlice<MODE, T, T> {
    /// Always `true` as TO and FROM are the same type.
    static constexpr bool value = true;
};

}  // namespace detail

/// Evaluates whether a `Slice<FROM>` and be reinterpreted as a `Slice<TO>`.
/// Slices can be reinterpreted if:
///  * TO has the same or more 'constness' than FROM.
///  * And either:
///  * `FROM` and `TO` are pointers to the same type
///  * `FROM` and `TO` are pointers to CastableBase (or derived), and the pointee type of `TO` is of
///     the same type as, or is an ancestor of the pointee type of `FROM`.
template <ReinterpretMode MODE, typename TO, typename FROM>
static constexpr bool CanReinterpretSlice = detail::CanReinterpretSlice<MODE, TO, FROM>::value;

/// A slice represents a contigious array of elements of type T.
template <typename T>
struct Slice {
    /// Type of `T`.
    using value_type = T;

    /// The pointer to the first element in the slice
    T* data = nullptr;

    /// The total number of elements in the slice
    size_t len = 0;

    /// The total capacity of the backing store for the slice
    size_t cap = 0;

    /// Constructor
    Slice() = default;

    /// Constructor
    Slice(EmptyType) {}  // NOLINT

    /// Constructor
    /// @param d pointer to the first element in the slice
    /// @param l total number of elements in the slice
    /// @param c total capacity of the backing store for the slice
    Slice(T* d, size_t l, size_t c) : data(d), len(l), cap(c) {}

    /// Constructor
    /// @param elements c-array of elements
    template <size_t N>
    Slice(T (&elements)[N])  // NOLINT
        : data(elements), len(N), cap(N) {}

    /// Reinterprets this slice as `const Slice<TO>&`
    /// @returns the reinterpreted slice
    /// @see CanReinterpretSlice
    template <typename TO, ReinterpretMode MODE = ReinterpretMode::kSafe>
    const Slice<TO>& Reinterpret() const {
        static_assert(CanReinterpretSlice<MODE, TO, T>);
        return *Bitcast<const Slice<TO>*>(this);
    }

    /// Reinterprets this slice as `Slice<TO>&`
    /// @returns the reinterpreted slice
    /// @see CanReinterpretSlice
    template <typename TO, ReinterpretMode MODE = ReinterpretMode::kSafe>
    Slice<TO>& Reinterpret() {
        static_assert(CanReinterpretSlice<MODE, TO, T>);
        return *Bitcast<Slice<TO>*>(this);
    }

    /// @return true if the slice length is zero
    bool IsEmpty() const { return len == 0; }

    /// Index operator
    /// @param i the element index. Must be less than `len`.
    /// @returns a reference to the i'th element.
    T& operator[](size_t i) { return data[i]; }

    /// Index operator
    /// @param i the element index. Must be less than `len`.
    /// @returns a reference to the i'th element.
    const T& operator[](size_t i) const { return data[i]; }

    /// @returns a reference to the first element in the vector
    T& Front() { return data[0]; }

    /// @returns a reference to the first element in the vector
    const T& Front() const { return data[0]; }

    /// @returns a reference to the last element in the vector
    T& Back() { return data[len - 1]; }

    /// @returns a reference to the last element in the vector
    const T& Back() const { return data[len - 1]; }

    /// @returns a pointer to the first element in the vector
    T* begin() { return data; }

    /// @returns a pointer to the first element in the vector
    const T* begin() const { return data; }

    /// @returns a pointer to one past the last element in the vector
    T* end() { return data + len; }

    /// @returns a pointer to one past the last element in the vector
    const T* end() const { return data + len; }

    /// @returns a reverse iterator starting with the last element in the vector
    auto rbegin() { return std::reverse_iterator<T*>(end()); }

    /// @returns a reverse iterator starting with the last element in the vector
    auto rbegin() const { return std::reverse_iterator<const T*>(end()); }

    /// @returns the end for a reverse iterator
    auto rend() { return std::reverse_iterator<T*>(begin()); }

    /// @returns the end for a reverse iterator
    auto rend() const { return std::reverse_iterator<const T*>(begin()); }
};

/// Deduction guide for Slice from c-array
template <typename T, size_t N>
Slice(T (&elements)[N]) -> Slice<T>;

}  // namespace tint::utils

#endif  // SRC_TINT_UTILS_SLICE_H_
