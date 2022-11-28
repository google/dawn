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

#ifndef SRC_TINT_IR_VALUE_H_
#define SRC_TINT_IR_VALUE_H_

#include <ostream>
#include <variant>

#include "src/tint/number.h"
#include "src/tint/symbol.h"

namespace tint::ir {

/// Value in the IR. The value can be one of several types these include, but aren't limited
/// to, `f32`, `u32`, `temp`, `var`. The type of the value determines the type of data stored
/// in the value.
class Value {
  public:
    /// A value id.
    using Id = uint32_t;

    /// The type of the value
    enum class Kind {
        /// A uninitialized value
        kUninitialized,
        /// A temporary allocated value
        kTemp,
        /// A f32 value
        kF32,
        /// A f16 value
        kF16,
        /// An i32 value
        kI32,
        /// A u32 value
        kU32,
        /// A variable value
        kVar,
        /// A boolean value
        kBool,
    };

    /// Stores data for a given variable. There will be multiple `VarData` entries for a given `id`.
    /// The `id` acts like a generation number (although they aren't sequential, they are
    /// increasing). As the variable is stored too a new value will be created and the the `id`
    /// will be incremented.
    struct VarData {
        /// The symbol for the variable
        Symbol sym;
        /// The id for the variable.
        Id id;
        // TODO(dsinclair): Should var type data be stored here along side the variable info?
    };

    /// Constructor
    /// Creates a uninitialized value
    Value();

    /// Constructor
    /// @param id the id for the value
    explicit Value(Id id);

    /// Constructor
    /// @param s the symbol for the value
    /// @param id the id for the value
    Value(Symbol s, Id id);

    /// Constructor
    /// @param b the `bool` value to store in the value
    explicit Value(bool b);

    /// Constructor
    /// @param f the `f32` value to store in the value
    explicit Value(f32 f);

    /// Constructor
    /// @param f the `f16` value to store in the value
    explicit Value(f16 f);

    /// Constructor
    /// @param u the `u32` value to store in the value
    explicit Value(u32 u);

    /// Constructor
    /// @param i the `i32` value to store in the value
    explicit Value(i32 i);

    /// Destructor
    ~Value();

    /// Copy constructor
    /// @param o the value to copy from
    Value(const Value& o);
    /// Move constructor
    /// @param o the value to move from
    Value(Value&& o);

    /// Copy assign
    /// @param o the value to copy from
    /// @returns this
    Value& operator=(const Value& o);
    /// Move assign
    /// @param o the value to move from
    /// @returns this
    Value& operator=(Value&& o);

    /// @returns true if this is a temporary value
    bool IsTemp() const { return kind_ == Kind::kTemp; }
    /// @returns true if this is a f32 value
    bool IsF32() const { return kind_ == Kind::kF32; }
    /// @returns true if this is a f16 value
    bool IsF16() const { return kind_ == Kind::kF16; }
    /// @returns true if this is an i32 value
    bool IsI32() const { return kind_ == Kind::kI32; }
    /// @returns true if this is a u32 value
    bool IsU32() const { return kind_ == Kind::kU32; }
    /// @returns true if this is a var value
    bool IsVar() const { return kind_ == Kind::kVar; }
    /// @returns true if this is a bool value
    bool IsBool() const { return kind_ == Kind::kBool; }

    /// @returns the kind of value
    Kind GetKind() const { return kind_; }

    /// @returns the value data as a `f32`.
    /// @note, must only be called if `IsF32()` is true
    f32 AsF32() const { return std::get<f32>(data_); }
    /// @returns the value data as a `f16`.
    /// @note, must only be called if `IsF16()` is true
    f16 AsF16() const { return std::get<f16>(data_); }
    /// @returns the value data as an `i32`.
    /// @note, must only be called if `IsI32()` is true
    i32 AsI32() const { return std::get<i32>(data_); }
    /// @returns the value data as a `u32`.
    /// @note, must only be called if `IsU32()` is true
    u32 AsU32() const { return std::get<u32>(data_); }
    /// @returns the value data as an `Id`.
    /// @note, must only be called if `IsTemp()` is true
    Id AsId() const { return std::get<Id>(data_); }
    /// @returns the value data as a `VarData` structure.
    /// @note, must only be called if `IsVar()` is true
    VarData AsVarData() const { return std::get<VarData>(data_); }
    /// @returns the value data as a `bool`.
    /// @note, must only be called if `IsBool()` is true
    bool AsBool() const { return std::get<bool>(data_); }

  private:
    /// The type of data stored in this value
    Kind kind_;
    /// The data stored in the value
    std::variant<Id, f32, f16, u32, i32, VarData, bool> data_;
};

std::ostream& operator<<(std::ostream& out, const Value& r);

}  // namespace tint::ir

#endif  // SRC_TINT_IR_VALUE_H_
