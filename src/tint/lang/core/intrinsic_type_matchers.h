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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_TYPE_MATCHERS_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_TYPE_MATCHERS_H_

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

namespace tint::core {

inline bool match_bool(intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, core::type::Bool>();
}

inline const core::type::AbstractFloat* build_fa(intrinsic::MatchState& state) {
    return state.types.AFloat();
}

inline bool match_fa(intrinsic::MatchState& state, const core::type::Type* ty) {
    return (state.earliest_eval_stage <= EvaluationStage::kConstant) &&
           ty->IsAnyOf<intrinsic::Any, core::type::AbstractNumeric>();
}

inline const core::type::AbstractInt* build_ia(intrinsic::MatchState& state) {
    return state.types.AInt();
}

inline bool match_ia(intrinsic::MatchState& state, const core::type::Type* ty) {
    return (state.earliest_eval_stage <= EvaluationStage::kConstant) &&
           ty->IsAnyOf<intrinsic::Any, core::type::AbstractInt>();
}

inline const core::type::Bool* build_bool(intrinsic::MatchState& state) {
    return state.types.bool_();
}

inline const core::type::F16* build_f16(intrinsic::MatchState& state) {
    return state.types.f16();
}

inline bool match_f16(intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, core::type::F16, core::type::AbstractNumeric>();
}

inline const core::type::F32* build_f32(intrinsic::MatchState& state) {
    return state.types.f32();
}

inline bool match_f32(intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, core::type::F32, core::type::AbstractNumeric>();
}

inline const core::type::I32* build_i32(intrinsic::MatchState& state) {
    return state.types.i32();
}

inline bool match_i32(intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, core::type::I32, core::type::AbstractInt>();
}

inline const core::type::U32* build_u32(intrinsic::MatchState& state) {
    return state.types.u32();
}

inline bool match_u32(intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, core::type::U32, core::type::AbstractInt>();
}

inline bool match_vec(intrinsic::MatchState&,
                      const core::type::Type* ty,
                      intrinsic::Number& N,
                      const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        N = intrinsic::Number::any;
        T = ty;
        return true;
    }

    if (auto* v = ty->As<core::type::Vector>()) {
        N = v->Width();
        T = v->type();
        return true;
    }
    return false;
}

template <uint32_t N>
inline bool match_vec(intrinsic::MatchState&,
                      const core::type::Type* ty,
                      const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* v = ty->As<core::type::Vector>()) {
        if (v->Width() == N) {
            T = v->type();
            return true;
        }
    }
    return false;
}

inline const core::type::Vector* build_vec(intrinsic::MatchState& state,
                                           intrinsic::Number N,
                                           const core::type::Type* el) {
    return state.types.vec(el, N.Value());
}

template <uint32_t N>
inline const core::type::Vector* build_vec(intrinsic::MatchState& state,
                                           const core::type::Type* el) {
    return state.types.vec(el, N);
}

constexpr auto match_vec2 = match_vec<2>;
constexpr auto match_vec3 = match_vec<3>;
constexpr auto match_vec4 = match_vec<4>;

constexpr auto build_vec2 = build_vec<2>;
constexpr auto build_vec3 = build_vec<3>;
constexpr auto build_vec4 = build_vec<4>;

inline bool match_packedVec3(intrinsic::MatchState&,
                             const core::type::Type* ty,
                             const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* v = ty->As<core::type::Vector>()) {
        if (v->Packed()) {
            T = v->type();
            return true;
        }
    }
    return false;
}

inline const core::type::Vector* build_packedVec3(intrinsic::MatchState& state,
                                                  const core::type::Type* el) {
    return state.types.Get<core::type::Vector>(el, 3u, /* packed */ true);
}

inline bool match_mat(intrinsic::MatchState&,
                      const core::type::Type* ty,
                      intrinsic::Number& M,
                      intrinsic::Number& N,
                      const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        M = intrinsic::Number::any;
        N = intrinsic::Number::any;
        T = ty;
        return true;
    }
    if (auto* m = ty->As<core::type::Matrix>()) {
        M = m->columns();
        N = m->ColumnType()->Width();
        T = m->type();
        return true;
    }
    return false;
}

