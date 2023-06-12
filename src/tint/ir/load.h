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

#ifndef SRC_TINT_IR_LOAD_H_
#define SRC_TINT_IR_LOAD_H_

#include "src/tint/ir/operand_instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A load instruction in the IR.
class Load : public utils::Castable<Load, OperandInstruction<1>> {
  public:
    /// Constructor (infers type)
    /// @param from the value being loaded from
    explicit Load(Value* from);

    ~Load() override;

    /// @returns the type of the value
    const type::Type* Type() override { return result_type_; }

    /// @returns the value being loaded from
    Value* From() { return operands_[0]; }

  private:
    const type::Type* result_type_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_LOAD_H_
