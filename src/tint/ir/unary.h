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

#ifndef SRC_TINT_IR_UNARY_H_
#define SRC_TINT_IR_UNARY_H_

#include "src/tint/ir/operand_instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A unary instruction in the IR.
class Unary : public utils::Castable<Unary, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the value
    static constexpr size_t kValueOperandOffset = 0;

    /// The kind of instruction.
    enum class Kind {
        kComplement,
        kNegation,
    };

    /// Constructor
    /// @param result the result value
    /// @param kind the kind of unary instruction
    /// @param val the input value for the instruction
    Unary(InstructionResult* result, enum Kind kind, Value* val);
    ~Unary() override;

    /// @returns the value for the instruction
    Value* Val() { return operands_[kValueOperandOffset]; }

    /// @returns the kind of unary instruction
    enum Kind Kind() { return kind_; }

  private:
    enum Kind kind_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_UNARY_H_
