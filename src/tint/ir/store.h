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

#ifndef SRC_TINT_IR_STORE_H_
#define SRC_TINT_IR_STORE_H_

#include "src/tint/ir/instruction.h"
#include "src/tint/utils/castable.h"

namespace tint::ir {

/// An instruction in the IR.
class Store : public utils::Castable<Store, Instruction> {
  public:
    /// Constructor
    /// @param to the value to store too
    /// @param from the value being stored from
    Store(Value* to, Value* from);
    Store(const Store& inst) = delete;
    Store(Store&& inst) = delete;
    ~Store() override;

    Store& operator=(const Store& inst) = delete;
    Store& operator=(Store&& inst) = delete;

    /// the value being stored to
    Value* to = nullptr;
    /// the value being stored
    Value* from = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_STORE_H_
