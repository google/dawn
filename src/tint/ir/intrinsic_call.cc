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

#include "src/tint/ir/intrinsic_call.h"

#include <utility>

#include "src/tint/debug.h"

TINT_INSTANTIATE_TYPEINFO(tint::ir::IntrinsicCall);

namespace tint::ir {

IntrinsicCall::IntrinsicCall(InstructionResult* result,
                             enum Kind kind,
                             utils::VectorRef<Value*> arguments)
    : kind_(kind) {
    AddOperands(IntrinsicCall::kArgsOperandOffset, std::move(arguments));
    AddResult(result);
}

IntrinsicCall::~IntrinsicCall() = default;

utils::StringStream& operator<<(utils::StringStream& out, enum IntrinsicCall::Kind kind) {
    switch (kind) {
        case IntrinsicCall::Kind::kSpirvDot:
            out << "spirv.dot";
            break;
        case IntrinsicCall::Kind::kSpirvSelect:
            out << "spirv.select";
            break;
    }
    return out;
}

}  // namespace tint::ir
