// Copyright 2022 The Tint Authors.
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

#include "src/tint/ir/constant.h"

#include <string>

TINT_INSTANTIATE_TYPEINFO(tint::ir::Constant);

namespace tint::ir {

Constant::Constant(f32 f) : kind_(Kind::kF32), data_(f) {}

Constant::Constant(f16 f) : kind_(Kind::kF16), data_(f) {}

Constant::Constant(u32 u) : kind_(Kind::kU32), data_(u) {}

Constant::Constant(i32 i) : kind_(Kind::kI32), data_(i) {}

Constant::Constant(bool b) : kind_(Kind::kBool), data_(b) {}

Constant::~Constant() = default;

std::ostream& Constant::ToString(std::ostream& out) const {
    switch (GetKind()) {
        case Constant::Kind::kF32:
            out << std::to_string(AsF32().value);
            break;
        case Constant::Kind::kF16:
            out << std::to_string(AsF16().value);
            break;
        case Constant::Kind::kI32:
            out << std::to_string(AsI32().value);
            break;
        case Constant::Kind::kU32:
            out << std::to_string(AsU32().value);
            break;
        case Constant::Kind::kBool:
            out << (AsBool() ? "true" : "false");
            break;
    }
    return out;
}

}  // namespace tint::ir
