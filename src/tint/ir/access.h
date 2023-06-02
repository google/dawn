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

#include "src/tint/ir/instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// An access instruction in the IR.
class Access : public utils::Castable<Access, Instruction> {
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
    Value* Object() const { return object_; }

    /// @returns the accessor indices
    utils::VectorRef<Value*> Indices() const { return indices_; }

  private:
    const type::Type* result_type_ = nullptr;
    Value* object_ = nullptr;
    utils::Vector<Value*, 1> indices_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_ACCESS_H_
