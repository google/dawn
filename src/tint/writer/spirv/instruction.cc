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

#include "src/tint/writer/spirv/instruction.h"

#include <utility>

namespace tint::writer::spirv {

Instruction::Instruction(spv::Op op, OperandList operands)
    : op_(op), operands_(std::move(operands)) {}

Instruction::Instruction(const Instruction&) = default;

Instruction& Instruction::operator=(const Instruction&) = default;

Instruction::~Instruction() = default;

uint32_t Instruction::word_length() const {
    uint32_t size = 1;  // Initial 1 for the op and size
    for (const auto& op : operands_) {
        size += OperandLength(op);
    }
    return size;
}

}  // namespace tint::writer::spirv
