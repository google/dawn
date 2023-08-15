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

#ifndef SRC_TINT_LANG_CORE_IR_INSTRUCTION_RESULT_H_
#define SRC_TINT_LANG_CORE_IR_INSTRUCTION_RESULT_H_

#include "src/tint/lang/core/ir/value.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::core::ir {

/// An instruction result in the IR.
class InstructionResult : public Castable<InstructionResult, Value> {
  public:
    /// Constructor
    /// @param type the type of the value
    explicit InstructionResult(const core::type::Type* type);

    /// Destructor
    ~InstructionResult() override;

    /// @copydoc Value::Destroy
    void Destroy() override;

    /// @returns the type of the value
    const core::type::Type* Type() override { return type_; }

    /// Sets the type of the value to @p type
    /// @param type the new type of the value
    void SetType(const core::type::Type* type) { type_ = type; }

    /// Sets the source instruction for this value
    /// @param inst the instruction to set
    void SetSource(Instruction* inst) { source_ = inst; }

    /// @returns the source instruction, if any
    Instruction* Source() { return source_; }

  private:
    Instruction* source_ = nullptr;
    const core::type::Type* type_ = nullptr;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_INSTRUCTION_RESULT_H_
