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

#include <cmath>
#include <utility>

#include "src/tint/debug.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/type.h"

namespace tint::sem {

namespace {
size_t CountElements(const Constant::Elements& elements) {
    return std::visit([](auto&& vec) { return vec.size(); }, elements);
}

template <typename T>
bool IsNegativeFloat(T value) {
    (void)value;
    if constexpr (IsFloatingPoint<T>) {
        return std::signbit(value);
    } else {
        return false;
    }
}

}  // namespace

Constant::Constant() {}

Constant::Constant(const sem::Type* ty, Elements els)
    : type_(ty), elem_type_(CheckElemType(ty, CountElements(els))), elems_(std::move(els)) {}

Constant::Constant(const sem::Type* ty, AInts vec) : Constant(ty, Elements{std::move(vec)}) {}

Constant::Constant(const sem::Type* ty, AFloats vec) : Constant(ty, Elements{std::move(vec)}) {}

Constant::Constant(const Constant&) = default;

Constant::~Constant() = default;

Constant& Constant::operator=(const Constant& rhs) = default;

bool Constant::AnyZero() const {
    return WithElements([&](auto&& vec) {
        using T = typename std::decay_t<decltype(vec)>::value_type;
        for (auto el : vec) {
            if (el == T(0) && !IsNegativeFloat(el.value)) {
                return true;
            }
        }
        return false;
    });
}

bool Constant::AllZero() const {
    return WithElements([&](auto&& vec) {
        using T = typename std::decay_t<decltype(vec)>::value_type;
        for (auto el : vec) {
            if (el != T(0) || IsNegativeFloat(el.value)) {
                return false;
            }
        }
        return true;
    });
}

bool Constant::AllEqual(size_t start, size_t end) const {
    return WithElements([&](auto&& vec) {
        if (!vec.empty()) {
            auto value = vec[start];
            bool float_sign = IsNegativeFloat(vec[start].value);
            for (size_t i = start + 1; i < end; i++) {
                if (vec[i] != value || float_sign != IsNegativeFloat(vec[i].value)) {
                    return false;
                }
            }
        }
        return true;
    });
}

const Type* Constant::CheckElemType(const sem::Type* ty, size_t num_elements) {
    diag::List diag;
    if (ty->is_abstract_or_scalar() || ty->IsAnyOf<Vector, Matrix>()) {
        uint32_t count = 0;
        auto* el_ty = Type::ElementOf(ty, &count);
        if (num_elements != count) {
            TINT_ICE(Semantic, diag) << "sem::Constant() type <-> element mismatch. type: '"
                                     << ty->TypeInfo().name << "' element: " << num_elements;
        }
        TINT_ASSERT(Semantic, el_ty->is_abstract_or_scalar());
        return el_ty;
    }
    TINT_UNREACHABLE(Semantic, diag) << "Unsupported sem::Constant type: " << ty->TypeInfo().name;
    return nullptr;
}

}  // namespace tint::sem
