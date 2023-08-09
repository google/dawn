// Copyright 2023 The Tint Authors.
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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_CORE_TYPE_MATCHERS_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_CORE_TYPE_MATCHERS_H_

#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/type/abstract_float.h"
#include "src/tint/lang/core/type/abstract_int.h"
#include "src/tint/lang/core/type/abstract_numeric.h"
#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/atomic.h"
#include "src/tint/lang/core/type/bool.h"
#include "src/tint/lang/core/type/builtin_structs.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/external_texture.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"

namespace tint::core::intrinsic {

inline bool match_bool(TableData::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<TableData::Any, type::Bool>();
}

inline const type::AbstractFloat* build_fa(TableData::MatchState& state) {
    return state.types.AFloat();
}

inline bool match_fa(TableData::MatchState& state, const type::Type* ty) {
    return (state.earliest_eval_stage <= EvaluationStage::kConstant) &&
           ty->IsAnyOf<TableData::Any, type::AbstractNumeric>();
}

inline const type::AbstractInt* build_ia(TableData::MatchState& state) {
    return state.types.AInt();
}

inline bool match_ia(TableData::MatchState& state, const type::Type* ty) {
    return (state.earliest_eval_stage <= EvaluationStage::kConstant) &&
           ty->IsAnyOf<TableData::Any, type::AbstractInt>();
}

inline const type::Bool* build_bool(TableData::MatchState& state) {
    return state.types.bool_();
}

inline const type::F16* build_f16(TableData::MatchState& state) {
    return state.types.f16();
}

inline bool match_f16(TableData::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<TableData::Any, type::F16, type::AbstractNumeric>();
}

inline const type::F32* build_f32(TableData::MatchState& state) {
    return state.types.f32();
}

inline bool match_f32(TableData::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<TableData::Any, type::F32, type::AbstractNumeric>();
}

inline const type::I32* build_i32(TableData::MatchState& state) {
    return state.types.i32();
}

inline bool match_i32(TableData::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<TableData::Any, type::I32, type::AbstractInt>();
}

inline const type::U32* build_u32(TableData::MatchState& state) {
    return state.types.u32();
}

inline bool match_u32(TableData::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<TableData::Any, type::U32, type::AbstractInt>();
}

inline bool match_vec(TableData::MatchState&,
                      const type::Type* ty,
                      TableData::Number& N,
                      const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        N = TableData::Number::any;
        T = ty;
        return true;
    }

    if (auto* v = ty->As<type::Vector>()) {
        N = v->Width();
        T = v->type();
        return true;
    }
    return false;
}

template <uint32_t N>
inline bool match_vec(TableData::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }

    if (auto* v = ty->As<type::Vector>()) {
        if (v->Width() == N) {
            T = v->type();
            return true;
        }
    }
    return false;
}

inline const type::Vector* build_vec(TableData::MatchState& state,
                                     TableData::Number N,
                                     const type::Type* el) {
    return state.types.vec(el, N.Value());
}

template <uint32_t N>
inline const type::Vector* build_vec(TableData::MatchState& state, const type::Type* el) {
    return state.types.vec(el, N);
}

constexpr auto match_vec2 = match_vec<2>;
constexpr auto match_vec3 = match_vec<3>;
constexpr auto match_vec4 = match_vec<4>;

constexpr auto build_vec2 = build_vec<2>;
constexpr auto build_vec3 = build_vec<3>;
constexpr auto build_vec4 = build_vec<4>;

inline bool match_packedVec3(TableData::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }

    if (auto* v = ty->As<type::Vector>()) {
        if (v->Packed()) {
            T = v->type();
            return true;
        }
    }
    return false;
}

inline const type::Vector* build_packedVec3(TableData::MatchState& state, const type::Type* el) {
    return state.types.Get<type::Vector>(el, 3u, /* packed */ true);
}

inline bool match_mat(TableData::MatchState&,
                      const type::Type* ty,
                      TableData::Number& M,
                      TableData::Number& N,
                      const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        M = TableData::Number::any;
        N = TableData::Number::any;
        T = ty;
        return true;
    }
    if (auto* m = ty->As<type::Matrix>()) {
        M = m->columns();
        N = m->ColumnType()->Width();
        T = m->type();
        return true;
    }
    return false;
}

template <uint32_t C, uint32_t R>
inline bool match_mat(TableData::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }
    if (auto* m = ty->As<type::Matrix>()) {
        if (m->columns() == C && m->rows() == R) {
            T = m->type();
            return true;
        }
    }
    return false;
}

