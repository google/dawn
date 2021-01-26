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

#include "src/namer.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

#include "src/symbol.h"

namespace tint {

Namer::Namer(SymbolTable* symbols) : symbols_(symbols) {}

Namer::~Namer() = default;

bool Namer::IsUsed(const std::string& name) {
  auto it = used_.find(name);
  return it != used_.end();
}

std::string Namer::GenerateName(const std::string& prefix) {
  std::string name = prefix;
  uint32_t i = 0;
  while (IsUsed(name)) {
    name = prefix + "_" + std::to_string(i);
    ++i;
  }
  used_.insert(name);
  return name;
}

MangleNamer::MangleNamer(SymbolTable* symbols) : Namer(symbols) {}

MangleNamer::~MangleNamer() = default;

std::string MangleNamer::NameFor(const Symbol& sym) {
  return sym.to_str();
}

UnsafeNamer::UnsafeNamer(SymbolTable* symbols) : Namer(symbols) {}

UnsafeNamer::~UnsafeNamer() = default;

std::string UnsafeNamer::NameFor(const Symbol& sym) {
  return symbols_->NameFor(sym);
}

}  // namespace tint
