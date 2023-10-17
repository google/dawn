// Copyright 2022 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "src/tint/lang/core/ir/module.h"

#include <limits>

#include "src/tint/utils/ice/ice.h"

namespace tint::core::ir {

Module::Module() : root_block(blocks.Create<ir::Block>()) {}

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

}  // namespace tint::core::ir
