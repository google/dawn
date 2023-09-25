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

#ifndef SRC_TINT_LANG_WGSL_IR_BUILTIN_CALL_H_
#define SRC_TINT_LANG_WGSL_IR_BUILTIN_CALL_H_

#include <string>

#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/intrinsic/data/data.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::wgsl::ir {

/// A WGSL builtin call instruction in the IR.
class BuiltinCall : public Castable<BuiltinCall, core::ir::BuiltinCall> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param fn the builtin function
    /// @param args the conversion arguments
    BuiltinCall(core::ir::InstructionResult* result,
                BuiltinFn fn,
                VectorRef<core::ir::Value*> args = tint::Empty);
    ~BuiltinCall() override;

    /// @copydoc core::ir::Instruction::Clone()
    BuiltinCall* Clone(core::ir::CloneContext& ctx) override;

    /// @returns the builtin function
    BuiltinFn Func() { return fn_; }

    /// @returns the identifier for the function
    size_t FuncId() override { return static_cast<size_t>(fn_); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return std::string("wgsl.") + str(fn_); }

    /// @returns the table data to validate this builtin
    const core::intrinsic::TableData& TableData() override { return intrinsic::data::kData; }

  private:
    BuiltinFn fn_;
};

}  // namespace tint::wgsl::ir

#endif  // SRC_TINT_LANG_WGSL_IR_BUILTIN_CALL_H_
