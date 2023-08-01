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

#ifndef SRC_TINT_UTILS_SYMBOL_SYMBOL_H_
#define SRC_TINT_UTILS_SYMBOL_SYMBOL_H_

#include <string>

#include "src/tint/utils/id/generation_id.h"

namespace tint {

/// A symbol representing a string in the system
class Symbol {
  public:
    /// Constructor
    /// An invalid symbol
    Symbol();
    /// Constructor
    /// @param val the symbol value
    /// @param pid the identifier of the program that owns this Symbol
    /// @param name the name this symbol represents
    Symbol(uint32_t val, tint::GenerationID pid, std::string_view name);

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

    /// Equality operator
    /// @param o the other symbol
    /// @returns true if the symbols are the same
    bool operator==(const Symbol& o) const;

    /// Inequality operator
    /// @param o the other symbol
    /// @returns true if the symbols are the different
    bool operator!=(const Symbol& o) const;

    /// Less-than operator
    /// @param o the other symbol
    /// @returns true if this symbol is ordered before symbol `o`
    bool operator<(const Symbol& o) const;

    /// @returns true if the symbol is valid
    bool IsValid() const { return val_ != static_cast<uint32_t>(-1); }

    /// @returns true if the symbol is valid
    operator bool() const { return IsValid(); }

    /// @returns the value for the symbol
    uint32_t value() const { return val_; }

    /// Convert the symbol to a string
    /// @return the string representation of the symbol
    std::string to_str() const;

    /// Converts the symbol to the registered name
    /// @returns the string_view representing the name of the symbol
    std::string_view NameView() const;

    /// Converts the symbol to the registered name
    /// @returns the string representing the name of the symbol
    std::string Name() const;

    /// @returns the identifier of the Program that owns this symbol.
    tint::GenerationID GenerationID() const { return generation_id_; }

  private:
    uint32_t val_ = static_cast<uint32_t>(-1);
    tint::GenerationID generation_id_;
    std::string_view name_;
};

/// @param sym the Symbol
/// @returns the GenerationID that owns the given Symbol
inline GenerationID GenerationIDOf(Symbol sym) {
    return sym.IsValid() ? sym.GenerationID() : GenerationID();
}

}  // namespace tint

namespace std {

/// Custom std::hash specialization for tint::Symbol so symbols can be used as
/// keys for std::unordered_map and std::unordered_set.
template <>
class hash<tint::Symbol> {
  public:
    /// @param sym the symbol to return
    /// @return the Symbol internal value
    inline std::size_t operator()(const tint::Symbol& sym) const {
        return static_cast<std::size_t>(sym.value());
    }
};

}  // namespace std

#endif  // SRC_TINT_UTILS_SYMBOL_SYMBOL_H_
