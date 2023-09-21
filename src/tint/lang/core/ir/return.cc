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

#include "src/tint/lang/core/ir/return.h"

#include <utility>

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::Return);

namespace tint::core::ir {

Return::Return(Function* func) {
    AddOperand(Return::kFunctionOperandOffset, func);
}

Return::Return(Function* func, ir::Value* arg) {
    AddOperand(Return::kFunctionOperandOffset, func);
    AddOperand(Return::kArgOperandOffset, arg);
}

Return::~Return() = default;

Return* Return::Clone(CloneContext& ctx) {
    auto* new_func = ctx.Clone(Func());
    auto new_val = Value() ? ctx.Clone(Value()) : nullptr;
    return ctx.ir.instructions.Create<Return>(new_func, new_val);
}

Function* Return::Func() const {
    return tint::As<Function>(operands_[kFunctionOperandOffset]);
}

}  // namespace tint::core::ir
