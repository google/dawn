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

#include "src/tint/symbol_table.h"

#include "src/tint/debug.h"

namespace tint {

SymbolTable::SymbolTable(tint::ProgramID program_id) : program_id_(program_id) {}

SymbolTable::SymbolTable(const SymbolTable&) = default;

SymbolTable::SymbolTable(SymbolTable&&) = default;

SymbolTable::~SymbolTable() = default;

SymbolTable& SymbolTable::operator=(const SymbolTable& other) = default;

SymbolTable& SymbolTable::operator=(SymbolTable&&) = default;

Symbol SymbolTable::Register(const std::string& name) {
    TINT_ASSERT(Symbol, !name.empty());

    auto it = name_to_symbol_.find(name);
    if (it != name_to_symbol_.end()) {
        return it->second;
    }

#if TINT_SYMBOL_STORE_DEBUG_NAME
    Symbol sym(next_symbol_, program_id_, name);
#else
    Symbol sym(next_symbol_, program_id_);
#endif
    ++next_symbol_;

    name_to_symbol_[name] = sym;
    symbol_to_name_[sym] = name;

    return sym;
}

Symbol SymbolTable::Get(const std::string& name) const {
    auto it = name_to_symbol_.find(name);
    return it != name_to_symbol_.end() ? it->second : Symbol();
}

std::string SymbolTable::NameFor(const Symbol symbol) const {
    TINT_ASSERT_PROGRAM_IDS_EQUAL(Symbol, program_id_, symbol);
    auto it = symbol_to_name_.find(symbol);
    if (it == symbol_to_name_.end()) {
        return symbol.to_str();
    }

    return it->second;
}

Symbol SymbolTable::New(std::string prefix /* = "" */) {
    if (prefix.empty()) {
        prefix = "tint_symbol";
    }
    auto it = name_to_symbol_.find(prefix);
    if (it == name_to_symbol_.end()) {
        return Register(prefix);
    }
    std::string name;
    size_t i = 1;
    do {
        name = prefix + "_" + std::to_string(i++);
    } while (name_to_symbol_.count(name));
    return Register(name);
}

}  // namespace tint
