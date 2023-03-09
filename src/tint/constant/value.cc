// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#include "src/tint/constant/value.h"

#include "src/tint/switch.h"
#include "src/tint/type/array.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/struct.h"
#include "src/tint/type/vector.h"

TINT_INSTANTIATE_TYPEINFO(tint::constant::Value);

namespace tint::constant {

Value::Value() = default;

Value::~Value() = default;

/// Equal returns true if the constants `a` and `b` are of the same type and value.
bool Value::Equal(const constant::Value* b) const {
    if (Hash() != b->Hash()) {
        return false;
    }
    if (Type() != b->Type()) {
        return false;
    }
    return Switch(
        Type(),  //
        [&](const type::Vector* vec) {
            for (size_t i = 0; i < vec->Width(); i++) {
                if (!Index(i)->Equal(b->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](const type::Matrix* mat) {
            for (size_t i = 0; i < mat->columns(); i++) {
                if (!Index(i)->Equal(b->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](const type::Array* arr) {
            if (auto count = arr->ConstantCount()) {
                for (size_t i = 0; i < count; i++) {
                    if (!Index(i)->Equal(b->Index(i))) {
                        return false;
                    }
                }
                return true;
            }

            return false;
        },
        [&](const type::Struct* str) {
            auto count = str->Members().Length();
            for (size_t i = 0; i < count; i++) {
                if (!Index(i)->Equal(b->Index(i))) {
                    return false;
                }
            }
            return true;
        },
        [&](Default) {
            auto va = InternalValue();
            auto vb = b->InternalValue();
            TINT_ASSERT(Resolver, !std::holds_alternative<std::monostate>(va));
            TINT_ASSERT(Resolver, !std::holds_alternative<std::monostate>(vb));
            return va == vb;
        });
}

}  // namespace tint::constant
