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

#include "src/tint/ir/value.h"

namespace tint::ir {

Value::Value(Id id) : kind_(Kind::kTemp), data_(id) {}

Value::Value(f32 f) : kind_(Kind::kF32), data_(f) {}

Value::Value(f16 f) : kind_(Kind::kF16), data_(f) {}

Value::Value(u32 u) : kind_(Kind::kU32), data_(u) {}

Value::Value(i32 i) : kind_(Kind::kI32), data_(i) {}

Value::Value(bool b) : kind_(Kind::kBool), data_(b) {}

Value::~Value() = default;

Value::Value(const Value& o) = default;

Value::Value(Value&& o) = default;

Value& Value::operator=(const Value& o) = default;

Value& Value::operator=(Value&& o) = default;

std::ostream& operator<<(std::ostream& out, const Value& r) {
    switch (r.GetKind()) {
        case Value::Kind::kTemp:
            out << "%" << std::to_string(r.AsId());
            break;
        case Value::Kind::kF32:
            out << std::to_string(r.AsF32().value);
            break;
        case Value::Kind::kF16:
            out << std::to_string(r.AsF16().value);
            break;
        case Value::Kind::kI32:
            out << std::to_string(r.AsI32().value);
            break;
        case Value::Kind::kU32:
            out << std::to_string(r.AsU32().value);
            break;
        case Value::Kind::kBool:
            out << (r.AsBool() ? "true" : "false");
            break;
    }
    return out;
}

}  // namespace tint::ir
