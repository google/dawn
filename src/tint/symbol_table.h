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

#ifndef SRC_TINT_SYMBOL_TABLE_H_
#define SRC_TINT_SYMBOL_TABLE_H_

#include <string>

#include "src/tint/symbol.h"
#include "utils/bump_allocator.h"
#include "utils/hashmap.h"

namespace tint {

/// Holds mappings from symbols to their associated string names
class SymbolTable {
  public:
    /// Constructor
    /// @param program_id the identifier of the program that owns this symbol
    /// table
    explicit SymbolTable(tint::ProgramID program_id);
    /// Move Constructor
    SymbolTable(SymbolTable&&);
    /// Destructor
    ~SymbolTable();

    /// Move assignment
    /// @param other the symbol table to move
    /// @returns the symbol table
    SymbolTable& operator=(SymbolTable&& other);

    /// Wrap sets this symbol table to hold symbols which point to the allocated names in @p o.
    /// The symbol table after Wrap is intended to temporarily extend the objects
    /// of an existing immutable SymbolTable
    /// As the copied objects are owned by @p o, @p o must not be destructed
    /// or assigned while using this symbol table.
    /// @param o the immutable SymbolTable to extend
    void Wrap(const SymbolTable& o) {
        next_symbol_ = o.next_symbol_;
        name_to_symbol_ = o.name_to_symbol_;
        last_prefix_to_index_ = o.last_prefix_to_index_;
        program_id_ = o.program_id_;
    }

    /// Registers a name into the symbol table, returning the Symbol.
    /// @param name the name to register
    /// @returns the symbol representing the given name
    Symbol Register(std::string_view name);

    /// Returns the symbol for the given `name`
    /// @param name the name to lookup
    /// @returns the symbol for the name or Symbol() if not found.
    Symbol Get(std::string_view name) const;

    /// Returns a new unique symbol with the given name, possibly suffixed with a
    /// unique number.
    /// @param name the symbol name
    /// @returns a new, unnamed symbol with the given name. If the name is already
    /// taken then this will be suffixed with an underscore and a unique numerical
    /// value
    Symbol New(std::string_view name = "");

    /// Foreach calls the callback function `F` for each symbol in the table.
    /// @param callback must be a function or function-like object with the
    /// signature: `void(Symbol)`
    template <typename F>
    void Foreach(F&& callback) const {
        for (auto it : name_to_symbol_) {
            callback(it.value);
        }
    }

    /// @returns the identifier of the Program that owns this symbol table.
    tint::ProgramID ProgramID() const { return program_id_; }

  private:
    SymbolTable(const SymbolTable&) = delete;
    SymbolTable& operator=(const SymbolTable& other) = delete;

    Symbol RegisterInternal(std::string_view name);

    // The value to be associated to the next registered symbol table entry.
    uint32_t next_symbol_ = 1;

    utils::Hashmap<std::string_view, Symbol, 0> name_to_symbol_;
    utils::Hashmap<std::string, size_t, 0> last_prefix_to_index_;
    tint::ProgramID program_id_;

    utils::BumpAllocator name_allocator_;
};

/// @param symbol_table the SymbolTable
/// @returns the ProgramID that owns the given SymbolTable
inline ProgramID ProgramIDOf(const SymbolTable& symbol_table) {
    return symbol_table.ProgramID();
}

}  // namespace tint

#endif  // SRC_TINT_SYMBOL_TABLE_H_
