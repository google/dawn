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

#include "src/tint/lang/core/ir/module.h"

#include <limits>

#include "src/tint/utils/ice/ice.h"

namespace tint::ir {

Module::Module() = default;

Module::Module(Module&&) = default;

Module::~Module() = default;

Module& Module::operator=(Module&&) = default;

Symbol Module::NameOf(Instruction* inst) {
    TINT_ASSERT(inst->HasResults() && !inst->HasMultiResults());
    return NameOf(inst->Result());
}

Symbol Module::NameOf(Value* value) {
    return value_to_name_.Get(value).value_or(Symbol{});
}

void Module::SetName(Instruction* inst, std::string_view name) {
    TINT_ASSERT(inst->HasResults() && !inst->HasMultiResults());
    return SetName(inst->Result(), name);
}

void Module::SetName(Value* value, std::string_view name) {
    TINT_ASSERT(!name.empty());
    value_to_name_.Replace(value, symbols.Register(name));
}

void Module::SetName(Value* value, Symbol name) {
    TINT_ASSERT(name.IsValid());
    value_to_name_.Replace(value, name);
}

}  // namespace tint::ir
