// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_IR_VALUE_H_
#define SRC_TINT_IR_VALUE_H_

#include "src/tint/type/type.h"
#include "src/tint/utils/castable.h"
#include "src/tint/utils/unique_vector.h"

// Forward declarations
namespace tint::ir {
class Instruction;
}  // namespace tint::ir

namespace tint::ir {

/// Value in the IR.
class Value : public utils::Castable<Value> {
  public:
    /// Destructor
    ~Value() override;

    Value(const Value&) = delete;
    Value(Value&&) = delete;

    Value& operator=(const Value&) = delete;
    Value& operator=(Value&&) = delete;

    /// Adds an instruction which uses this value.
    /// @param inst the instruction
    void AddUsage(const Instruction* inst) { uses_.Add(inst); }

    /// @returns the vector of instructions which use this value. An instruction will only be
    /// returned once even if that instruction uses the given value multiple times.
    utils::VectorRef<const Instruction*> Usage() const { return uses_; }

    /// @returns the type of the value
    virtual const type::Type* Type() const { return nullptr; }

  protected:
    /// Constructor
    Value();

  private:
    utils::UniqueVector<const Instruction*, 4> uses_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_VALUE_H_
