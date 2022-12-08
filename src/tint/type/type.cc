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

#include "src/tint/type/type.h"

#include "src/tint/sem/array.h"
#include "src/tint/sem/matrix.h"
#include "src/tint/sem/pointer.h"
#include "src/tint/sem/reference.h"
#include "src/tint/sem/struct.h"
#include "src/tint/sem/vector.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/sampler.h"
#include "src/tint/type/texture.h"
#include "src/tint/type/u32.h"

TINT_INSTANTIATE_TYPEINFO(tint::type::Type);

namespace tint::type {

Type::Type(TypeFlags flags) : flags_(flags) {
    if (IsConstructible()) {
        TINT_ASSERT(Type, HasCreationFixedFootprint());
    }
}

Type::Type(Type&&) = default;

Type::~Type() = default;

const Type* Type::UnwrapPtr() const {
    auto* type = this;
    while (auto* ptr = type->As<sem::Pointer>()) {
        type = ptr->StoreType();
    }
    return type;
}

const Type* Type::UnwrapRef() const {
    auto* type = this;
    if (auto* ref = type->As<sem::Reference>()) {
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

bool Type::is_scalar() const {
    return IsAnyOf<type::F16, type::F32, type::U32, type::I32, type::AbstractNumeric, type::Bool>();
}

bool Type::is_numeric_scalar() const {
    return IsAnyOf<type::F16, type::F32, type::U32, type::I32, type::AbstractNumeric>();
}

bool Type::is_float_scalar() const {
    return IsAnyOf<type::F16, type::F32, type::AbstractNumeric>();
}

bool Type::is_float_matrix() const {
    return Is([](const sem::Matrix* m) { return m->type()->is_float_scalar(); });
}

bool Type::is_square_float_matrix() const {
    return Is([](const sem::Matrix* m) {
        return m->type()->is_float_scalar() && m->rows() == m->columns();
    });
}

bool Type::is_float_vector() const {
    return Is([](const sem::Vector* v) { return v->type()->is_float_scalar(); });
}

bool Type::is_float_scalar_or_vector() const {
    return is_float_scalar() || is_float_vector();
}

bool Type::is_float_scalar_or_vector_or_matrix() const {
    return is_float_scalar() || is_float_vector() || is_float_matrix();
}

bool Type::is_integer_scalar() const {
    return IsAnyOf<type::U32, type::I32>();
}

bool Type::is_signed_integer_scalar() const {
    return IsAnyOf<type::I32, type::AbstractInt>();
}

bool Type::is_unsigned_integer_scalar() const {
    return Is<type::U32>();
}

bool Type::is_signed_integer_vector() const {
    return Is(
        [](const sem::Vector* v) { return v->type()->IsAnyOf<type::I32, type::AbstractInt>(); });
}

bool Type::is_unsigned_integer_vector() const {
    return Is([](const sem::Vector* v) { return v->type()->Is<type::U32>(); });
}

bool Type::is_unsigned_integer_scalar_or_vector() const {
    return Is<type::U32>() || is_unsigned_integer_vector();
}

bool Type::is_signed_integer_scalar_or_vector() const {
    return IsAnyOf<type::I32, type::AbstractInt>() || is_signed_integer_vector();
}

bool Type::is_integer_scalar_or_vector() const {
    return is_unsigned_integer_scalar_or_vector() || is_signed_integer_scalar_or_vector();
}

bool Type::is_abstract_integer_vector() const {
    return Is([](const sem::Vector* v) { return v->type()->Is<type::AbstractInt>(); });
}

bool Type::is_abstract_float_vector() const {
    return Is([](const sem::Vector* v) { return v->type()->Is<type::AbstractFloat>(); });
}

bool Type::is_abstract_integer_scalar_or_vector() const {
    return Is<type::AbstractInt>() || is_abstract_integer_vector();
}

bool Type::is_abstract_float_scalar_or_vector() const {
    return Is<type::AbstractFloat>() || is_abstract_float_vector();
}

bool Type::is_bool_vector() const {
    return Is([](const sem::Vector* v) { return v->type()->Is<type::Bool>(); });
}

bool Type::is_bool_scalar_or_vector() const {
    return Is<type::Bool>() || is_bool_vector();
}

bool Type::is_numeric_vector() const {
    return Is([](const sem::Vector* v) { return v->type()->is_numeric_scalar(); });
}

bool Type::is_scalar_vector() const {
    return Is([](const sem::Vector* v) { return v->type()->is_scalar(); });
}

bool Type::is_numeric_scalar_or_vector() const {
    return is_numeric_scalar() || is_numeric_vector();
}

bool Type::is_handle() const {
    return IsAnyOf<type::Sampler, type::Texture>();
}

bool Type::HoldsAbstract() const {
    return Switch(
        this,  //
        [&](const type::AbstractNumeric*) { return true; },
        [&](const sem::Vector* v) { return v->type()->HoldsAbstract(); },
        [&](const sem::Matrix* m) { return m->type()->HoldsAbstract(); },
        [&](const sem::Array* a) { return a->ElemType()->HoldsAbstract(); },
        [&](const sem::Struct* s) {
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
        [&](const type::AbstractFloat*) {
            return Switch(
                to,                                   //
                [&](const type::F32*) { return 1; },  //
                [&](const type::F16*) { return 2; },  //
                [&](Default) { return kNoConversion; });
        },
        [&](const type::AbstractInt*) {
            return Switch(
                to,                                             //
                [&](const type::I32*) { return 3; },            //
                [&](const type::U32*) { return 4; },            //
                [&](const type::AbstractFloat*) { return 5; },  //
                [&](const type::F32*) { return 6; },            //
                [&](const type::F16*) { return 7; },            //
                [&](Default) { return kNoConversion; });
        },
        [&](const sem::Vector* from_vec) {
            if (auto* to_vec = to->As<sem::Vector>()) {
                if (from_vec->Width() == to_vec->Width()) {
                    return ConversionRank(from_vec->type(), to_vec->type());
                }
            }
            return kNoConversion;
        },
        [&](const sem::Matrix* from_mat) {
            if (auto* to_mat = to->As<sem::Matrix>()) {
                if (from_mat->columns() == to_mat->columns() &&
                    from_mat->rows() == to_mat->rows()) {
                    return ConversionRank(from_mat->type(), to_mat->type());
                }
            }
            return kNoConversion;
        },
        [&](const sem::Array* from_arr) {
            if (auto* to_arr = to->As<sem::Array>()) {
                if (from_arr->Count() == to_arr->Count()) {
                    return ConversionRank(from_arr->ElemType(), to_arr->ElemType());
                }
            }
            return kNoConversion;
        },
        [&](const sem::Struct* from_str) {
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

const Type* Type::ElementOf(const Type* ty, uint32_t* count /* = nullptr */) {
    if (ty->is_scalar()) {
        if (count) {
            *count = 1;
        }
        return ty;
    }
    return Switch(
        ty,  //
        [&](const sem::Vector* v) {
            if (count) {
                *count = v->Width();
            }
            return v->type();
        },
        [&](const sem::Matrix* m) {
            if (count) {
                *count = m->columns();
            }
            return m->ColumnType();
        },
        [&](const sem::Array* a) {
            if (count) {
                if (auto* const_count = a->Count()->As<type::ConstantArrayCount>()) {
                    *count = const_count->value;
                }
            }
            return a->ElemType();
        },
        [&](Default) {
            if (count) {
                *count = 1;
            }
            return ty;
        });
}

const Type* Type::DeepestElementOf(const Type* ty, uint32_t* count /* = nullptr */) {
    auto el_ty = ElementOf(ty, count);
    while (el_ty && ty != el_ty) {
        ty = el_ty;

        uint32_t n = 0;
        el_ty = ElementOf(ty, &n);
        if (count) {
            *count *= n;
        }
    }
    return el_ty;
}

const type::Type* Type::Common(utils::VectorRef<const Type*> types) {
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
        if (type::Type::ConversionRank(ty, common) != type::Type::kNoConversion) {
            continue;  // ty can be converted to common.
        }
        if (type::Type::ConversionRank(common, ty) != type::Type::kNoConversion) {
            common = ty;  // common can be converted to ty.
            continue;
        }
        return nullptr;  // Conversion is not valid.
    }
    return common;
}

}  // namespace tint::type
