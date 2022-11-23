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

#ifndef SRC_TINT_IR_REGISTER_H_
#define SRC_TINT_IR_REGISTER_H_

#include <string>
#include <variant>

#include "src/tint/number.h"
#include "src/tint/symbol.h"

namespace tint::ir {

/// Register in the IR. The register can be one of several types these include, but aren't limited
/// to, `f32`, `u32`, `temp`, `var`. The type of the register determines the type of data stored
/// in the register.
class Register {
  public:
    /// A register id.
    using Id = uint32_t;

    /// Stores data for a given variable. There will be multiple `VarData` entries for a given `id`.
    /// The `id` acts like a generation number (although they aren't sequential, they are
    /// increasing). As the variable is stored too a new register will be created and the the `id`
    /// will be incremented.
    struct VarData {
        /// The symbol for the variable
        Symbol sym;
        /// The id for the variable.
        Id id;
        // TODO(dsinclair): Should var type data be stored here along side the variable info?
    };

    /// Constructor
    /// Creates a uninitialized register
    Register();

    /// Constructor
    /// @param id the id for the register
    explicit Register(Id id);

    /// Constructor
    /// @param s the symbol for the register
    /// @param id the id for the register
    Register(Symbol s, Id id);

    /// Constructor
    /// @param b the `bool` value to store in the register
    explicit Register(bool b);

    /// Constructor
    /// @param f the `f32` value to store in the register
    explicit Register(f32 f);

    /// Constructor
    /// @param f the `f16` value to store in the register
    explicit Register(f16 f);

    /// Constructor
    /// @param u the `u32` value to store in the register
    explicit Register(u32 u);

    /// Constructor
    /// @param i the `i32` value to store in the register
    explicit Register(i32 i);

    /// Destructor
    ~Register();

    /// Copy constructor
    /// @param o the register to copy from
    Register(const Register& o);
    /// Move constructor
    /// @param o the register to move from
    Register(Register&& o);

    /// Copy assign
    /// @param o the register to copy from
    /// @returns this
    Register& operator=(const Register& o);
    /// Move assign
    /// @param o the register to move from
    /// @returns this
    Register& operator=(Register&& o);

    /// @returns true if this is a temporary register
    bool IsTemp() const { return kind_ == Kind::kTemp; }
    /// @returns true if this is a f32 register
    bool IsF32() const { return kind_ == Kind::kF32; }
    /// @returns true if this is a f16 register
    bool IsF16() const { return kind_ == Kind::kF16; }
    /// @returns true if this is an i32 register
    bool IsI32() const { return kind_ == Kind::kI32; }
    /// @returns true if this is a u32 register
    bool IsU32() const { return kind_ == Kind::kU32; }
    /// @returns true if this is a var register
    bool IsVar() const { return kind_ == Kind::kVar; }
    /// @returns true if this is a bool register
    bool IsBool() const { return kind_ == Kind::kBool; }

    /// @returns the register data as a `f32`.
    /// @note, must only be called if `IsF32()` is true
    f32 AsF32() const { return std::get<f32>(data_); }
    /// @returns the register data as a `f16`.
    /// @note, must only be called if `IsF16()` is true
    f16 AsF16() const { return std::get<f16>(data_); }
    /// @returns the register data as an `i32`.
    /// @note, must only be called if `IsI32()` is true
    i32 AsI32() const { return std::get<i32>(data_); }
    /// @returns the register data as a `u32`.
    /// @note, must only be called if `IsU32()` is true
    u32 AsU32() const { return std::get<u32>(data_); }
    /// @returns the register data as an `Id`.
    /// @note, must only be called if `IsTemp()` is true
    Id AsId() const { return std::get<Id>(data_); }
    /// @returns the register data as a `VarData` structure.
    /// @note, must only be called if `IsVar()` is true
    VarData AsVarData() const { return std::get<VarData>(data_); }
    /// @returns the register data as a `bool`.
    /// @note, must only be called if `IsBool()` is true
    bool AsBool() const { return std::get<bool>(data_); }

    /// @returns the string representation of the register
    std::string AsString() const;

  private:
    /// The type of the register
    enum class Kind {
        /// A uninitialized register
        kUninitialized,
        /// A temporary allocated register
        kTemp,
        /// A f32 register
        kF32,
        /// A f16 register
        kF16,
        /// An i32 register
        kI32,
        /// A u32 register
        kU32,
        /// A variable register
        kVar,
        /// A boolean register
        kBool,
    };

    /// The type of data stored in this register
    Kind kind_;
    /// The data stored in the register
    std::variant<Id, f32, f16, u32, i32, VarData, bool> data_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_REGISTER_H_
