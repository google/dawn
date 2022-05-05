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

#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/type.h"

namespace tint::sem {

/// A Constant is compile-time known expression value, expressed as a flattened
/// list of scalar values. Value may be of a scalar or vector type.
class Constant {
  public:
    /// Scalar holds a single constant scalar value, as a union of an i32, u32,
    /// f32 or boolean.
    union Scalar {
        /// The scalar value as a i32
        tint::i32 i32;
        /// The scalar value as a u32
        tint::u32 u32;
        /// The scalar value as a f32
        tint::f32 f32;
        /// The scalar value as a bool
        bool bool_;

        /// Constructs the scalar with the i32 value `v`
        /// @param v the value of the Scalar
        Scalar(tint::i32 v) : i32(v) {}  // NOLINT

        /// Constructs the scalar with the u32 value `v`
        /// @param v the value of the Scalar
        Scalar(tint::u32 v) : u32(v) {}  // NOLINT

        /// Constructs the scalar with the f32 value `v`
        /// @param v the value of the Scalar
        Scalar(tint::f32 v) : f32(v) {}  // NOLINT

        /// Constructs the scalar with the bool value `v`
        /// @param v the value of the Scalar
        Scalar(bool v) : bool_(v) {}  // NOLINT
    };

    /// Scalars is a list of scalar values
    using Scalars = std::vector<Scalar>;

    /// Constructs an invalid Constant
    Constant();

    /// Constructs a Constant of the given type and element values
    /// @param ty the Constant type
    /// @param els the Constant element values
    Constant(const Type* ty, Scalars els);

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

    /// @returns the element type of the Constant
    const sem::Type* ElementType() const { return elem_type_; }

    /// @returns the constant's scalar elements
    const Scalars& Elements() const { return elems_; }

    /// @returns true if any scalar element is zero
    bool AnyZero() const;

    /// Calls `func(s)` with s being the current scalar value at `index`.
    /// `func` is typically a lambda of the form '[](auto&& s)'.
    /// @param index the index of the scalar value
    /// @param func a function with signature `T(S)`
    /// @return the value returned by func.
    template <typename Func>
    auto WithScalarAt(size_t index, Func&& func) const {
        return Switch(
            ElementType(),  //
            [&](const I32*) { return func(elems_[index].i32); },
            [&](const U32*) { return func(elems_[index].u32); },
            [&](const F32*) { return func(elems_[index].f32); },
            [&](const Bool*) { return func(elems_[index].bool_); },
            [&](Default) {
                diag::List diags;
                TINT_UNREACHABLE(Semantic, diags)
                    << "invalid scalar type " << type_->TypeInfo().name;
                return func(u32(0u));
            });
    }

    /// @param index the index of the scalar value
    /// @return the value of the scalar `static_cast` to type T.
    template <typename T>
    T ElementAs(size_t index) const {
        return WithScalarAt(index, [](auto val) { return static_cast<T>(val); });
    }

  private:
    const sem::Type* type_ = nullptr;
    const sem::Type* elem_type_ = nullptr;
    Scalars elems_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_CONSTANT_H_
