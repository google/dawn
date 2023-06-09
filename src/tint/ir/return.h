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

#ifndef SRC_TINT_IR_RETURN_H_
#define SRC_TINT_IR_RETURN_H_

#include "src/tint/ir/branch.h"
#include "src/tint/utils/castable.h"

// Forward declarations
namespace tint::ir {
class Function;
}  // namespace tint::ir

namespace tint::ir {

/// A return instruction.
class Return : public utils::Castable<Return, Branch> {
  public:
    /// Constructor (no return value)
    /// @param func the function being returned
    explicit Return(Function* func);

    /// Constructor
    /// @param func the function being returned
    /// @param arg the return value
    Return(Function* func, Value* arg);

    ~Return() override;

    /// @returns the function being returned
    Function* Func() { return func_; }

  private:
    Function* func_ = nullptr;
};

}  // namespace tint::ir

#endif  // SRC_TINT_IR_RETURN_H_
