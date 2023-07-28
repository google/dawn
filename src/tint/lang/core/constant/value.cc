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

#include "src/tint/lang/core/constant/value.h"

#include "src/tint/lang/core/constant/splat.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::constant::Value);

namespace tint::constant {

Value::Value() = default;

Value::~Value() = default;

/// Equal returns true if the constants `a` and `b` are of the same type and value.
bool Value::Equal(const constant::Value* b) const {
    if (this == b) {
        return true;
    }
    if (Hash() != b->Hash()) {
        return false;
    }
    if (Type() != b->Type()) {
        return false;
    }

    auto elements_equal = [&](size_t count) {
        if (count == 0) {
            return true;
        }

        // Avoid per-element comparisons if the constants are splats
        bool a_is_splat = Is<Splat>();
        bool b_is_splat = b->Is<Splat>();
        if (a_is_splat && b_is_splat) {
            return Index(0)->Equal(b->Index(0));
        }

        if (a_is_splat) {
            auto* el_a = Index(0);
            for (size_t i = 0; i < count; i++) {
                if (!el_a->Equal(b->Index(i))) {
                    return false;
                }
            }
            return true;
        }

        if (b_is_splat) {
            auto* el_b = b->Index(0);
            for (size_t i = 0; i < count; i++) {
                if (!Index(i)->Equal(el_b)) {
                    return false;
                }
            }
            return true;
        }

        // Per-element comparison
        for (size_t i = 0; i < count; i++) {
            if (!Index(i)->Equal(b->Index(i))) {
                return false;
            }
        }
        return true;
    };

    return Switch(
        Type(),  //
        [&](const type::Vector* vec) { return elements_equal(vec->Width()); },
        [&](const type::Matrix* mat) { return elements_equal(mat->columns()); },
        [&](const type::Struct* str) { return elements_equal(str->Members().Length()); },
        [&](const type::Array* arr) {
            if (auto n = arr->ConstantCount()) {
                return elements_equal(*n);
            }
            return false;
        },
        [&](Default) {
            auto va = InternalValue();
            auto vb = b->InternalValue();
            TINT_ASSERT(!std::holds_alternative<std::monostate>(va));
            TINT_ASSERT(!std::holds_alternative<std::monostate>(vb));
            return va == vb;
        });
}

}  // namespace tint::constant
