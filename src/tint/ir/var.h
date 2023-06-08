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

#ifndef SRC_TINT_IR_VAR_H_
#define SRC_TINT_IR_VAR_H_

#include "src/tint/builtin/access.h"
#include "src/tint/builtin/address_space.h"
#include "src/tint/ir/binding_point.h"
#include "src/tint/ir/operand_instruction.h"
#include "src/tint/type/pointer.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/vector.h"

namespace tint::ir {

/// A var instruction in the IR.
class Var : public utils::Castable<Var, OperandInstruction<1>> {
  public:
    /// Constructor
    /// @param type the type of the var
    explicit Var(const type::Pointer* type);
    ~Var() override;

    /// @returns the type of the var
    const type::Pointer* Type() override { return type_; }

    /// Sets the var initializer
    /// @param initializer the initializer
    void SetInitializer(Value* initializer);
    /// @returns the initializer
    Value* Initializer() { return operands_[0]; }

    /// Sets the binding point
    /// @param group the group
    /// @param binding the binding
    void SetBindingPoint(uint32_t group, uint32_t binding) { binding_point_ = {group, binding}; }
    /// @returns the binding points if `Attributes` contains `kBindingPoint`
    std::optional<struct BindingPoint> BindingPoint() { return binding_point_; }

  private:
    const type::Pointer* type_ = nullptr;
    std::optional<struct BindingPoint> binding_point_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_VAR_H_
