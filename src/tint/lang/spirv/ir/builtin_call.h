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

#ifndef SRC_TINT_LANG_SPIRV_IR_BUILTIN_CALL_H_
#define SRC_TINT_LANG_SPIRV_IR_BUILTIN_CALL_H_

#include <string>

#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/ir/builtin_call.h"
#include "src/tint/lang/spirv/intrinsic/data/data.h"
#include "src/tint/lang/spirv/ir/function.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::spirv::ir {

/// A spirv builtin call instruction in the IR.
class BuiltinCall : public Castable<BuiltinCall, core::ir::BuiltinCall> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param func the builtin function
    /// @param args the conversion arguments
    BuiltinCall(core::ir::InstructionResult* result,
                Function func,
                VectorRef<core::ir::Value*> args = tint::Empty);
    ~BuiltinCall() override;

    /// @returns the builtin function
    Function Func() { return func_; }

    /// @returns the identifier for the function
    size_t FuncId() override { return static_cast<size_t>(func_); }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return str(func_); }

    /// @returns the intrinsic name
    const char* IntrinsicName() override { return str(func_); }

    /// @returns the table data to validate this builtin
    const core::intrinsic::TableData& TableData() override { return spirv::intrinsic::data::kData; }

  private:
    Function func_;
};

}  // namespace tint::spirv::ir

#endif  // SRC_TINT_LANG_SPIRV_IR_BUILTIN_CALL_H_
