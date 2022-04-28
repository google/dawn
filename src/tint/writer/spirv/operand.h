// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_WRITER_SPIRV_OPERAND_H_
#define SRC_TINT_WRITER_SPIRV_OPERAND_H_

#include <cstring>
#include <string>
#include <vector>

#include "src/tint/utils/hash.h"

namespace tint::writer::spirv {

/// A single SPIR-V instruction operand
class Operand {
 public:
  /// The kind of the operand
  // Note, the `kInt` will cover most cases as things like IDs in SPIR-V are
  // just ints for the purpose of converting to binary.
  enum class Kind { kInt = 0, kFloat, kString };

  /// Creates a float operand
  /// @param val the float value
  /// @returns the operand
  static Operand Float(float val);
  /// Creates an int operand
  /// @param val the int value
  /// @returns the operand
  static Operand Int(uint32_t val);
  /// Creates a string operand
  /// @param val the string value
  /// @returns the operand
  static Operand String(const std::string& val);

  /// Constructor
  /// @param kind the type of operand
  explicit Operand(Kind kind);
  /// Copy Constructor
  Operand(const Operand&) = default;
  ~Operand();

  /// Copy assignment
  /// @param b the operand to copy
  /// @returns a copy of this operand
  Operand& operator=(const Operand& b) = default;

  /// Equality operator
  /// @param other the RHS of the operator
  /// @returns true if this operand is equal to other
  bool operator==(const Operand& other) const {
    if (kind_ == other.kind_) {
      switch (kind_) {
        case tint::writer::spirv::Operand::Kind::kFloat:
          // Use memcmp to work around:
          // error: comparing floating point with == or != is unsafe
          // [-Werror,-Wfloat-equal]
          return memcmp(&float_val_, &other.float_val_, sizeof(float_val_)) ==
                 0;
        case tint::writer::spirv::Operand::Kind::kInt:
          return int_val_ == other.int_val_;
        case tint::writer::spirv::Operand::Kind::kString:
          return str_val_ == other.str_val_;
      }
    }
    return false;
  }

  /// @returns the kind of the operand
  Kind GetKind() const { return kind_; }
  /// @returns true if this is a float operand
  bool IsFloat() const { return kind_ == Kind::kFloat; }
  /// @returns true if this is an integer operand
  bool IsInt() const { return kind_ == Kind::kInt; }
  /// @returns true if this is a string operand
  bool IsString() const { return kind_ == Kind::kString; }

  /// @returns the number of uint32_t's needed for this operand
  uint32_t length() const;

  /// @returns the float value
  float to_f() const { return float_val_; }
  /// @returns the int value
  uint32_t to_i() const { return int_val_; }
  /// @returns the string value
  const std::string& to_s() const { return str_val_; }

 private:
  Kind kind_ = Kind::kInt;
  float float_val_ = 0.0;
  uint32_t int_val_ = 0;
  std::string str_val_;
};

/// A list of operands
using OperandList = std::vector<Operand>;

using OperandListKey = utils::UnorderedKeyWrapper<OperandList>;

}  // namespace tint::writer::spirv

namespace std {

/// Custom std::hash specialization for tint::writer::spirv::Operand
template <>
class hash<tint::writer::spirv::Operand> {
 public:
  /// @param o the Operand
  /// @return the hash value
  inline std::size_t operator()(const tint::writer::spirv::Operand& o) const {
    switch (o.GetKind()) {
      case tint::writer::spirv::Operand::Kind::kFloat:
        return tint::utils::Hash(o.to_f());
      case tint::writer::spirv::Operand::Kind::kInt:
        return tint::utils::Hash(o.to_i());
      case tint::writer::spirv::Operand::Kind::kString:
        return tint::utils::Hash(o.to_s());
    }
    return 0;
  }
};

}  // namespace std
#endif  // SRC_TINT_WRITER_SPIRV_OPERAND_H_
