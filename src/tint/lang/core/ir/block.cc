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

#include "src/tint/lang/core/ir/block.h"
#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::Block);

namespace tint::ir {

Block::Block() : Base() {}

Block::~Block() = default;

Instruction* Block::Prepend(Instruction* inst) {
    TINT_ASSERT_OR_RETURN_VALUE(inst, inst);
    TINT_ASSERT_OR_RETURN_VALUE(inst->Block() == nullptr, inst);

    inst->SetBlock(this);
    instructions_.count += 1;

    if (instructions_.first == nullptr) {
        instructions_.first = inst;
        instructions_.last = inst;
    } else {
        inst->next = instructions_.first;
        instructions_.first->prev = inst;
        instructions_.first = inst;
    }

    return inst;
}

Instruction* Block::Append(Instruction* inst) {
    TINT_ASSERT_OR_RETURN_VALUE(inst, inst);
    TINT_ASSERT_OR_RETURN_VALUE(inst->Block() == nullptr, inst);

    inst->SetBlock(this);
    instructions_.count += 1;

    if (instructions_.first == nullptr) {
        instructions_.first = inst;
        instructions_.last = inst;
    } else {
        inst->prev = instructions_.last;
        instructions_.last->next = inst;
        instructions_.last = inst;
    }

    return inst;
}

void Block::InsertBefore(Instruction* before, Instruction* inst) {
    TINT_ASSERT_OR_RETURN(before);
    TINT_ASSERT_OR_RETURN(inst);
    TINT_ASSERT_OR_RETURN(before->Block() == this);
    TINT_ASSERT_OR_RETURN(inst->Block() == nullptr);

    inst->SetBlock(this);
    instructions_.count += 1;

    inst->next = before;
    inst->prev = before->prev;
    before->prev = inst;

    if (inst->prev) {
        inst->prev->next = inst;
    }

    if (before == instructions_.first) {
        instructions_.first = inst;
    }
}

void Block::InsertAfter(Instruction* after, Instruction* inst) {
    TINT_ASSERT_OR_RETURN(after);
    TINT_ASSERT_OR_RETURN(inst);
    TINT_ASSERT_OR_RETURN(after->Block() == this);
    TINT_ASSERT_OR_RETURN(inst->Block() == nullptr);

    inst->SetBlock(this);
    instructions_.count += 1;

    inst->prev = after;
    inst->next = after->next;
    after->next = inst;

    if (inst->next) {
        inst->next->prev = inst;
    }
    if (after == instructions_.last) {
        instructions_.last = inst;
    }
}

void Block::Replace(Instruction* target, Instruction* inst) {
    TINT_ASSERT_OR_RETURN(target);
    TINT_ASSERT_OR_RETURN(inst);
    TINT_ASSERT_OR_RETURN(target->Block() == this);
    TINT_ASSERT_OR_RETURN(inst->Block() == nullptr);

    inst->SetBlock(this);
    target->SetBlock(nullptr);

    inst->next = target->next;
    inst->prev = target->prev;

    target->next = nullptr;
    target->prev = nullptr;

    if (inst->next) {
        inst->next->prev = inst;
    }
    if (inst->prev) {
        inst->prev->next = inst;
    }

    if (target == instructions_.first) {
        instructions_.first = inst;
    }
    if (target == instructions_.last) {
        instructions_.last = inst;
    }
}

void Block::Remove(Instruction* inst) {
    TINT_ASSERT_OR_RETURN(inst);
    TINT_ASSERT_OR_RETURN(inst->Block() == this);

    inst->SetBlock(nullptr);
    instructions_.count -= 1;

    if (inst->prev) {
        inst->prev->next = inst->next;
    }
    if (inst->next) {
        inst->next->prev = inst->prev;
    }
    if (inst == instructions_.first) {
        instructions_.first = inst->next;
    }
    if (inst == instructions_.last) {
        instructions_.last = inst->prev;
    }

    inst->prev = nullptr;
    inst->next = nullptr;
}

}  // namespace tint::ir
