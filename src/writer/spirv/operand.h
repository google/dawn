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

#ifndef SRC_WRITER_SPIRV_OPERAND_H_
#define SRC_WRITER_SPIRV_OPERAND_H_

#include <string>
#include <vector>

namespace tint {
namespace writer {
namespace spirv {

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

  /// Sets the float value
  /// @param val the value to set
  void set_float(float val) { float_val_ = val; }
  /// Sets the int value
  /// @param val the value to set
  void set_int(uint32_t val) { int_val_ = val; }
  /// Sets the string value
  /// @param val the value to set
  void set_string(const std::string& val) { str_val_ = val; }

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

}  // namespace spirv
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_SPIRV_OPERAND_H_
