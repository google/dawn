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

#ifndef SRC_TINT_LANG_CORE_IR_VAR_H_
#define SRC_TINT_LANG_CORE_IR_VAR_H_

#include "src/tint/lang/core/access.h"
#include "src/tint/lang/core/address_space.h"
#include "src/tint/lang/core/ir/binding_point.h"
#include "src/tint/lang/core/ir/operand_instruction.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/utils/containers/vector.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::ir {

/// A var instruction in the IR.
class Var : public Castable<Var, OperandInstruction<1, 1>> {
  public:
    /// The offset in Operands() for the initializer
    static constexpr size_t kInitializerOperandOffset = 0;

    /// Constructor
    /// @param result the result value
    explicit Var(InstructionResult* result);
    ~Var() override;

    /// Sets the var initializer
    /// @param initializer the initializer
    void SetInitializer(Value* initializer);
    /// @returns the initializer
    Value* Initializer() { return operands_[kInitializerOperandOffset]; }

    /// Sets the binding point
    /// @param group the group
    /// @param binding the binding
    void SetBindingPoint(uint32_t group, uint32_t binding) { binding_point_ = {group, binding}; }
    /// @returns the binding points if `Attributes` contains `kBindingPoint`
    std::optional<struct BindingPoint> BindingPoint() { return binding_point_; }

    /// Destroys this instruction along with any assignment instructions, if the var is never read.
    void DestroyIfOnlyAssigned();

    /// @returns the friendly name for the instruction
    std::string_view FriendlyName() override { return "var"; }

  private:
    std::optional<struct BindingPoint> binding_point_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_VAR_H_
