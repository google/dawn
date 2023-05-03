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

#include "src/tint/writer/spirv/function.h"

namespace tint::writer::spirv {

Function::Function() : declaration_(Instruction{spv::Op::OpNop, {}}), label_op_(Operand(0u)) {}

Function::Function(const Instruction& declaration,
                   const Operand& label_op,
                   const InstructionList& params)
    : declaration_(declaration), label_op_(label_op), params_(params) {}

Function::Function(const Function& other) = default;

Function& Function::operator=(const Function& other) = default;

Function::~Function() = default;

void Function::iterate(std::function<void(const Instruction&)> cb) const {
    cb(declaration_);

    for (const auto& param : params_) {
        cb(param);
    }

    cb(Instruction{spv::Op::OpLabel, {label_op_}});

    for (const auto& var : vars_) {
        cb(var);
    }
    for (const auto& inst : instructions_) {
        cb(inst);
    }

    cb(Instruction{spv::Op::OpFunctionEnd, {}});
}

}  // namespace tint::writer::spirv
