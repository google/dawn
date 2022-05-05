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

#include "src/tint/sem/constant.h"

#include <functional>
#include <utility>

#include "src/tint/debug.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/type.h"

namespace tint::sem {

namespace {

const Type* ElemType(const Type* ty, size_t num_elements) {
    diag::List diag;
    if (ty->is_scalar()) {
        if (num_elements != 1) {
            TINT_ICE(Semantic, diag) << "sem::Constant() type <-> num_element mismatch. type: '"
                                     << ty->TypeInfo().name << "' num_elements: " << num_elements;
        }
        return ty;
    }
    if (auto* vec = ty->As<Vector>()) {
        if (num_elements != vec->Width()) {
            TINT_ICE(Semantic, diag) << "sem::Constant() type <-> num_element mismatch. type: '"
                                     << ty->TypeInfo().name << "' num_elements: " << num_elements;
        }
        TINT_ASSERT(Semantic, vec->type()->is_scalar());
        return vec->type();
    }
    TINT_UNREACHABLE(Semantic, diag) << "Unsupported sem::Constant type";
    return nullptr;
}

}  // namespace

Constant::Constant() {}

Constant::Constant(const sem::Type* ty, Scalars els)
    : type_(ty), elem_type_(ElemType(ty, els.size())), elems_(std::move(els)) {}

Constant::Constant(const Constant&) = default;

Constant::~Constant() = default;

Constant& Constant::operator=(const Constant& rhs) = default;

bool Constant::AnyZero() const {
    for (size_t i = 0; i < Elements().size(); ++i) {
        if (WithScalarAt(i, [&](auto&& s) {
                // Use std::equal_to to work around -Wfloat-equal warnings
                using T = std::remove_reference_t<decltype(s)>;
                auto equal_to = std::equal_to<T>{};
                if (equal_to(s, T(0))) {
                    return true;
                }
                return false;
            })) {
            return true;
        }
    }
    return false;
}

}  // namespace tint::sem
