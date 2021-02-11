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

#include "src/symbol_table.h"

namespace tint {

SymbolTable::SymbolTable() = default;

SymbolTable::SymbolTable(const SymbolTable&) = default;

SymbolTable::SymbolTable(SymbolTable&&) = default;

SymbolTable::~SymbolTable() = default;

SymbolTable& SymbolTable::operator=(const SymbolTable& other) = default;

SymbolTable& SymbolTable::operator=(SymbolTable&&) = default;

Symbol SymbolTable::Register(const std::string& name) {
  if (name == "")
    return Symbol();

  auto it = name_to_symbol_.find(name);
  if (it != name_to_symbol_.end())
    return it->second;

  Symbol sym(next_symbol_);
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
  auto it = symbol_to_name_.find(symbol);
  if (it == symbol_to_name_.end()) {
    return symbol.to_str();
  }

  return it->second;
}

Symbol SymbolTable::New() {
  return Symbol(next_symbol_++);
}

}  // namespace tint
