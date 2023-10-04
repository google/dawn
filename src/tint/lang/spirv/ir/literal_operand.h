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

#ifndef SRC_TINT_LANG_SPIRV_IR_LITERAL_OPERAND_H_
#define SRC_TINT_LANG_SPIRV_IR_LITERAL_OPERAND_H_

#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/utils/rtti/castable.h"

namespace tint::spirv::ir {

/// LiteralOperand is a type of constant value that is intended to be emitted as a literal in
/// the SPIR-V instruction stream.
class LiteralOperand final : public Castable<LiteralOperand, core::ir::Constant> {
  public:
    /// Constructor
    /// @param value the operand value
    explicit LiteralOperand(const core::constant::Value* value);
    /// Destructor
    ~LiteralOperand() override;
};

}  // namespace tint::spirv::ir

#endif  // SRC_TINT_LANG_SPIRV_IR_LITERAL_OPERAND_H_
