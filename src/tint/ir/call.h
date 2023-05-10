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

#include "src/tint/ir/instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// A Call instruction in the IR.
class Call : public utils::Castable<Call, Instruction> {
  public:
    Call(const Call& inst) = delete;
    Call(Call&& inst) = delete;
    ~Call() override;

    Call& operator=(const Call& inst) = delete;
    Call& operator=(Call&& inst) = delete;

    /// @returns the type of the value
    const type::Type* Type() const override { return result_type; }

    /// The instruction type
    const type::Type* result_type = nullptr;

    /// The constructor arguments
    utils::Vector<Value*, 1> args;

  protected:
    /// Constructor
    Call() = delete;
    /// Constructor
    /// @param result_type the result type
    /// @param args the constructor arguments
    Call(const type::Type* result_type, utils::VectorRef<Value*> args);
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CALL_H_
