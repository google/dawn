// Copyright 2023 The Tint Authors.
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

#include "src/tint/ir/var.h"
#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Var);

namespace tint::ir {

Var::Var(uint32_t id,
         const type::Type* ty,
         builtin::AddressSpace address_space,
         builtin::Access access)
    : Base(id, ty), address_space_(address_space), access_(access) {}

Var::~Var() = default;

utils::StringStream& Var::ToInstruction(utils::StringStream& out) const {
    ToValue(out) << " = var " << address_space_ << " " << access_;
    return out;
}

}  // namespace tint::ir
