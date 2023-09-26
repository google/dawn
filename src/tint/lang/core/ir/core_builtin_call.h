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

#include <string>

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/intrinsic/dialect.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::core::ir {

/// A core builtin call instruction in the IR.
class CoreBuiltinCall final : public Castable<CoreBuiltinCall, BuiltinCall> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param func the builtin function
    /// @param args the conversion arguments
    CoreBuiltinCall(InstructionResult* result,
                    core::BuiltinFn func,
                    VectorRef<Value*> args = tint::Empty);
    ~CoreBuiltinCall() override;

    /// @copydoc Instruction::Clone()
    CoreBuiltinCall* Clone(CloneContext& ctx) override;

    /// @returns the builtin function
    core::BuiltinFn Func() { return func_; }

    /// @returns the identifier for the function
    size_t FuncId() override { return static_cast<size_t>(func_); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return core::str(func_); }

    /// @returns the table data to validate this builtin
    const core::intrinsic::TableData& TableData() override {
        return core::intrinsic::Dialect::kData;
    }

  private:
    core::BuiltinFn func_;
};

}  // namespace tint::core::ir

#endif  // SRC_TINT_LANG_CORE_IR_CORE_BUILTIN_CALL_H_
