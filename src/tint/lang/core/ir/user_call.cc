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

#include "src/tint/lang/core/ir/user_call.h"

#include <utility>

#include "src/tint/lang/core/ir/clone_context.h"
#include "src/tint/lang/core/ir/module.h"

TINT_INSTANTIATE_TYPEINFO(tint::core::ir::UserCall);

namespace tint::core::ir {

UserCall::UserCall(InstructionResult* result, Function* func, VectorRef<Value*> arguments) {
    flags_.Add(Flag::kSequenced);
    AddOperand(UserCall::kFunctionOperandOffset, func);
    AddOperands(UserCall::kArgsOperandOffset, std::move(arguments));
    AddResult(result);
}

UserCall::~UserCall() = default;

UserCall* UserCall::Clone(CloneContext& ctx) {
    auto* new_result = ctx.Clone(Result());
    auto* target = ctx.Remap(Target());
    auto args = ctx.Remap<UserCall::kDefaultNumOperands>(Args());
    return ctx.ir.instructions.Create<UserCall>(new_result, target, args);
}

void UserCall::SetArgs(VectorRef<Value*> arguments) {
    auto* fn = Target();
    ClearOperands();
    AddOperand(UserCall::kFunctionOperandOffset, fn);
    AddOperands(UserCall::kArgsOperandOffset, std::move(arguments));
}

}  // namespace tint::core::ir
