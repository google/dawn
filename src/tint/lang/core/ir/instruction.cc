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

#include "src/tint/lang/core/ir/instruction.h"

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Instruction);

namespace tint::ir {

Instruction::Instruction() = default;

Instruction::~Instruction() = default;

void Instruction::Destroy() {
    TINT_ASSERT(Alive());
    if (Block()) {
        Remove();
    }
    for (auto* result : Results()) {
        result->SetSource(nullptr);
        result->Destroy();
    }
    flags_.Add(Flag::kDead);
}

void Instruction::InsertBefore(Instruction* before) {
    TINT_ASSERT_OR_RETURN(before);
    TINT_ASSERT_OR_RETURN(before->Block() != nullptr);
    before->Block()->InsertBefore(before, this);
}

void Instruction::InsertAfter(Instruction* after) {
    TINT_ASSERT_OR_RETURN(after);
    TINT_ASSERT_OR_RETURN(after->Block() != nullptr);
    after->Block()->InsertAfter(after, this);
}

void Instruction::ReplaceWith(Instruction* replacement) {
    TINT_ASSERT_OR_RETURN(replacement);
    TINT_ASSERT_OR_RETURN(Block() != nullptr);
    Block()->Replace(this, replacement);
}

void Instruction::Remove() {
    TINT_ASSERT_OR_RETURN(Block() != nullptr);
    Block()->Remove(this);
}

}  // namespace tint::ir
