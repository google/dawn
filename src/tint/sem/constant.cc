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

#include <utility>

#include "src/tint/debug.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/type.h"

namespace tint::sem {

namespace {

const Type* CheckElemType(const Type* ty, size_t num_scalars) {
    diag::List diag;
    if (ty->is_abstract_or_scalar() || ty->IsAnyOf<Vector, Matrix>()) {
        uint32_t count = 0;
        auto* el_ty = Type::ElementOf(ty, &count);
        if (num_scalars != count) {
            TINT_ICE(Semantic, diag) << "sem::Constant() type <-> scalar mismatch. type: '"
                                     << ty->TypeInfo().name << "' scalar: " << num_scalars;
        }
        TINT_ASSERT(Semantic, el_ty->is_abstract_or_scalar());
        return el_ty;
    }
    TINT_UNREACHABLE(Semantic, diag) << "Unsupported sem::Constant type: " << ty->TypeInfo().name;
    return nullptr;
}

}  // namespace

Constant::Constant() {}

Constant::Constant(const sem::Type* ty, Scalars els)
    : type_(ty), elem_type_(CheckElemType(ty, els.size())), elems_(std::move(els)) {}

Constant::Constant(const Constant&) = default;

Constant::~Constant() = default;

Constant& Constant::operator=(const Constant& rhs) = default;

bool Constant::AnyZero() const {
    for (auto scalar : elems_) {
        auto is_zero = [&](auto&& s) {
            using T = std::remove_reference_t<decltype(s)>;
            return s == T(0);
        };
        if (std::visit(is_zero, scalar)) {
            return true;
        }
    }
    return false;
}

}  // namespace tint::sem
