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

#include "src/tint/lang/spirv/ir/builtin_call.h"

#include <utility>

#include "src/tint/utils/ice/ice.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::ir::BuiltinCall);

namespace tint::spirv::ir {

BuiltinCall::BuiltinCall(core::ir::InstructionResult* result,
                         Function func,
                         VectorRef<core::ir::Value*> arguments)
    : Base(result, arguments), func_(func) {
    TINT_ASSERT(func != Function::kNone);
}

BuiltinCall::~BuiltinCall() = default;

}  // namespace tint::spirv::ir
