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

#ifndef SRC_TINT_LANG_SPIRV_IR_INTRINSIC_CALL_H_
#define SRC_TINT_LANG_SPIRV_IR_INTRINSIC_CALL_H_

#include <string>

#include "src/tint/lang/core/ir/intrinsic_call.h"
#include "src/tint/lang/spirv/ir/intrinsic.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::spirv::ir {

/// A spir-v intrinsic call instruction in the IR.
class IntrinsicCall : public Castable<IntrinsicCall, core::ir::IntrinsicCall> {
  public:
    /// Constructor
    /// @param result the result value
    /// @param intrinsic the kind of intrinsic
    /// @param args the intrinsic call arguments
    IntrinsicCall(core::ir::InstructionResult* result,
                  Intrinsic intrinsic,
                  VectorRef<core::ir::Value*> args = tint::Empty);
    ~IntrinsicCall() override;

    /// @returns the kind of the intrinsic
    Intrinsic Kind() const { return intrinsic_; }

    /// @returns the friendly name for the instruction
    std::string FriendlyName() override { return "spirv." + std::string(ToString(intrinsic_)); }

  private:
    Intrinsic intrinsic_ = Intrinsic::kUndefined;
};

}  // namespace tint::spirv::ir

#endif  // SRC_TINT_LANG_SPIRV_IR_INTRINSIC_CALL_H_
