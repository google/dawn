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

#ifndef SRC_TINT_IR_CONTROL_INSTRUCTION_H_
#define SRC_TINT_IR_CONTROL_INSTRUCTION_H_

#include "src/tint/ir/branch.h"

namespace tint::ir {

/// Base class of instructions that perform branches to two or more blocks, owned by the
/// ControlInstruction.
class ControlInstruction : public utils::Castable<ControlInstruction, Branch> {
  public:
    /// Destructor
    ~ControlInstruction() override;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_CONTROL_INSTRUCTION_H_
