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

#include "src/tint/lang/core/type/type.h"

#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/reference.h"
#include "src/tint/lang/core/type/sampler.h"
#include "src/tint/lang/core/type/struct.h"
#include "src/tint/lang/core/type/texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"
#include "src/tint/utils/rtti/switch.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Type);

namespace tint::type {

Type::Type(size_t hash, type::Flags flags) : Base(hash), flags_(flags) {
    if (IsConstructible()) {
        TINT_ASSERT(HasCreationFixedFootprint());
    }
}

Type::~Type() = default;

const Type* Type::UnwrapPtr() const {
    auto* type = this;
    while (auto* ptr = type->As<Pointer>()) {
        type = ptr->StoreType();
    }
    return type;
}

const Type* Type::UnwrapRef() const {
    auto* type = this;
    if (auto* ref = type->As<Reference>()) {
        type = ref->StoreType();
    }
    return type;
}

uint32_t Type::Size() const {
    return 0;
}

uint32_t Type::Align() const {
    return 0;
}

bool Type::is_float_scalar() const {
    return IsAnyOf<F16, F32, AbstractFloat>();
}

bool Type::is_float_matrix() const {
    return Is([](const Matrix* m) { return m->type()->is_float_scalar(); });
}

bool Type::is_square_float_matrix() const {
    return Is(
        [](const Matrix* m) { return m->type()->is_float_scalar() && m->rows() == m->columns(); });
}

bool Type::is_float_vector() const {
    return Is([](const Vector* v) { return v->type()->is_float_scalar(); });
}

bool Type::is_float_scalar_or_vector() const {
    return is_float_scalar() || is_float_vector();
}

bool Type::is_float_scalar_or_vector_or_matrix() const {
    return is_float_scalar() || is_float_vector() || is_float_matrix();
}

bool Type::is_integer_scalar() const {
    return IsAnyOf<U32, I32>();
}

bool Type::is_signed_integer_scalar() const {
    return IsAnyOf<I32, AbstractInt>();
}

bool Type::is_unsigned_integer_scalar() const {
    return Is<U32>();
}

bool Type::is_signed_integer_vector() const {
    return Is([](const Vector* v) { return v->type()->IsAnyOf<I32, AbstractInt>(); });
}

bool Type::is_unsigned_integer_vector() const {
    return Is([](const Vector* v) { return v->type()->Is<U32>(); });
}

bool Type::is_unsigned_integer_scalar_or_vector() const {
    return Is<U32>() || is_unsigned_integer_vector();
}

bool Type::is_signed_integer_scalar_or_vector() const {
    return IsAnyOf<I32, AbstractInt>() || is_signed_integer_vector();
}

bool Type::is_integer_scalar_or_vector() const {
    return is_unsigned_integer_scalar_or_vector() || is_signed_integer_scalar_or_vector();
}

bool Type::is_abstract_integer_vector() const {
    return Is([](const Vector* v) { return v->type()->Is<AbstractInt>(); });
}

bool Type::is_abstract_float_vector() const {
    return Is([](const Vector* v) { return v->type()->Is<AbstractFloat>(); });
}

bool Type::is_abstract_integer_scalar_or_vector() const {
    return Is<AbstractInt>() || is_abstract_integer_vector();
}

bool Type::is_abstract_float_scalar_or_vector() const {
    return Is<AbstractFloat>() || is_abstract_float_vector();
}

bool Type::is_bool_vector() const {
    return Is([](const Vector* v) { return v->type()->Is<Bool>(); });
}

bool Type::is_bool_scalar_or_vector() const {
    return Is<Bool>() || is_bool_vector();
}

bool Type::is_numeric_vector() const {
    return Is([](const Vector* v) { return v->type()->Is<type::NumericScalar>(); });
}

bool Type::is_scalar_vector() const {
    return Is([](const Vector* v) { return v->type()->Is<type::Scalar>(); });
}

bool Type::is_numeric_scalar_or_vector() const {
    return Is<type::NumericScalar>() || is_numeric_vector();
}

bool Type::is_handle() const {
    return IsAnyOf<Sampler, Texture>();
}

bool Type::HoldsAbstract() const {
    return Switch(
        this,  //
        [&](const AbstractNumeric*) { return true; },
        [&](const Vector* v) { return v->type()->HoldsAbstract(); },
        [&](const Matrix* m) { return m->type()->HoldsAbstract(); },
        [&](const Array* a) { return a->ElemType()->HoldsAbstract(); },
        [&](const Struct* s) {
            for (auto* m : s->Members()) {
                if (m->Type()->HoldsAbstract()) {
                    return true;
                }
            }
            return false;
        });
}

uint32_t Type::ConversionRank(const Type* from, const Type* to) {
    if (from->UnwrapRef() == to) {
        return 0;
    }
    return Switch(
        from,
        [&](const AbstractFloat*) {
            return Switch(
                to,                             //
                [&](const F32*) { return 1; },  //
                [&](const F16*) { return 2; },  //
                [&](Default) { return kNoConversion; });
        },
        [&](const AbstractInt*) {
            return Switch(
                to,                                       //
                [&](const I32*) { return 3; },            //
                [&](const U32*) { return 4; },            //
                [&](const AbstractFloat*) { return 5; },  //
                [&](const F32*) { return 6; },            //
                [&](const F16*) { return 7; },            //
                [&](Default) { return kNoConversion; });
        },
        [&](const Vector* from_vec) {
            if (auto* to_vec = to->As<Vector>()) {
                if (from_vec->Width() == to_vec->Width()) {
                    return ConversionRank(from_vec->type(), to_vec->type());
                }
            }
            return kNoConversion;
        },
        [&](const Matrix* from_mat) {
            if (auto* to_mat = to->As<Matrix>()) {
                if (from_mat->columns() == to_mat->columns() &&
                    from_mat->rows() == to_mat->rows()) {
                    return ConversionRank(from_mat->type(), to_mat->type());
                }
            }
            return kNoConversion;
        },
        [&](const Array* from_arr) {
            if (auto* to_arr = to->As<Array>()) {
                if (from_arr->Count() == to_arr->Count()) {
                    return ConversionRank(from_arr->ElemType(), to_arr->ElemType());
                }
            }
            return kNoConversion;
        },
        [&](const Struct* from_str) {
            auto concrete_tys = from_str->ConcreteTypes();
            for (size_t i = 0; i < concrete_tys.Length(); i++) {
                if (concrete_tys[i] == to) {
                    return static_cast<uint32_t>(i + 1);
                }
            }
            return kNoConversion;
        },
        [&](Default) { return kNoConversion; });
}

TypeAndCount Type::Elements(const Type* type_if_invalid /* = nullptr */,
                            uint32_t count_if_invalid /* = 0 */) const {
    return {type_if_invalid, count_if_invalid};
}

const Type* Type::Element(uint32_t /* index */) const {
    return nullptr;
}

const Type* Type::DeepestElement() const {
    const Type* ty = this;
    while (true) {
        auto [el, n] = ty->Elements();
        if (!el) {
            return ty;
        }
        ty = el;
    }
}

const Type* Type::Common(VectorRef<const Type*> types) {
    const auto count = types.Length();
    if (count == 0) {
        return nullptr;
    }
    const auto* common = types[0];
    for (size_t i = 1; i < count; i++) {
        auto* ty = types[i];
        if (ty == common) {
            continue;  // ty == common
        }
        if (Type::ConversionRank(ty, common) != Type::kNoConversion) {
            continue;  // ty can be converted to common.
        }
        if (Type::ConversionRank(common, ty) != Type::kNoConversion) {
            common = ty;  // common can be converted to ty.
            continue;
        }
        return nullptr;  // Conversion is not valid.
    }
    return common;
}

}  // namespace tint::type
