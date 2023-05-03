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

#ifndef SRC_TINT_WRITER_SPIRV_INSTRUCTION_H_
#define SRC_TINT_WRITER_SPIRV_INSTRUCTION_H_

#include <vector>

#include "spirv/unified1/spirv.hpp11"
#include "src/tint/writer/spirv/operand.h"

namespace tint::writer::spirv {

/// A single SPIR-V instruction
class Instruction {
  public:
    /// Constructor
    /// @param op the op to generate
    /// @param operands the operand values for the instruction
    Instruction(spv::Op op, OperandList operands);
    /// Copy Constructor
    Instruction(const Instruction&);
    /// Copy assignment operator
    /// @param other the instruction to copy
    /// @returns the new Instruction
    Instruction& operator=(const Instruction& other);
    /// Destructor
    ~Instruction();

    /// @returns the instructions op
    spv::Op opcode() const { return op_; }

    /// @returns the instructions operands
    const OperandList& operands() const { return operands_; }

    /// @returns the number of uint32_t's needed to hold the instruction
    uint32_t word_length() const;

  private:
    spv::Op op_ = spv::Op::OpNop;
    OperandList operands_;
};

/// A list of instructions
using InstructionList = std::vector<Instruction>;

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_INSTRUCTION_H_
