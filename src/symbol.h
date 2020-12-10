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

#ifndef SRC_SYMBOL_H_
#define SRC_SYMBOL_H_

#include <string>

namespace tint {

/// A symbol representing a string in the system
class Symbol {
 public:
  /// Constructor
  /// An invalid symbol
  Symbol();
  /// Constructor
  /// @param val the symbol value
  explicit Symbol(uint32_t val);
  /// Copy constructor
  /// @param o the symbol to copy
  Symbol(const Symbol& o);
  /// Move constructor
  /// @param o the symbol to move
  Symbol(Symbol&& o);
  /// Destructor
  ~Symbol();

  /// Copy assignment
  /// @param o the other symbol
  /// @returns the symbol after doing the copy
  Symbol& operator=(const Symbol& o);
  /// Move assignment
  /// @param o the other symbol
  /// @returns teh symbol after doing the move
  Symbol& operator=(Symbol&& o);

  /// Comparison operator
  /// @param o the other symbol
  /// @returns true if the symbols are the same
  bool operator==(const Symbol& o) const;

  /// @returns true if the symbol is valid
  bool IsValid() const { return val_ != static_cast<uint32_t>(-1); }

  /// @returns the value for the symbol
  uint32_t value() const { return val_; }

  /// Convert the symbol to a string
  /// @return the string representation of the symbol
  std::string to_str() const;

 private:
  uint32_t val_ = static_cast<uint32_t>(-1);
};

}  // namespace tint

#endif  // SRC_SYMBOL_H_
