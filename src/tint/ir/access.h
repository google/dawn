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

#ifndef SRC_TINT_IR_ACCESS_H_
#define SRC_TINT_IR_ACCESS_H_

#include "src/tint/ir/operand_instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// An access instruction in the IR.
class Access : public utils::Castable<Access, OperandInstruction<3>> {
  public:
    /// Constructor
    /// @param result_type the result type
    /// @param object the accessor object
    /// @param indices the indices to access
    Access(const type::Type* result_type, Value* object, utils::VectorRef<Value*> indices);
    ~Access() override;

    /// @returns the type of the value
    const type::Type* Type() const override { return result_type_; }

    /// @returns the object used for the access
    Value* Object() const { return operands_[0]; }

    /// @returns the accessor indices
    utils::Slice<Value const* const> Indices() const {
        return operands_.Slice().Offset(1).Reinterpret<Value const* const>();
    }

    /// @returns the accessor indices
    utils::Slice<Value*> Indices() { return operands_.Slice().Offset(1); }

  private:
    const type::Type* result_type_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_ACCESS_H_