template <uint32_t C, uint32_t R>
inline bool match_mat(intrinsic::MatchState&,
                      const core::type::Type* ty,
                      const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* m = ty->As<core::type::Matrix>()) {
        if (m->columns() == C && m->rows() == R) {
            T = m->type();
            return true;
        }
    }
    return false;
}

inline const core::type::Matrix* build_mat(intrinsic::MatchState& state,
                                           intrinsic::Number C,
                                           intrinsic::Number R,
                                           const core::type::Type* T) {
    auto* column_type = state.types.vec(T, R.Value());
    return state.types.mat(column_type, C.Value());
}

template <uint32_t C, uint32_t R>
inline const core::type::Matrix* build_mat(intrinsic::MatchState& state,
                                           const core::type::Type* T) {
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

inline bool match_array(intrinsic::MatchState&,
                        const core::type::Type* ty,
                        const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<core::type::Array>()) {
        if (a->Count()->Is<core::type::RuntimeArrayCount>()) {
            T = a->ElemType();
            return true;
        }
    }
    return false;
}

inline const core::type::Array* build_array(intrinsic::MatchState& state,
                                            const core::type::Type* el) {
    return state.types.Get<core::type::Array>(
        el,
        /* count */ state.types.Get<core::type::RuntimeArrayCount>(),
        /* align */ 0u,
        /* size */ 0u,
        /* stride */ 0u,
        /* stride_implicit */ 0u);
}

inline bool match_ptr(intrinsic::MatchState&,
                      const core::type::Type* ty,
                      intrinsic::Number& S,
                      const core::type::Type*& T,
                      intrinsic::Number& A) {
    if (ty->Is<intrinsic::Any>()) {
        S = intrinsic::Number::any;
        T = ty;
        A = intrinsic::Number::any;
        return true;
    }

    if (auto* p = ty->As<core::type::Pointer>()) {
        S = intrinsic::Number(static_cast<uint32_t>(p->AddressSpace()));
        T = p->StoreType();
        A = intrinsic::Number(static_cast<uint32_t>(p->Access()));
        return true;
    }
    return false;
}

inline const core::type::Pointer* build_ptr(intrinsic::MatchState& state,
                                            intrinsic::Number S,
                                            const core::type::Type* T,
                                            intrinsic::Number& A) {
    return state.types.ptr(static_cast<core::AddressSpace>(S.Value()), T,
                           static_cast<core::Access>(A.Value()));
}

inline bool match_atomic(intrinsic::MatchState&,
                         const core::type::Type* ty,
                         const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<core::type::Atomic>()) {
        T = a->Type();
        return true;
    }
    return false;
}

inline const core::type::Atomic* build_atomic(intrinsic::MatchState& state,
                                              const core::type::Type* T) {
    return state.types.atomic(T);
}

inline bool match_sampler(intrinsic::MatchState&, const core::type::Type* ty) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is([](const core::type::Sampler* s) {
        return s->kind() == core::type::SamplerKind::kSampler;
    });
}

inline const core::type::Sampler* build_sampler(intrinsic::MatchState& state) {
    return state.types.sampler();
}

inline bool match_sampler_comparison(intrinsic::MatchState&, const core::type::Type* ty) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is([](const core::type::Sampler* s) {
        return s->kind() == core::type::SamplerKind::kComparisonSampler;
    });
}

inline const core::type::Sampler* build_sampler_comparison(intrinsic::MatchState& state) {
    return state.types.comparison_sampler();
}

inline bool match_texture(intrinsic::MatchState&,
                          const core::type::Type* ty,
                          core::type::TextureDimension dim,
                          const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<core::type::SampledTexture>()) {
        if (v->dim() == dim) {
            T = v->type();
            return true;
        }
    }
    return false;
}

#define JOIN(a, b) a##b

#define DECLARE_SAMPLED_TEXTURE(suffix, dim)                                                     \
    inline bool JOIN(match_texture_, suffix)(                                                    \
        intrinsic::MatchState & state, const core::type::Type* ty, const core::type::Type*& T) { \
        return match_texture(state, ty, dim, T);                                                 \
    }                                                                                            \
    inline const core::type::SampledTexture* JOIN(build_texture_, suffix)(                       \
        intrinsic::MatchState & state, const core::type::Type* T) {                              \
        return state.types.Get<core::type::SampledTexture>(dim, T);                              \
    }

