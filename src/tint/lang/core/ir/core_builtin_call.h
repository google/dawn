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

#ifndef SRC_TINT_LANG_CORE_IR_CORE_BUILTIN_CALL_H_
#define SRC_TINT_LANG_CORE_IR_CORE_BUILTIN_CALL_H_

#include "src/tint/lang/core/function.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::ir {

/// A core builtin call instruction in the IR.
class CoreBuiltinCall : public Castable<CoreBuiltinCall, BuiltinCall> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param func the builtin function
    /// @param args the conversion arguments
    CoreBuiltinCall(InstructionResult* result,
                    core::Function func,
                    VectorRef<Value*> args = tint::Empty);
    ~CoreBuiltinCall() override;

    /// @returns the builtin function
    core::Function Func() { return func_; }

    /// @returns the friendly name for the instruction
    std::string_view FriendlyName() override { return "core-builtin-call"; }

  private:
    core::Function func_;
};

}  // namespace tint::ir

#endif  // SRC_TINT_LANG_CORE_IR_CORE_BUILTIN_CALL_H_
