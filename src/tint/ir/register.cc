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

#include "src/tint/ir/register.h"

namespace tint::ir {

Register::Register() : kind_(Kind::kUninitialized), data_(Id(0)) {}

Register::Register(Id id) : kind_(Kind::kTemp), data_(id) {}

Register::Register(f32 f) : kind_(Kind::kF32), data_(f) {}

Register::Register(f16 f) : kind_(Kind::kF16), data_(f) {}

Register::Register(u32 u) : kind_(Kind::kU32), data_(u) {}

Register::Register(i32 i) : kind_(Kind::kI32), data_(i) {}

Register::Register(bool b) : kind_(Kind::kBool), data_(b) {}

Register::Register(Symbol s, Id id) : kind_(Kind::kVar), data_(VarData{s, id}) {}

Register::~Register() = default;

Register::Register(const Register& o) = default;

Register::Register(Register&& o) = default;

Register& Register::operator=(const Register& o) = default;

Register& Register::operator=(Register&& o) = default;

std::ostream& operator<<(std::ostream& out, const Register& r) {
    switch (r.GetKind()) {
        case Register::Kind::kTemp:
            out << "%" << std::to_string(r.AsId());
            break;
        case Register::Kind::kF32:
            out << std::to_string(r.AsF32().value);
            break;
        case Register::Kind::kF16:
            out << std::to_string(r.AsF16().value);
            break;
        case Register::Kind::kI32:
            out << std::to_string(r.AsI32().value);
            break;
        case Register::Kind::kU32:
            out << std::to_string(r.AsU32().value);
            break;
            // TODO(dsinclair): Emit the symbol instead of v
        case Register::Kind::kVar:
            out << "%v" << std::to_string(r.AsVarData().id);
            break;
        case Register::Kind::kBool:
            out << (r.AsBool() ? "true" : "false");
            break;
        case Register::Kind::kUninitialized:
            out << "unknown register";
            break;
    }
    return out;
}

}  // namespace tint::ir