DECLARE_SAMPLED_TEXTURE(1d, core::type::TextureDimension::k1d)
DECLARE_SAMPLED_TEXTURE(2d, core::type::TextureDimension::k2d)
DECLARE_SAMPLED_TEXTURE(2d_array, core::type::TextureDimension::k2dArray)
DECLARE_SAMPLED_TEXTURE(3d, core::type::TextureDimension::k3d)
DECLARE_SAMPLED_TEXTURE(cube, core::type::TextureDimension::kCube)
DECLARE_SAMPLED_TEXTURE(cube_array, core::type::TextureDimension::kCubeArray)
#undef DECLARE_SAMPLED_TEXTURE

inline bool match_texture_multisampled(intrinsic::MatchState&,
                                       const core::type::Type* ty,
                                       core::type::TextureDimension dim,
                                       const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<core::type::MultisampledTexture>()) {
        if (v->dim() == dim) {
            T = v->type();
            return true;
        }
    }
    return false;
}

#define DECLARE_MULTISAMPLED_TEXTURE(suffix, dim)                                                \
    inline bool JOIN(match_texture_multisampled_, suffix)(                                       \
        intrinsic::MatchState & state, const core::type::Type* ty, const core::type::Type*& T) { \
        return match_texture_multisampled(state, ty, dim, T);                                    \
    }                                                                                            \
    inline const core::type::MultisampledTexture* JOIN(build_texture_multisampled_, suffix)(     \
        intrinsic::MatchState & state, const core::type::Type* T) {                              \
        return state.types.Get<core::type::MultisampledTexture>(dim, T);                         \
    }

DECLARE_MULTISAMPLED_TEXTURE(2d, core::type::TextureDimension::k2d)
#undef DECLARE_MULTISAMPLED_TEXTURE

inline bool match_texture_depth(intrinsic::MatchState&,
                                const core::type::Type* ty,
                                core::type::TextureDimension dim) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is([&](const core::type::DepthTexture* t) { return t->dim() == dim; });
}

#define DECLARE_DEPTH_TEXTURE(suffix, dim)                                               \
    inline bool JOIN(match_texture_depth_, suffix)(intrinsic::MatchState & state,        \
                                                   const core::type::Type* ty) {         \
        return match_texture_depth(state, ty, dim);                                      \
    }                                                                                    \
    inline const core::type::DepthTexture* JOIN(build_texture_depth_,                    \
                                                suffix)(intrinsic::MatchState & state) { \
        return state.types.Get<core::type::DepthTexture>(dim);                           \
    }

DECLARE_DEPTH_TEXTURE(2d, core::type::TextureDimension::k2d)
DECLARE_DEPTH_TEXTURE(2d_array, core::type::TextureDimension::k2dArray)
DECLARE_DEPTH_TEXTURE(cube, core::type::TextureDimension::kCube)
DECLARE_DEPTH_TEXTURE(cube_array, core::type::TextureDimension::kCubeArray)
#undef DECLARE_DEPTH_TEXTURE

inline bool match_texture_depth_multisampled_2d(intrinsic::MatchState&,
                                                const core::type::Type* ty) {
    if (ty->Is<intrinsic::Any>()) {
        return true;
    }
    return ty->Is([&](const core::type::DepthMultisampledTexture* t) {
        return t->dim() == core::type::TextureDimension::k2d;
    });
}

inline core::type::DepthMultisampledTexture* build_texture_depth_multisampled_2d(
    intrinsic::MatchState& state) {
    return state.types.Get<core::type::DepthMultisampledTexture>(core::type::TextureDimension::k2d);
}

inline bool match_texture_storage(intrinsic::MatchState&,
                                  const core::type::Type* ty,
                                  core::type::TextureDimension dim,
                                  intrinsic::Number& F,
                                  intrinsic::Number& A) {
    if (ty->Is<intrinsic::Any>()) {
        F = intrinsic::Number::any;
        A = intrinsic::Number::any;
        return true;
    }
    if (auto* v = ty->As<core::type::StorageTexture>()) {
        if (v->dim() == dim) {
            F = intrinsic::Number(static_cast<uint32_t>(v->texel_format()));
            A = intrinsic::Number(static_cast<uint32_t>(v->access()));
            return true;
        }
    }
    return false;
}

