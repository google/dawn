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

#ifndef SRC_TINT_IR_UNREACHABLE_H_
#define SRC_TINT_IR_UNREACHABLE_H_

#include "src/tint/ir/terminator.h"

namespace tint::ir {

/// An unreachable instruction in the IR.
class Unreachable : public utils::Castable<Unreachable, Terminator> {
  public:
    ~Unreachable() override;

    /// @returns the friendly name for the instruction
    std::string_view FriendlyName() override { return "unreachable"; }
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_UNREACHABLE_H_
