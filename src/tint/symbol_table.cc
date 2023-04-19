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

SymbolTable::SymbolTable(SymbolTable&&) = default;

SymbolTable::~SymbolTable() = default;

SymbolTable& SymbolTable::operator=(SymbolTable&&) = default;

Symbol SymbolTable::Register(std::string_view name) {
    TINT_ASSERT(Symbol, !name.empty());

    auto it = name_to_symbol_.Find(name);
    if (it) {
        return *it;
    }
    return RegisterInternal(name);
}

Symbol SymbolTable::RegisterInternal(std::string_view name) {
    char* name_mem = name_allocator_.Allocate(name.length() + 1);
    if (name_mem == nullptr) {
        return Symbol();
    }

    memcpy(name_mem, name.data(), name.length() + 1);
    std::string_view nv(name_mem, name.length());

    Symbol sym(next_symbol_, program_id_, nv);
    ++next_symbol_;
    name_to_symbol_.Add(sym.NameView(), sym);

    return sym;
}

Symbol SymbolTable::Get(std::string_view name) const {
    auto it = name_to_symbol_.Find(name);
    return it ? *it : Symbol();
}

Symbol SymbolTable::New(std::string_view prefix_view /* = "" */) {
    std::string prefix;
    if (prefix_view.empty()) {
        prefix = "tint_symbol";
    } else {
        prefix = std::string(prefix_view);
    }

    auto it = name_to_symbol_.Find(prefix);
    if (!it) {
        return RegisterInternal(prefix);
    }

    size_t i = 0;
    auto last_prefix = last_prefix_to_index_.Find(prefix);
    if (last_prefix) {
        i = *last_prefix;
    }

    std::string name;
    do {
        ++i;
        name = prefix + "_" + std::to_string(i);
    } while (name_to_symbol_.Contains(name));

    auto sym = RegisterInternal(name);
    if (last_prefix) {
        *last_prefix = i;
    } else {
        last_prefix_to_index_.Add(prefix, i);
    }
    return sym;
}

}  // namespace tint