#define DECLARE_STORAGE_TEXTURE(suffix, dim)                                                       \
    inline bool JOIN(match_texture_storage_, suffix)(intrinsic::MatchState & state,                \
                                                     const core::type::Type* ty,                   \
                                                     intrinsic::Number& F, intrinsic::Number& A) { \
        return match_texture_storage(state, ty, dim, F, A);                                        \
    }                                                                                              \
    inline const core::type::StorageTexture* JOIN(build_texture_storage_, suffix)(                 \
        intrinsic::MatchState & state, intrinsic::Number F, intrinsic::Number A) {                 \
        auto format = static_cast<TexelFormat>(F.Value());                                         \
        auto access = static_cast<Access>(A.Value());                                              \
        auto* T = core::type::StorageTexture::SubtypeFor(format, state.types);                     \
        return state.types.Get<core::type::StorageTexture>(dim, format, access, T);                \
    }

DECLARE_STORAGE_TEXTURE(1d, core::type::TextureDimension::k1d)
DECLARE_STORAGE_TEXTURE(2d, core::type::TextureDimension::k2d)
DECLARE_STORAGE_TEXTURE(2d_array, core::type::TextureDimension::k2dArray)
DECLARE_STORAGE_TEXTURE(3d, core::type::TextureDimension::k3d)
#undef DECLARE_STORAGE_TEXTURE

inline bool match_texture_external(intrinsic::MatchState&, const core::type::Type* ty) {
    return ty->IsAnyOf<intrinsic::Any, core::type::ExternalTexture>();
}

inline const core::type::ExternalTexture* build_texture_external(intrinsic::MatchState& state) {
    return state.types.Get<core::type::ExternalTexture>();
}

// Builtin types starting with a _ prefix cannot be declared in WGSL, so they
// can only be used as return types. Because of this, they must only match Any,
// which is used as the return type matcher.
inline bool match_modf_result(intrinsic::MatchState&,
                              const core::type::Type* ty,
                              const core::type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    T = ty;
    return true;
}
inline bool match_modf_result_vec(intrinsic::MatchState&,
                                  const core::type::Type* ty,
                                  intrinsic::Number& N,
                                  const core::type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    N = intrinsic::Number::any;
    T = ty;
    return true;
}
inline bool match_frexp_result(intrinsic::MatchState&,
                               const core::type::Type* ty,
                               const core::type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    T = ty;
    return true;
}
inline bool match_frexp_result_vec(intrinsic::MatchState&,
                                   const core::type::Type* ty,
                                   intrinsic::Number& N,
                                   const core::type::Type*& T) {
    if (!ty->Is<intrinsic::Any>()) {
        return false;
    }
    N = intrinsic::Number::any;
    T = ty;
    return true;
}

inline bool match_atomic_compare_exchange_result(intrinsic::MatchState&,
                                                 const core::type::Type* ty,
                                                 const core::type::Type*& T) {
    if (ty->Is<intrinsic::Any>()) {
        T = ty;
        return true;
    }
    return false;
}

inline const core::type::Struct* build_modf_result(intrinsic::MatchState& state,
                                                   const core::type::Type* el) {
    return core::type::CreateModfResult(state.types, state.symbols, el);
}

inline const core::type::Struct* build_modf_result_vec(intrinsic::MatchState& state,
                                                       intrinsic::Number& n,
                                                       const core::type::Type* el) {
    auto* vec = state.types.vec(el, n.Value());
    return core::type::CreateModfResult(state.types, state.symbols, vec);
}

inline const core::type::Struct* build_frexp_result(intrinsic::MatchState& state,
                                                    const core::type::Type* el) {
    return core::type::CreateFrexpResult(state.types, state.symbols, el);
}

inline const core::type::Struct* build_frexp_result_vec(intrinsic::MatchState& state,
                                                        intrinsic::Number& n,
                                                        const core::type::Type* el) {
    auto* vec = state.types.vec(el, n.Value());
    return core::type::CreateFrexpResult(state.types, state.symbols, vec);
}

inline const core::type::Struct* build_atomic_compare_exchange_result(intrinsic::MatchState& state,
                                                                      const core::type::Type* ty) {
    return core::type::CreateAtomicCompareExchangeResult(state.types, state.symbols, ty);
}

}  // namespace tint::core

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_TYPE_MATCHERS_H_
