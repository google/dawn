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

#include "src/tint/demangler.h"

#include "src/tint/program.h"

namespace tint {
namespace {

constexpr char kSymbol[] = "$";
constexpr size_t kSymbolLen = sizeof(kSymbol) - 1;

}  // namespace

Demangler::Demangler() = default;

Demangler::~Demangler() = default;

std::string Demangler::Demangle(const SymbolTable& symbols, const std::string& str) const {
    std::stringstream out;

    size_t pos = 0;
    for (;;) {
        auto idx = str.find(kSymbol, pos);
        if (idx == std::string::npos) {
            out << str.substr(pos);
            break;
        }

        out << str.substr(pos, idx - pos);

        auto start_idx = idx + kSymbolLen;
        auto end_idx = start_idx;
        while (str[end_idx] >= '0' && str[end_idx] <= '9') {
            end_idx++;
        }
        auto len = end_idx - start_idx;

        auto id = str.substr(start_idx, len);
        Symbol sym(static_cast<uint32_t>(std::stoi(id)), symbols.ProgramID());
        out << symbols.NameFor(sym);

        pos = end_idx;
    }

    return out.str();
}

}  // namespace tint
