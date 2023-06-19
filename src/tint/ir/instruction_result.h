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

#ifndef SRC_TINT_IR_INSTRUCTION_RESULT_H_
#define SRC_TINT_IR_INSTRUCTION_RESULT_H_

#include "src/tint/ir/value.h"
#include "src/tint/utils/string_stream.h"

namespace tint::ir {

/// An instruction result in the IR.
class InstructionResult : public utils::Castable<InstructionResult, Value> {
  public:
    /// Constructor
    /// @param type the type of the value
    explicit InstructionResult(const type::Type* type);

    /// Destructor
    ~InstructionResult() override;

    /// @copydoc Value::Destroy
    void Destroy() override;

    /// @returns the type of the value
    const type::Type* Type() override { return type_; }

    /// Sets the source instruction for this value
    /// @param inst the instruction to set
    void SetSource(Instruction* inst) { source_ = inst; }

    /// @returns the source instruction, if any
    Instruction* Source() { return source_; }

  private:
    Instruction* source_ = nullptr;
    const type::Type* type_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_INSTRUCTION_RESULT_H_
