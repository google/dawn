// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_CONSTANT_H_
#define SRC_TINT_SEM_CONSTANT_H_

#include <ostream>
// TODO(https://crbug.com/dawn/1379) Update cpplint and remove NOLINT
#include <utility>
#include <variant>  // NOLINT(build/include_order)
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/type.h"

namespace tint::sem {

/// A Constant holds a compile-time evaluated expression value, expressed as a flattened list of
/// element values. The expression type may be of an abstract-numeric, scalar, vector or matrix
/// type. Constant holds the element values in either a vector of abstract-integer (AInt) or
/// abstract-float (AFloat), depending on the element type.
class Constant {
  public:
    /// AInts is a vector of AInt, used to hold elements of the WGSL types:
    /// * abstract-integer
    /// * i32
    /// * u32
    /// * bool (0 or 1)
    using AInts = std::vector<AInt>;

    /// AFloats is a vector of AFloat, used to hold elements of the WGSL types:
    /// * abstract-float
    /// * f32
    /// * f16
    using AFloats = std::vector<AFloat>;

    /// Elements is either a vector of AInts or AFloats
    using Elements = std::variant<AInts, AFloats>;

    /// Helper that resolves to either AInt or AFloat based on the element type T.
    template <typename T>
    using ElementFor = std::conditional_t<IsFloatingPoint<UnwrapNumber<T>>, AFloat, AInt>;

    /// Helper that resolves to either AInts or AFloats based on the element type T.
    template <typename T>
    using ElementVectorFor = std::conditional_t<IsFloatingPoint<UnwrapNumber<T>>, AFloats, AInts>;

    /// Constructs an invalid Constant
    Constant();

    /// Constructs a Constant of the given type and element values
    /// @param ty the Constant type
    /// @param els the Constant element values
    Constant(const sem::Type* ty, Elements els);

    /// Constructs a Constant of the given type and element values
    /// @param ty the Constant type
    /// @param vec the Constant element values
    Constant(const sem::Type* ty, AInts vec);

    /// Constructs a Constant of the given type and element values
    /// @param ty the Constant type
    /// @param vec the Constant element values
    Constant(const sem::Type* ty, AFloats vec);

    /// Constructs a Constant of the given type and element values
    /// @param ty the Constant type
    /// @param els the Constant element values
    template <typename T>
    Constant(const sem::Type* ty, std::initializer_list<T> els);

    /// Copy constructor
    Constant(const Constant&);

    /// Destructor
    ~Constant();

    /// Copy assignment
    /// @param other the Constant to copy
    /// @returns this Constant
    Constant& operator=(const Constant& other);

    /// @returns true if the Constant has been initialized
    bool IsValid() const { return type_ != nullptr; }

    /// @return true if the Constant has been initialized
    operator bool() const { return IsValid(); }

    /// @returns the type of the Constant
    const sem::Type* Type() const { return type_; }

    /// @returns the number of elements
    size_t ElementCount() const {
        return std::visit([](auto&& v) { return v.size(); }, elems_);
    }

    /// @returns the element type of the Constant
    const sem::Type* ElementType() const { return elem_type_; }

    /// @returns the constant's elements
    const Elements& GetElements() const { return elems_; }

    /// WithElements calls the function `f` with the vector of elements as either AFloats or AInts
    /// @param f a function-like with the signature `R(auto&&)`.
    /// @returns the result of calling `f`.
    template <typename F>
    auto WithElements(F&& f) const {
        return std::visit(std::forward<F>(f), elems_);
    }

    /// WithElements calls the function `f` with the element vector as either AFloats or AInts
    /// @param f a function-like with the signature `R(auto&&)`.
    /// @returns the result of calling `f`.
    template <typename F>
    auto WithElements(F&& f) {
        return std::visit(std::forward<F>(f), elems_);
    }

    /// @returns the elements as a vector of AInt
    inline const AInts& IElements() const { return std::get<AInts>(elems_); }

    /// @returns the elements as a vector of AFloat
    inline const AFloats& FElements() const { return std::get<AFloats>(elems_); }

    /// @returns true if any element is positive zero
    bool AnyZero() const;

    /// @returns true if all elements are positive zero
    bool AllZero() const;

    /// @returns true if all elements are the same value, with the same sign-bit.
    bool AllEqual() const { return AllEqual(0, ElementCount()); }

    /// @param start the first element index
    /// @param end one past the last element index
    /// @returns true if all elements between `[start, end)` are the same value
    bool AllEqual(size_t start, size_t end) const;

    /// @param index the index of the element
    /// @return the element at `index`, which must be of type `T`.
    template <typename T>
    T Element(size_t index) const;

  private:
    /// Checks that the provided type matches the number of expected elements.
    /// @returns the element type of `ty`.
    const sem::Type* CheckElemType(const sem::Type* ty, size_t num_elements);

    const sem::Type* type_ = nullptr;
    const sem::Type* elem_type_ = nullptr;
    Elements elems_;
};

template <typename T>
Constant::Constant(const sem::Type* ty, std::initializer_list<T> els)
    : type_(ty), elem_type_(CheckElemType(type_, els.size())) {
    ElementVectorFor<T> elements;
    elements.reserve(els.size());
    for (auto el : els) {
        elements.emplace_back(ElementFor<T>(el));
    }
    elems_ = Elements{std::move(elements)};
}

template <typename T>
T Constant::Element(size_t index) const {
    if constexpr (std::is_same_v<ElementVectorFor<T>, AFloats>) {
        return static_cast<T>(FElements()[index].value);
    } else {
        return static_cast<T>(IElements()[index].value);
    }
}

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_CONSTANT_H_
