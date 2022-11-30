// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_IR_CONSTANT_H_
#define SRC_TINT_IR_CONSTANT_H_

#include <ostream>
#include <variant>

#include "src/tint/ir/value.h"
#include "src/tint/number.h"

namespace tint::ir {

/// Constant in the IR. The constant can be one of several types these include, but aren't limited
/// to, `f32`, `u32`, `bool`. The type of the constant determines the type of data stored.
class Constant : public Castable<Constant, Value> {
  public:
    /// The type of the constant
    enum class Kind {
        /// A f32 constant
        kF32,
        /// A f16 constant
        kF16,
        /// An i32 constant
        kI32,
        /// A u32 constant
        kU32,
        /// A boolean constant
        kBool,
    };

    /// Constructor
    /// @param b the `bool` constant to store in the constant
    explicit Constant(bool b);

    /// Constructor
    /// @param f the `f32` constant to store in the constant
    explicit Constant(f32 f);

    /// Constructor
    /// @param f the `f16` constant to store in the constant
    explicit Constant(f16 f);

    /// Constructor
    /// @param u the `u32` constant to store in the constant
    explicit Constant(u32 u);

    /// Constructor
    /// @param i the `i32` constant to store in the constant
    explicit Constant(i32 i);

    /// Destructor
    ~Constant() override;

    Constant(const Constant&) = delete;
    Constant(Constant&&) = delete;

    Constant& operator=(const Constant&) = delete;
    Constant& operator=(Constant&&) = delete;

    /// @returns true if this is a f32 constant
    bool IsF32() const { return kind_ == Kind::kF32; }
    /// @returns true if this is a f16 constant
    bool IsF16() const { return kind_ == Kind::kF16; }
    /// @returns true if this is an i32 constant
    bool IsI32() const { return kind_ == Kind::kI32; }
    /// @returns true if this is a u32 constant
    bool IsU32() const { return kind_ == Kind::kU32; }
    /// @returns true if this is a bool constant
    bool IsBool() const { return kind_ == Kind::kBool; }

    /// @returns the kind of constant
    Kind GetKind() const { return kind_; }

    /// @returns the constant data as a `f32`.
    /// @note, must only be called if `IsF32()` is true
    f32 AsF32() const { return std::get<f32>(data_); }
    /// @returns the constant data as a `f16`.
    /// @note, must only be called if `IsF16()` is true
    f16 AsF16() const { return std::get<f16>(data_); }
    /// @returns the constant data as an `i32`.
    /// @note, must only be called if `IsI32()` is true
    i32 AsI32() const { return std::get<i32>(data_); }
    /// @returns the constant data as a `u32`.
    /// @note, must only be called if `IsU32()` is true
    u32 AsU32() const { return std::get<u32>(data_); }
    /// @returns the constant data as a `bool`.
    /// @note, must only be called if `IsBool()` is true
    bool AsBool() const { return std::get<bool>(data_); }

    /// Write the constant to the given stream
    /// @param out the stream to write to
    /// @returns the stream
    std::ostream& ToString(std::ostream& out) const override;

  private:
    /// The type of data stored in this constant
    Kind kind_;
    /// The data stored in the constant
    std::variant<f32, f16, u32, i32, bool> data_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CONSTANT_H_