inline const type::Matrix* build_mat(TableData::MatchState& state,
                                     TableData::Number C,
                                     TableData::Number R,
                                     const type::Type* T) {
    auto* column_type = state.types.vec(T, R.Value());
    return state.types.mat(column_type, C.Value());
}

template <uint32_t C, uint32_t R>
inline const type::Matrix* build_mat(TableData::MatchState& state, const type::Type* T) {
    auto* column_type = state.types.vec(T, R);
    return state.types.mat(column_type, C);
}

constexpr auto build_mat2x2 = build_mat<2, 2>;
constexpr auto build_mat2x3 = build_mat<2, 3>;
constexpr auto build_mat2x4 = build_mat<2, 4>;
constexpr auto build_mat3x2 = build_mat<3, 2>;
constexpr auto build_mat3x3 = build_mat<3, 3>;
constexpr auto build_mat3x4 = build_mat<3, 4>;
constexpr auto build_mat4x2 = build_mat<4, 2>;
constexpr auto build_mat4x3 = build_mat<4, 3>;
constexpr auto build_mat4x4 = build_mat<4, 4>;

constexpr auto match_mat2x2 = match_mat<2, 2>;
constexpr auto match_mat2x3 = match_mat<2, 3>;
constexpr auto match_mat2x4 = match_mat<2, 4>;
constexpr auto match_mat3x2 = match_mat<3, 2>;
constexpr auto match_mat3x3 = match_mat<3, 3>;
constexpr auto match_mat3x4 = match_mat<3, 4>;
constexpr auto match_mat4x2 = match_mat<4, 2>;
constexpr auto match_mat4x3 = match_mat<4, 3>;
constexpr auto match_mat4x4 = match_mat<4, 4>;

inline bool match_array(TableData::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<type::Array>()) {
        if (a->Count()->Is<type::RuntimeArrayCount>()) {
            T = a->ElemType();
            return true;
        }
    }
    return false;
}

inline const type::Array* build_array(TableData::MatchState& state, const type::Type* el) {
    return state.types.Get<type::Array>(el,
                                        /* count */ state.types.Get<type::RuntimeArrayCount>(),
                                        /* align */ 0u,
                                        /* size */ 0u,
                                        /* stride */ 0u,
                                        /* stride_implicit */ 0u);
}

inline bool match_ptr(TableData::MatchState&,
                      const type::Type* ty,
                      TableData::Number& S,
                      const type::Type*& T,
                      TableData::Number& A) {
    if (ty->Is<TableData::Any>()) {
        S = TableData::Number::any;
        T = ty;
        A = TableData::Number::any;
        return true;
    }

    if (auto* p = ty->As<type::Pointer>()) {
        S = TableData::Number(static_cast<uint32_t>(p->AddressSpace()));
        T = p->StoreType();
        A = TableData::Number(static_cast<uint32_t>(p->Access()));
        return true;
    }
    return false;
}

inline const type::Pointer* build_ptr(TableData::MatchState& state,
                                      TableData::Number S,
                                      const type::Type* T,
                                      TableData::Number& A) {
    return state.types.ptr(static_cast<core::AddressSpace>(S.Value()), T,
                           static_cast<core::Access>(A.Value()));
}

inline bool match_atomic(TableData::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<type::Atomic>()) {
        T = a->Type();
        return true;
    }
    return false;
}

inline const type::Atomic* build_atomic(TableData::MatchState& state, const type::Type* T) {
    return state.types.atomic(T);
}

inline bool match_sampler(TableData::MatchState&, const type::Type* ty) {
    if (ty->Is<TableData::Any>()) {
        return true;
    }
    return ty->Is([](const type::Sampler* s) { return s->kind() == type::SamplerKind::kSampler; });
}

inline const type::Sampler* build_sampler(TableData::MatchState& state) {
    return state.types.sampler();
}

inline bool match_sampler_comparison(TableData::MatchState&, const type::Type* ty) {
    if (ty->Is<TableData::Any>()) {
        return true;
    }
    return ty->Is(
        [](const type::Sampler* s) { return s->kind() == type::SamplerKind::kComparisonSampler; });
}

inline const type::Sampler* build_sampler_comparison(TableData::MatchState& state) {
    return state.types.comparison_sampler();
}

inline bool match_texture(TableData::MatchState&,
                          const type::Type* ty,
                          type::TextureDimension dim,
                          const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<type::SampledTexture>()) {
        if (v->dim() == dim) {
            T = v->type();
            return true;
        }
    }
    return false;
}

