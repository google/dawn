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

#include "src/tint/lang/core/ir/instruction_result.h"

#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/instruction.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::InstructionResult);

namespace tint::ir {

InstructionResult::InstructionResult(const type::Type* type) : type_(type) {
    TINT_ASSERT(type_ != nullptr);
}

InstructionResult::~InstructionResult() = default;

void InstructionResult::Destroy() {
    TINT_ASSERT(source_ == nullptr);
    Base::Destroy();
}

}  // namespace tint::ir
