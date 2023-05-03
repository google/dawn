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

#ifndef SRC_TINT_WRITER_SPIRV_FUNCTION_H_
#define SRC_TINT_WRITER_SPIRV_FUNCTION_H_

#include <functional>

#include "src/tint/writer/spirv/instruction.h"

namespace tint::writer::spirv {

/// A SPIR-V function
class Function {
  public:
    /// Constructor for testing purposes
    /// This creates a bad declaration, so won't generate correct SPIR-V
    Function();

    /// Constructor
    /// @param declaration the function declaration
    /// @param label_op the operand for function's entry block label
    /// @param params the function parameters
    Function(const Instruction& declaration,
             const Operand& label_op,
             const InstructionList& params);
    /// Copy constructor
    /// @param other the function to copy
    Function(const Function& other);
    /// Copy assignment operator
    /// @param other the function to copy
    /// @returns the new Function
    Function& operator=(const Function& other);
    /// Destructor
    ~Function();

    /// Iterates over the function call the cb on each instruction
    /// @param cb the callback to call
    void iterate(std::function<void(const Instruction&)> cb) const;

    /// @returns the declaration
    const Instruction& declaration() const { return declaration_; }

    /// @returns the label ID for the function entry block
    uint32_t label_id() const { return std::get<uint32_t>(label_op_); }

    /// Adds an instruction to the instruction list
    /// @param op the op to set
    /// @param operands the operands for the instruction
    void push_inst(spv::Op op, const OperandList& operands) {
        instructions_.push_back(Instruction{op, operands});
    }
    /// @returns the instruction list
    const InstructionList& instructions() const { return instructions_; }

    /// Adds a variable to the variable list
    /// @param operands the operands for the variable
    void push_var(const OperandList& operands) {
        vars_.push_back(Instruction{spv::Op::OpVariable, operands});
    }
    /// @returns the variable list
    const InstructionList& variables() const { return vars_; }

    /// @returns the word length of the function
    uint32_t word_length() const {
        // 1 for the Label and 1 for the FunctionEnd
        uint32_t size = 2 + declaration_.word_length();

        for (const auto& param : params_) {
            size += param.word_length();
        }
        for (const auto& var : vars_) {
            size += var.word_length();
        }
        for (const auto& inst : instructions_) {
            size += inst.word_length();
        }
        return size;
    }

    /// @returns true if the function has a valid declaration
    explicit operator bool() const { return declaration_.opcode() == spv::Op::OpFunction; }

  private:
    Instruction declaration_;
    Operand label_op_;
    InstructionList params_;
    InstructionList vars_;
    InstructionList instructions_;
};

}  // namespace tint::writer::spirv

#endif  // SRC_TINT_WRITER_SPIRV_FUNCTION_H_