#define JOIN(a, b) a##b

#define DECLARE_SAMPLED_TEXTURE(suffix, dim)                                                       \
    inline bool JOIN(match_texture_, suffix)(TableData::MatchState & state, const type::Type* ty,  \
                                             const type::Type*& T) {                               \
        return match_texture(state, ty, dim, T);                                                   \
    }                                                                                              \
    inline const type::SampledTexture* JOIN(build_texture_, suffix)(TableData::MatchState & state, \
                                                                    const type::Type* T) {         \
        return state.types.Get<type::SampledTexture>(dim, T);                                      \
    }

DECLARE_SAMPLED_TEXTURE(1d, type::TextureDimension::k1d)
DECLARE_SAMPLED_TEXTURE(2d, type::TextureDimension::k2d)
DECLARE_SAMPLED_TEXTURE(2d_array, type::TextureDimension::k2dArray)
DECLARE_SAMPLED_TEXTURE(3d, type::TextureDimension::k3d)
DECLARE_SAMPLED_TEXTURE(cube, type::TextureDimension::kCube)
DECLARE_SAMPLED_TEXTURE(cube_array, type::TextureDimension::kCubeArray)
#undef DECLARE_SAMPLED_TEXTURE

inline bool match_texture_multisampled(TableData::MatchState&,
                                       const type::Type* ty,
                                       type::TextureDimension dim,
                                       const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<type::MultisampledTexture>()) {
        if (v->dim() == dim) {
            T = v->type();
            return true;
        }
    }
    return false;
}

#define DECLARE_MULTISAMPLED_TEXTURE(suffix, dim)                                      \
    inline bool JOIN(match_texture_multisampled_, suffix)(                             \
        TableData::MatchState & state, const type::Type* ty, const type::Type*& T) {   \
        return match_texture_multisampled(state, ty, dim, T);                          \
    }                                                                                  \
    inline const type::MultisampledTexture* JOIN(build_texture_multisampled_, suffix)( \
        TableData::MatchState & state, const type::Type* T) {                          \
        return state.types.Get<type::MultisampledTexture>(dim, T);                     \
    }

DECLARE_MULTISAMPLED_TEXTURE(2d, type::TextureDimension::k2d)
#undef DECLARE_MULTISAMPLED_TEXTURE

inline bool match_texture_depth(TableData::MatchState&,
                                const type::Type* ty,
                                type::TextureDimension dim) {
    if (ty->Is<TableData::Any>()) {
        return true;
    }
    return ty->Is([&](const type::DepthTexture* t) { return t->dim() == dim; });
}

#define DECLARE_DEPTH_TEXTURE(suffix, dim)                                         \
    inline bool JOIN(match_texture_depth_, suffix)(TableData::MatchState & state,  \
                                                   const type::Type* ty) {         \
        return match_texture_depth(state, ty, dim);                                \
    }                                                                              \
    inline const type::DepthTexture* JOIN(build_texture_depth_,                    \
                                          suffix)(TableData::MatchState & state) { \
        return state.types.Get<type::DepthTexture>(dim);                           \
    }

DECLARE_DEPTH_TEXTURE(2d, type::TextureDimension::k2d)
DECLARE_DEPTH_TEXTURE(2d_array, type::TextureDimension::k2dArray)
DECLARE_DEPTH_TEXTURE(cube, type::TextureDimension::kCube)
DECLARE_DEPTH_TEXTURE(cube_array, type::TextureDimension::kCubeArray)
#undef DECLARE_DEPTH_TEXTURE

inline bool match_texture_depth_multisampled_2d(TableData::MatchState&, const type::Type* ty) {
    if (ty->Is<TableData::Any>()) {
        return true;
    }
    return ty->Is([&](const type::DepthMultisampledTexture* t) {
        return t->dim() == type::TextureDimension::k2d;
    });
}

inline type::DepthMultisampledTexture* build_texture_depth_multisampled_2d(
    TableData::MatchState& state) {
    return state.types.Get<type::DepthMultisampledTexture>(type::TextureDimension::k2d);
}

