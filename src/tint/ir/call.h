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

#include "src/tint/castable.h"
#include "src/tint/ir/instruction.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

/// A Call instruction in the IR.
class Call : public Castable<Call, Instruction> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param args the constructor arguments
    Call(Value* result, utils::VectorRef<Value*> args);
    Call(const Call& instr) = delete;
    Call(Call&& instr) = delete;
    ~Call() override;

    Call& operator=(const Call& instr) = delete;
    Call& operator=(Call&& instr) = delete;

    /// @returns the constructor arguments
    utils::VectorRef<Value*> Args() const { return args_; }

    /// Writes the call arguments to the given stream.
    /// @param out the output stream
    void EmitArgs(utils::StringStream& out) const;

  private:
    utils::Vector<Value*, 1> args_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CALL_H_
