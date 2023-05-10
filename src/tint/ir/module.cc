// Copyright 2022 The Tint Authors.
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

#include "src/tint/ir/module.h"

#include <limits>

namespace tint::ir {

Module::Module() = default;

Module::Module(Module&&) = default;

Module::~Module() = default;

Module& Module::operator=(Module&&) = default;

Symbol Module::NameOf(const Value* value) const {
    return value_to_id_.Get(value).value_or(Symbol{});
}

Symbol Module::SetName(const Value* value, std::string_view name) {
    TINT_ASSERT(IR, !name.empty());

    if (auto old = value_to_id_.Get(value)) {
        value_to_id_.Remove(value);
        id_to_value_.Remove(old.value());
    }

    auto sym = symbols.Register(name);
    if (id_to_value_.Add(sym, value)) {
        value_to_id_.Add(value, sym);
        return sym;
    }
    auto prefix = std::string(name) + "_";
    for (uint64_t suffix = 1; suffix != std::numeric_limits<uint64_t>::max(); suffix++) {
        sym = symbols.Register(prefix + std::to_string(suffix));
        if (id_to_value_.Add(sym, value)) {
            value_to_id_.Add(value, sym);
            return sym;
        }
    }
    TINT_ASSERT(IR, false);  // !
    return Symbol{};
}

}  // namespace tint::ir