inline bool match_texture_storage(TableData::MatchState&,
                                  const type::Type* ty,
                                  type::TextureDimension dim,
                                  TableData::Number& F,
                                  TableData::Number& A) {
    if (ty->Is<TableData::Any>()) {
        F = TableData::Number::any;
        A = TableData::Number::any;
        return true;
    }
    if (auto* v = ty->As<type::StorageTexture>()) {
        if (v->dim() == dim) {
            F = TableData::Number(static_cast<uint32_t>(v->texel_format()));
            A = TableData::Number(static_cast<uint32_t>(v->access()));
            return true;
        }
    }
    return false;
}

#define DECLARE_STORAGE_TEXTURE(suffix, dim)                                                     \
    inline bool JOIN(match_texture_storage_, suffix)(TableData::MatchState & state,              \
                                                     const type::Type* ty, TableData::Number& F, \
                                                     TableData::Number& A) {                     \
        return match_texture_storage(state, ty, dim, F, A);                                      \
    }                                                                                            \
    inline const type::StorageTexture* JOIN(build_texture_storage_, suffix)(                     \
        TableData::MatchState & state, TableData::Number F, TableData::Number A) {               \
        auto format = static_cast<TexelFormat>(F.Value());                                       \
        auto access = static_cast<Access>(A.Value());                                            \
        auto* T = type::StorageTexture::SubtypeFor(format, state.types);                         \
        return state.types.Get<type::StorageTexture>(dim, format, access, T);                    \
    }

DECLARE_STORAGE_TEXTURE(1d, type::TextureDimension::k1d)
DECLARE_STORAGE_TEXTURE(2d, type::TextureDimension::k2d)
DECLARE_STORAGE_TEXTURE(2d_array, type::TextureDimension::k2dArray)
DECLARE_STORAGE_TEXTURE(3d, type::TextureDimension::k3d)
#undef DECLARE_STORAGE_TEXTURE

inline bool match_texture_external(TableData::MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<TableData::Any, type::ExternalTexture>();
}

inline const type::ExternalTexture* build_texture_external(TableData::MatchState& state) {
    return state.types.Get<type::ExternalTexture>();
}

// Builtin types starting with a _ prefix cannot be declared in WGSL, so they
// can only be used as return types. Because of this, they must only match Any,
// which is used as the return type matcher.
inline bool match_modf_result(TableData::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (!ty->Is<TableData::Any>()) {
        return false;
    }
    T = ty;
    return true;
}
inline bool match_modf_result_vec(TableData::MatchState&,
                                  const type::Type* ty,
                                  TableData::Number& N,
                                  const type::Type*& T) {
    if (!ty->Is<TableData::Any>()) {
        return false;
    }
    N = TableData::Number::any;
    T = ty;
    return true;
}
inline bool match_frexp_result(TableData::MatchState&, const type::Type* ty, const type::Type*& T) {
    if (!ty->Is<TableData::Any>()) {
        return false;
    }
    T = ty;
    return true;
}
inline bool match_frexp_result_vec(TableData::MatchState&,
                                   const type::Type* ty,
                                   TableData::Number& N,
                                   const type::Type*& T) {
    if (!ty->Is<TableData::Any>()) {
        return false;
    }
    N = TableData::Number::any;
    T = ty;
    return true;
}

inline bool match_atomic_compare_exchange_result(TableData::MatchState&,
                                                 const type::Type* ty,
                                                 const type::Type*& T) {
    if (ty->Is<TableData::Any>()) {
        T = ty;
        return true;
    }
    return false;
}

inline const type::Struct* build_modf_result(TableData::MatchState& state, const type::Type* el) {
    return type::CreateModfResult(state.types, state.symbols, el);
}

inline const type::Struct* build_modf_result_vec(TableData::MatchState& state,
                                                 TableData::Number& n,
                                                 const type::Type* el) {
    auto* vec = state.types.vec(el, n.Value());
    return type::CreateModfResult(state.types, state.symbols, vec);
}

inline const type::Struct* build_frexp_result(TableData::MatchState& state, const type::Type* el) {
    return type::CreateFrexpResult(state.types, state.symbols, el);
}

inline const type::Struct* build_frexp_result_vec(TableData::MatchState& state,
                                                  TableData::Number& n,
                                                  const type::Type* el) {
    auto* vec = state.types.vec(el, n.Value());
    return type::CreateFrexpResult(state.types, state.symbols, vec);
}

inline const type::Struct* build_atomic_compare_exchange_result(TableData::MatchState& state,
                                                                const type::Type* ty) {
    return type::CreateAtomicCompareExchangeResult(state.types, state.symbols, ty);
}

}  // namespace tint::core::intrinsic

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_CORE_TYPE_MATCHERS_H_
