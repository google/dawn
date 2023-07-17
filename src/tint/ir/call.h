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

#ifndef SRC_TINT_IR_CALL_H_
#define SRC_TINT_IR_CALL_H_

#include "src/tint/ir/operand_instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A Call instruction in the IR.
class Call : public utils::Castable<Call, OperandInstruction<4, 1>> {
  public:
    ~Call() override;

    /// @returns the call arguments
    virtual utils::Slice<Value*> Args() { return operands_.Slice(); }

    /// Append a new argument to the argument list for this call instruction.
    /// @param arg the argument value to append
    void AppendArg(ir::Value* arg) { AddOperand(operands_.Length(), arg); }

  protected:
    /// Constructor
    Call();
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CALL_H_
