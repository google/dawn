// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TINT_NUMBER_H_
#define SRC_TINT_NUMBER_H_

#include <stdint.h>
#include <functional>

namespace tint {

/// Number wraps a integer or floating point number, enforcing explicit casting.
template <typename T>
struct Number {
    /// Constructor. The value is zero-initialized.
    Number() = default;

    /// Constructor.
    /// @param v the value to initialize this Number to
    template <typename U>
    explicit Number(U v) : value(static_cast<T>(v)) {}

    /// Constructor.
    /// @param v the value to initialize this Number to
    template <typename U>
    explicit Number(Number<U> v) : value(static_cast<T>(v.value)) {}

    /// Conversion operator
    /// @returns the value as T
    operator T() const { return value; }

    /// Negation operator
    /// @returns the negative value of the number
    Number operator-() const { return Number(-value); }

    /// Assignment operator
    /// @param v the new value
    /// @returns this Number so calls can be chained
    Number& operator=(T v) {
        value = v;
        return *this;
    }

    /// The number value
    T value = {};
};

template <typename A, typename B>
bool operator==(Number<A> a, Number<B> b) {
    using T = decltype(a.value + b.value);
    return std::equal_to<T>()(a.value, b.value);
}

template <typename A, typename B>
bool operator==(Number<A> a, B b) {
    return a == Number<B>(b);
}

template <typename A, typename B>
bool operator==(A a, Number<B> b) {
    return Number<A>(a) == b;
}

/// `i32` is a type alias to `Number<int32_t>`.
using i32 = Number<int32_t>;
/// `u32` is a type alias to `Number<uint32_t>`.
using u32 = Number<uint32_t>;
/// `f32` is a type alias to `Number<float>`
using f32 = Number<float>;

}  // namespace tint

namespace tint::number_suffixes {

/// Literal suffix for i32 literals
inline i32 operator"" _i(unsigned long long int value) {  // NOLINT
    return i32(static_cast<int32_t>(value));
}

/// Literal suffix for u32 literals
inline u32 operator"" _u(unsigned long long int value) {  // NOLINT
    return u32(static_cast<uint32_t>(value));
}

/// Literal suffix for f32 literals
inline f32 operator"" _f(long double value) {  // NOLINT
    return f32(static_cast<double>(value));
}

/// Literal suffix for f32 literals
inline f32 operator"" _f(unsigned long long int value) {  // NOLINT
    return f32(static_cast<double>(value));
}

}  // namespace tint::number_suffixes

#endif  // SRC_TINT_NUMBER_H_
