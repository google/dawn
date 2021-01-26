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

#include "src/demangler.h"

namespace tint {
namespace {

constexpr char kSymbol[] = "tint_symbol_";
constexpr size_t kSymbolLen = sizeof(kSymbol) - 1;

}  // namespace

Demangler::Demangler() = default;

Demangler::~Demangler() = default;

std::string Demangler::Demangle(const Program& program,
                                const std::string& str) const {
  auto ret = str;

  size_t pos = 0;
  for (;;) {
    auto idx = ret.find(kSymbol, pos);
    if (idx == std::string::npos)
      break;

    auto start_idx = idx + kSymbolLen;
    auto end_idx = start_idx;
    while (ret[end_idx] >= '0' && ret[end_idx] <= '9') {
      end_idx++;
    }
    auto len = end_idx - start_idx;

    auto id = ret.substr(start_idx, len);

    Symbol sym(std::stoi(id));
    auto name = program.SymbolToName(sym);
    ret.replace(idx, end_idx - idx, name);

    pos = idx + name.length();
  }

  return ret;
}

}  // namespace tint
