// Copyright 2021 The Tint Authors.
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

#include "src/tint/builtin_table.h"

#include <algorithm>
#include <limits>
#include <unordered_map>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/atomic.h"
#include "src/tint/sem/depth_multisampled_texture.h"
#include "src/tint/sem/depth_texture.h"
#include "src/tint/sem/external_texture.h"
#include "src/tint/sem/multisampled_texture.h"
#include "src/tint/sem/pipeline_stage_set.h"
#include "src/tint/sem/sampled_texture.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/scoped_assignment.h"

namespace tint {
namespace {

// Forward declarations
struct OverloadInfo;
class Matchers;
class NumberMatcher;
class TypeMatcher;

/// A special type that matches all TypeMatchers
class Any final : public Castable<Any, sem::Type> {
  public:
    Any() = default;
    ~Any() override = default;

    // Stub implementations for sem::Type conformance.
    size_t Hash() const override { return 0; }
    bool Equals(const sem::Type&) const override { return false; }
    std::string FriendlyName(const SymbolTable&) const override { return "<any>"; }
};

/// Number is an 32 bit unsigned integer, which can be in one of three states:
/// * Invalid - Number has not been assigned a value
/// * Valid   - a fixed integer value
/// * Any     - matches any other non-invalid number
struct Number {
    static const Number any;
    static const Number invalid;

    /// Constructed as a valid number with the value v
    explicit Number(uint32_t v) : value_(v), state_(kValid) {}

    /// @returns the value of the number
    inline uint32_t Value() const { return value_; }

    /// @returns the true if the number is valid
    inline bool IsValid() const { return state_ == kValid; }

    /// @returns the true if the number is any
    inline bool IsAny() const { return state_ == kAny; }

    /// Assignment operator.
    /// The number becomes valid, with the value n
    inline Number& operator=(uint32_t n) {
        value_ = n;
        state_ = kValid;
        return *this;
    }

  private:
    enum State {
        kInvalid,
        kValid,
        kAny,
    };

    constexpr explicit Number(State state) : state_(state) {}

    uint32_t value_ = 0;
    State state_ = kInvalid;
};

const Number Number::any{Number::kAny};
const Number Number::invalid{Number::kInvalid};

/// ClosedState holds the state of the open / closed numbers and types.
/// Used by the MatchState.
class ClosedState {
  public:
    explicit ClosedState(ProgramBuilder& b) : builder(b) {}

    /// If the type with index `idx` is open, then it is closed with type `ty` and
    /// Type() returns true. If the type is closed, then `Type()` returns true iff
    /// it is equal to `ty`.
    bool Type(uint32_t idx, const sem::Type* ty) {
        auto res = types_.emplace(idx, ty);
        return res.second || res.first->second == ty;
    }

    /// If the number with index `idx` is open, then it is closed with number
    /// `number` and Num() returns true. If the number is closed, then `Num()`
    /// returns true iff it is equal to `ty`.
    bool Num(uint32_t idx, Number number) {
        auto res = numbers_.emplace(idx, number.Value());
        return res.second || res.first->second == number.Value();
    }

    /// Type returns the closed type with index `idx`.
    /// An ICE is raised if the type is not closed.
    const sem::Type* Type(uint32_t idx) const {
        auto it = types_.find(idx);
        if (it == types_.end()) {
            TINT_ICE(Resolver, builder.Diagnostics())
                << "type with index " << idx << " is not closed";
            return nullptr;
        }
        TINT_ASSERT(Resolver, it != types_.end());
        return it->second;
    }

    /// Type returns the number type with index `idx`.
    /// An ICE is raised if the number is not closed.
    Number Num(uint32_t idx) const {
        auto it = numbers_.find(idx);
        if (it == numbers_.end()) {
            TINT_ICE(Resolver, builder.Diagnostics())
                << "number with index " << idx << " is not closed";
            return Number::invalid;
        }
        return Number(it->second);
    }

  private:
    ProgramBuilder& builder;
    std::unordered_map<uint32_t, const sem::Type*> types_;
    std::unordered_map<uint32_t, uint32_t> numbers_;
};

/// Index type used for matcher indices
using MatcherIndex = uint8_t;

/// Index value used for open types / numbers that do not have a constraint
constexpr MatcherIndex kNoMatcher = std::numeric_limits<MatcherIndex>::max();

/// MatchState holds the state used to match an overload.
class MatchState {
  public:
    MatchState(ProgramBuilder& b,
               ClosedState& c,
               const Matchers& m,
               const OverloadInfo& o,
               MatcherIndex const* matcher_indices)
        : builder(b), closed(c), matchers(m), overload(o), matcher_indices_(matcher_indices) {}

    /// The program builder
    ProgramBuilder& builder;
    /// The open / closed types and numbers
    ClosedState& closed;
    /// The type and number matchers
    Matchers const& matchers;
    /// The current overload being evaluated
    OverloadInfo const& overload;

    /// Type uses the next TypeMatcher from the matcher indices to match the type
    /// `ty`. If the type matches, the canonical expected type is returned. If the
    /// type `ty` does not match, then nullptr is returned.
    /// @note: The matcher indices are progressed on calling.
    const sem::Type* Type(const sem::Type* ty);

    /// Num uses the next NumMatcher from the matcher indices to match the number
    /// `num`. If the number matches, the canonical expected number is returned.
    /// If the number `num` does not match, then an invalid number is returned.
    /// @note: The matcher indices are progressed on calling.
    Number Num(Number num);

    /// @returns a string representation of the next TypeMatcher from the matcher
    /// indices.
    /// @note: The matcher indices are progressed on calling.
    std::string TypeName();

    /// @returns a string representation of the next NumberMatcher from the
    /// matcher indices.
    /// @note: The matcher indices are progressed on calling.
    std::string NumName();

  private:
    MatcherIndex const* matcher_indices_ = nullptr;
};

/// A TypeMatcher is the interface used to match an type used as part of an
/// overload's parameter or return type.
class TypeMatcher {
  public:
    /// Destructor
    virtual ~TypeMatcher() = default;

    /// Checks whether the given type matches the matcher rules, and returns the
    /// expected, canonicalized type on success.
    /// Match may close open types and numbers in state.
    /// @param type the type to match
    /// @returns the canonicalized type on match, otherwise nullptr
    virtual const sem::Type* Match(MatchState& state, const sem::Type* type) const = 0;

    /// @return a string representation of the matcher. Used for printing error
    /// messages when no overload is found.
    virtual std::string String(MatchState& state) const = 0;
};

/// A NumberMatcher is the interface used to match a number or enumerator used
/// as part of an overload's parameter or return type.
class NumberMatcher {
  public:
    /// Destructor
    virtual ~NumberMatcher() = default;

    /// Checks whether the given number matches the matcher rules.
    /// Match may close open numbers in state.
    /// @param number the number to match
    /// @returns true if the argument type is as expected.
    virtual Number Match(MatchState& state, Number number) const = 0;

    /// @return a string representation of the matcher. Used for printing error
    /// messages when no overload is found.
    virtual std::string String(MatchState& state) const = 0;
};

/// OpenTypeMatcher is a Matcher for an open type.
/// The OpenTypeMatcher will match against any type (so long as it is consistent
/// across all uses in the overload)
class OpenTypeMatcher : public TypeMatcher {
  public:
    /// Constructor
    explicit OpenTypeMatcher(uint32_t index) : index_(index) {}

    const sem::Type* Match(MatchState& state, const sem::Type* type) const override {
        if (type->Is<Any>()) {
            return state.closed.Type(index_);
        }
        return state.closed.Type(index_, type) ? type : nullptr;
    }

    std::string String(MatchState& state) const override;

  private:
    uint32_t index_;
};

/// OpenNumberMatcher is a Matcher for an open number.
/// The OpenNumberMatcher will match against any number (so long as it is
/// consistent for the overload)
class OpenNumberMatcher : public NumberMatcher {
  public:
    explicit OpenNumberMatcher(uint32_t index) : index_(index) {}

    Number Match(MatchState& state, Number number) const override {
        if (number.IsAny()) {
            return state.closed.Num(index_);
        }
        return state.closed.Num(index_, number) ? number : Number::invalid;
    }

    std::string String(MatchState& state) const override;

  private:
    uint32_t index_;
};

////////////////////////////////////////////////////////////////////////////////
// Binding functions for use in the generated builtin_table.inl
// TODO(bclayton): See if we can move more of this hand-rolled code to the
// template
////////////////////////////////////////////////////////////////////////////////
using TexelFormat = ast::TexelFormat;
using Access = ast::Access;
using StorageClass = ast::StorageClass;
using ParameterUsage = sem::ParameterUsage;
using PipelineStageSet = sem::PipelineStageSet;
using PipelineStage = ast::PipelineStage;

bool match_bool(const sem::Type* ty) {
    return ty->IsAnyOf<Any, sem::Bool>();
}

const sem::Bool* build_bool(MatchState& state) {
    return state.builder.create<sem::Bool>();
}

bool match_f32(const sem::Type* ty) {
    return ty->IsAnyOf<Any, sem::F32>();
}

const sem::I32* build_i32(MatchState& state) {
    return state.builder.create<sem::I32>();
}

bool match_i32(const sem::Type* ty) {
    return ty->IsAnyOf<Any, sem::I32>();
}

const sem::U32* build_u32(MatchState& state) {
    return state.builder.create<sem::U32>();
}

bool match_u32(const sem::Type* ty) {
    return ty->IsAnyOf<Any, sem::U32>();
}

const sem::F32* build_f32(MatchState& state) {
    return state.builder.create<sem::F32>();
}

bool match_vec(const sem::Type* ty, Number& N, const sem::Type*& T) {
    if (ty->Is<Any>()) {
        N = Number::any;
        T = ty;
        return true;
    }

    if (auto* v = ty->As<sem::Vector>()) {
        N = v->Width();
        T = v->type();
        return true;
    }
    return false;
}

const sem::Vector* build_vec(MatchState& state, Number N, const sem::Type* el) {
    return state.builder.create<sem::Vector>(el, N.Value());
}

template <int N>
bool match_vec(const sem::Type* ty, const sem::Type*& T) {
    if (ty->Is<Any>()) {
        T = ty;
        return true;
    }

    if (auto* v = ty->As<sem::Vector>()) {
        if (v->Width() == N) {
            T = v->type();
            return true;
        }
    }
    return false;
}

bool match_vec2(const sem::Type* ty, const sem::Type*& T) {
    return match_vec<2>(ty, T);
}

const sem::Vector* build_vec2(MatchState& state, const sem::Type* T) {
    return build_vec(state, Number(2), T);
}

bool match_vec3(const sem::Type* ty, const sem::Type*& T) {
    return match_vec<3>(ty, T);
}

const sem::Vector* build_vec3(MatchState& state, const sem::Type* T) {
    return build_vec(state, Number(3), T);
}

bool match_vec4(const sem::Type* ty, const sem::Type*& T) {
    return match_vec<4>(ty, T);
}

const sem::Vector* build_vec4(MatchState& state, const sem::Type* T) {
    return build_vec(state, Number(4), T);
}

bool match_mat(const sem::Type* ty, Number& M, Number& N, const sem::Type*& T) {
    if (ty->Is<Any>()) {
        M = Number::any;
        N = Number::any;
        T = ty;
        return true;
    }
    if (auto* m = ty->As<sem::Matrix>()) {
        M = m->columns();
        N = m->ColumnType()->Width();
        T = m->type();
        return true;
    }
    return false;
}

const sem::Matrix* build_mat(MatchState& state, Number N, Number M, const sem::Type* T) {
    auto* column_type = state.builder.create<sem::Vector>(T, M.Value());
    return state.builder.create<sem::Matrix>(column_type, N.Value());
}

bool match_array(const sem::Type* ty, const sem::Type*& T) {
    if (ty->Is<Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<sem::Array>()) {
        if (a->Count() == 0) {
            T = a->ElemType();
            return true;
        }
    }
    return false;
}

const sem::Array* build_array(MatchState& state, const sem::Type* el) {
    return state.builder.create<sem::Array>(el,
                                            /* count */ 0u,
                                            /* align */ 0u,
                                            /* size */ 0u,
                                            /* stride */ 0u,
                                            /* stride_implicit */ 0u);
}

bool match_ptr(const sem::Type* ty, Number& S, const sem::Type*& T, Number& A) {
    if (ty->Is<Any>()) {
        S = Number::any;
        T = ty;
        A = Number::any;
        return true;
    }

    if (auto* p = ty->As<sem::Pointer>()) {
        S = Number(static_cast<uint32_t>(p->StorageClass()));
        T = p->StoreType();
        A = Number(static_cast<uint32_t>(p->Access()));
        return true;
    }
    return false;
}

const sem::Pointer* build_ptr(MatchState& state, Number S, const sem::Type* T, Number& A) {
    return state.builder.create<sem::Pointer>(T, static_cast<ast::StorageClass>(S.Value()),
                                              static_cast<ast::Access>(A.Value()));
}

bool match_atomic(const sem::Type* ty, const sem::Type*& T) {
    if (ty->Is<Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<sem::Atomic>()) {
        T = a->Type();
        return true;
    }
    return false;
}

const sem::Atomic* build_atomic(MatchState& state, const sem::Type* T) {
    return state.builder.create<sem::Atomic>(T);
}

bool match_sampler(const sem::Type* ty) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is([](const sem::Sampler* s) { return s->kind() == ast::SamplerKind::kSampler; });
}

const sem::Sampler* build_sampler(MatchState& state) {
    return state.builder.create<sem::Sampler>(ast::SamplerKind::kSampler);
}

bool match_sampler_comparison(const sem::Type* ty) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is(
        [](const sem::Sampler* s) { return s->kind() == ast::SamplerKind::kComparisonSampler; });
}

const sem::Sampler* build_sampler_comparison(MatchState& state) {
    return state.builder.create<sem::Sampler>(ast::SamplerKind::kComparisonSampler);
}

bool match_texture(const sem::Type* ty, ast::TextureDimension dim, const sem::Type*& T) {
    if (ty->Is<Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<sem::SampledTexture>()) {
        if (v->dim() == dim) {
            T = v->type();
            return true;
        }
    }
    return false;
}

#define JOIN(a, b) a##b

#define DECLARE_SAMPLED_TEXTURE(suffix, dim)                                      \
    bool JOIN(match_texture_, suffix)(const sem::Type* ty, const sem::Type*& T) { \
        return match_texture(ty, dim, T);                                         \
    }                                                                             \
    const sem::SampledTexture* JOIN(build_texture_, suffix)(MatchState & state,   \
                                                            const sem::Type* T) { \
        return state.builder.create<sem::SampledTexture>(dim, T);                 \
    }

DECLARE_SAMPLED_TEXTURE(1d, ast::TextureDimension::k1d)
DECLARE_SAMPLED_TEXTURE(2d, ast::TextureDimension::k2d)
DECLARE_SAMPLED_TEXTURE(2d_array, ast::TextureDimension::k2dArray)
DECLARE_SAMPLED_TEXTURE(3d, ast::TextureDimension::k3d)
DECLARE_SAMPLED_TEXTURE(cube, ast::TextureDimension::kCube)
DECLARE_SAMPLED_TEXTURE(cube_array, ast::TextureDimension::kCubeArray)
#undef DECLARE_SAMPLED_TEXTURE

bool match_texture_multisampled(const sem::Type* ty,
                                ast::TextureDimension dim,
                                const sem::Type*& T) {
    if (ty->Is<Any>()) {
        T = ty;
        return true;
    }
    if (auto* v = ty->As<sem::MultisampledTexture>()) {
        if (v->dim() == dim) {
            T = v->type();
            return true;
        }
    }
    return false;
}

#define DECLARE_MULTISAMPLED_TEXTURE(suffix, dim)                                              \
    bool JOIN(match_texture_multisampled_, suffix)(const sem::Type* ty, const sem::Type*& T) { \
        return match_texture_multisampled(ty, dim, T);                                         \
    }                                                                                          \
    const sem::MultisampledTexture* JOIN(build_texture_multisampled_, suffix)(                 \
        MatchState & state, const sem::Type* T) {                                              \
        return state.builder.create<sem::MultisampledTexture>(dim, T);                         \
    }

DECLARE_MULTISAMPLED_TEXTURE(2d, ast::TextureDimension::k2d)
#undef DECLARE_MULTISAMPLED_TEXTURE

bool match_texture_depth(const sem::Type* ty, ast::TextureDimension dim) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is([&](const sem::DepthTexture* t) { return t->dim() == dim; });
}

#define DECLARE_DEPTH_TEXTURE(suffix, dim)                                            \
    bool JOIN(match_texture_depth_, suffix)(const sem::Type* ty) {                    \
        return match_texture_depth(ty, dim);                                          \
    }                                                                                 \
    const sem::DepthTexture* JOIN(build_texture_depth_, suffix)(MatchState & state) { \
        return state.builder.create<sem::DepthTexture>(dim);                          \
    }

DECLARE_DEPTH_TEXTURE(2d, ast::TextureDimension::k2d)
DECLARE_DEPTH_TEXTURE(2d_array, ast::TextureDimension::k2dArray)
DECLARE_DEPTH_TEXTURE(cube, ast::TextureDimension::kCube)
DECLARE_DEPTH_TEXTURE(cube_array, ast::TextureDimension::kCubeArray)
#undef DECLARE_DEPTH_TEXTURE

bool match_texture_depth_multisampled_2d(const sem::Type* ty) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is([&](const sem::DepthMultisampledTexture* t) {
        return t->dim() == ast::TextureDimension::k2d;
    });
}

sem::DepthMultisampledTexture* build_texture_depth_multisampled_2d(MatchState& state) {
    return state.builder.create<sem::DepthMultisampledTexture>(ast::TextureDimension::k2d);
}

bool match_texture_storage(const sem::Type* ty, ast::TextureDimension dim, Number& F, Number& A) {
    if (ty->Is<Any>()) {
        F = Number::any;
        A = Number::any;
        return true;
    }
    if (auto* v = ty->As<sem::StorageTexture>()) {
        if (v->dim() == dim) {
            F = Number(static_cast<uint32_t>(v->texel_format()));
            A = Number(static_cast<uint32_t>(v->access()));
            return true;
        }
    }
    return false;
}

#define DECLARE_STORAGE_TEXTURE(suffix, dim)                                                      \
    bool JOIN(match_texture_storage_, suffix)(const sem::Type* ty, Number& F, Number& A) {        \
        return match_texture_storage(ty, dim, F, A);                                              \
    }                                                                                             \
    const sem::StorageTexture* JOIN(build_texture_storage_, suffix)(MatchState & state, Number F, \
                                                                    Number A) {                   \
        auto format = static_cast<TexelFormat>(F.Value());                                        \
        auto access = static_cast<Access>(A.Value());                                             \
        auto* T = sem::StorageTexture::SubtypeFor(format, state.builder.Types());                 \
        return state.builder.create<sem::StorageTexture>(dim, format, access, T);                 \
    }

DECLARE_STORAGE_TEXTURE(1d, ast::TextureDimension::k1d)
DECLARE_STORAGE_TEXTURE(2d, ast::TextureDimension::k2d)
DECLARE_STORAGE_TEXTURE(2d_array, ast::TextureDimension::k2dArray)
DECLARE_STORAGE_TEXTURE(3d, ast::TextureDimension::k3d)
#undef DECLARE_STORAGE_TEXTURE

bool match_texture_external(const sem::Type* ty) {
    return ty->IsAnyOf<Any, sem::ExternalTexture>();
}

const sem::ExternalTexture* build_texture_external(MatchState& state) {
    return state.builder.create<sem::ExternalTexture>();
}

// Builtin types starting with a _ prefix cannot be declared in WGSL, so they
// can only be used as return types. Because of this, they must only match Any,
// which is used as the return type matcher.
bool match_modf_result(const sem::Type* ty) {
    return ty->Is<Any>();
}
bool match_modf_result_vec(const sem::Type* ty, Number& N) {
    if (!ty->Is<Any>()) {
        return false;
    }
    N = Number::any;
    return true;
}
bool match_frexp_result(const sem::Type* ty) {
    return ty->Is<Any>();
}
bool match_frexp_result_vec(const sem::Type* ty, Number& N) {
    if (!ty->Is<Any>()) {
        return false;
    }
    N = Number::any;
    return true;
}

struct NameAndType {
    std::string name;
    sem::Type* type;
};
const sem::Struct* build_struct(MatchState& state,
                                std::string name,
                                std::initializer_list<NameAndType> member_names_and_types) {
    uint32_t offset = 0;
    uint32_t max_align = 0;
    sem::StructMemberList members;
    for (auto& m : member_names_and_types) {
        uint32_t align = m.type->Align();
        uint32_t size = m.type->Size();
        offset = utils::RoundUp(align, offset);
        max_align = std::max(max_align, align);
        members.emplace_back(state.builder.create<sem::StructMember>(
            /* declaration */ nullptr,
            /* name */ state.builder.Sym(m.name),
            /* type */ m.type,
            /* index */ static_cast<uint32_t>(members.size()),
            /* offset */ offset,
            /* align */ align,
            /* size */ size));
        offset += size;
    }
    uint32_t size_without_padding = offset;
    uint32_t size_with_padding = utils::RoundUp(max_align, offset);
    return state.builder.create<sem::Struct>(
        /* declaration */ nullptr,
        /* name */ state.builder.Sym(name),
        /* members */ members,
        /* align */ max_align,
        /* size */ size_with_padding,
        /* size_no_padding */ size_without_padding);
}

const sem::Struct* build_modf_result(MatchState& state) {
    auto* f32 = state.builder.create<sem::F32>();
    return build_struct(state, "__modf_result", {{"fract", f32}, {"whole", f32}});
}
const sem::Struct* build_modf_result_vec(MatchState& state, Number& n) {
    auto* vec_f32 = state.builder.create<sem::Vector>(state.builder.create<sem::F32>(), n.Value());
    return build_struct(state, "__modf_result_vec" + std::to_string(n.Value()),
                        {{"fract", vec_f32}, {"whole", vec_f32}});
}
const sem::Struct* build_frexp_result(MatchState& state) {
    auto* f32 = state.builder.create<sem::F32>();
    auto* i32 = state.builder.create<sem::I32>();
    return build_struct(state, "__frexp_result", {{"sig", f32}, {"exp", i32}});
}
const sem::Struct* build_frexp_result_vec(MatchState& state, Number& n) {
    auto* vec_f32 = state.builder.create<sem::Vector>(state.builder.create<sem::F32>(), n.Value());
    auto* vec_i32 = state.builder.create<sem::Vector>(state.builder.create<sem::I32>(), n.Value());
    return build_struct(state, "__frexp_result_vec" + std::to_string(n.Value()),
                        {{"sig", vec_f32}, {"exp", vec_i32}});
}

/// ParameterInfo describes a parameter
struct ParameterInfo {
    /// The parameter usage (parameter name in definition file)
    const ParameterUsage usage;

    /// Pointer to a list of indices that are used to match the parameter type.
    /// The matcher indices index on Matchers::type and / or Matchers::number.
    /// These indices are consumed by the matchers themselves.
    /// The first index is always a TypeMatcher.
    MatcherIndex const* const matcher_indices;
};

/// OpenTypeInfo describes an open type
struct OpenTypeInfo {
    /// Name of the open type (e.g. 'T')
    const char* name;
    /// Optional type matcher constraint.
    /// Either an index in Matchers::type, or kNoMatcher
    const MatcherIndex matcher_index;
};

/// OpenNumberInfo describes an open number
struct OpenNumberInfo {
    /// Name of the open number (e.g. 'N')
    const char* name;
    /// Optional number matcher constraint.
    /// Either an index in Matchers::number, or kNoMatcher
    const MatcherIndex matcher_index;
};

/// OverloadInfo describes a single function overload
struct OverloadInfo {
    /// Total number of parameters for the overload
    const uint8_t num_parameters;
    /// Total number of open types for the overload
    const uint8_t num_open_types;
    /// Total number of open numbers for the overload
    const uint8_t num_open_numbers;
    /// Pointer to the first open type
    OpenTypeInfo const* const open_types;
    /// Pointer to the first open number
    OpenNumberInfo const* const open_numbers;
    /// Pointer to the first parameter
    ParameterInfo const* const parameters;
    /// Pointer to a list of matcher indices that index on Matchers::type and
    /// Matchers::number, used to build the return type. If the function has no
    /// return type then this is null
    MatcherIndex const* const return_matcher_indices;
    /// The pipeline stages that this overload can be used in
    PipelineStageSet supported_stages;
    /// True if the overload is marked as deprecated
    bool is_deprecated;
};

/// BuiltinInfo describes a builtin function
struct BuiltinInfo {
    /// Number of overloads of the builtin function
    const uint8_t num_overloads;
    /// Pointer to the start of the overloads for the function
    OverloadInfo const* const overloads;
};

#include "builtin_table.inl"

/// BuiltinPrototype describes a fully matched builtin function, which is
/// used as a lookup for building unique sem::Builtin instances.
struct BuiltinPrototype {
    /// Parameter describes a single parameter
    struct Parameter {
        /// Parameter type
        const sem::Type* const type;
        /// Parameter usage
        ParameterUsage const usage = ParameterUsage::kNone;
    };

    /// Hasher provides a hash function for the BuiltinPrototype
    struct Hasher {
        /// @param i the BuiltinPrototype to create a hash for
        /// @return the hash value
        inline std::size_t operator()(const BuiltinPrototype& i) const {
            size_t hash = utils::Hash(i.parameters.size());
            for (auto& p : i.parameters) {
                utils::HashCombine(&hash, p.type, p.usage);
            }
            return utils::Hash(hash, i.type, i.return_type, i.supported_stages, i.is_deprecated);
        }
    };

    sem::BuiltinType type = sem::BuiltinType::kNone;
    std::vector<Parameter> parameters;
    sem::Type const* return_type = nullptr;
    PipelineStageSet supported_stages;
    bool is_deprecated = false;
};

/// Equality operator for BuiltinPrototype
bool operator==(const BuiltinPrototype& a, const BuiltinPrototype& b) {
    if (a.type != b.type || a.supported_stages != b.supported_stages ||
        a.return_type != b.return_type || a.is_deprecated != b.is_deprecated ||
        a.parameters.size() != b.parameters.size()) {
        return false;
    }
    for (size_t i = 0; i < a.parameters.size(); i++) {
        auto& pa = a.parameters[i];
        auto& pb = b.parameters[i];
        if (pa.type != pb.type || pa.usage != pb.usage) {
            return false;
        }
    }
    return true;
}

/// Impl is the private implementation of the BuiltinTable interface.
class Impl : public BuiltinTable {
  public:
    explicit Impl(ProgramBuilder& builder);

    const sem::Builtin* Lookup(sem::BuiltinType builtin_type,
                               const std::vector<const sem::Type*>& args,
                               const Source& source) override;

  private:
    const sem::Builtin* Match(sem::BuiltinType builtin_type,
                              const OverloadInfo& overload,
                              const std::vector<const sem::Type*>& args,
                              int& match_score);

    MatchState Match(ClosedState& closed,
                     const OverloadInfo& overload,
                     MatcherIndex const* matcher_indices) const;

    void PrintOverload(std::ostream& ss,
                       const OverloadInfo& overload,
                       sem::BuiltinType builtin_type) const;

    ProgramBuilder& builder;
    Matchers matchers;
    std::unordered_map<BuiltinPrototype, sem::Builtin*, BuiltinPrototype::Hasher> builtins;
};

/// @return a string representing a call to a builtin with the given argument
/// types.
std::string CallSignature(ProgramBuilder& builder,
                          sem::BuiltinType builtin_type,
                          const std::vector<const sem::Type*>& args) {
    std::stringstream ss;
    ss << sem::str(builtin_type) << "(";
    {
        bool first = true;
        for (auto* arg : args) {
            if (!first) {
                ss << ", ";
            }
            first = false;
            ss << arg->UnwrapRef()->FriendlyName(builder.Symbols());
        }
    }
    ss << ")";

    return ss.str();
}

std::string OpenTypeMatcher::String(MatchState& state) const {
    return state.overload.open_types[index_].name;
}

std::string OpenNumberMatcher::String(MatchState& state) const {
    return state.overload.open_numbers[index_].name;
}

Impl::Impl(ProgramBuilder& b) : builder(b) {}

const sem::Builtin* Impl::Lookup(sem::BuiltinType builtin_type,
                                 const std::vector<const sem::Type*>& args,
                                 const Source& source) {
    // Candidate holds information about a mismatched overload that could be what
    // the user intended to call.
    struct Candidate {
        const OverloadInfo* overload;
        int score;
    };

    // The list of failed matches that had promise.
    std::vector<Candidate> candidates;

    auto& builtin = kBuiltins[static_cast<uint32_t>(builtin_type)];
    for (uint32_t o = 0; o < builtin.num_overloads; o++) {
        int match_score = 1000;
        auto& overload = builtin.overloads[o];
        if (auto* match = Match(builtin_type, overload, args, match_score)) {
            return match;
        }
        if (match_score > 0) {
            candidates.emplace_back(Candidate{&overload, match_score});
        }
    }

    // Sort the candidates with the most promising first
    std::stable_sort(candidates.begin(), candidates.end(),
                     [](const Candidate& a, const Candidate& b) { return a.score > b.score; });

    // Generate an error message
    std::stringstream ss;
    ss << "no matching call to " << CallSignature(builder, builtin_type, args) << std::endl;
    if (!candidates.empty()) {
        ss << std::endl;
        ss << candidates.size() << " candidate function" << (candidates.size() > 1 ? "s:" : ":")
           << std::endl;
        for (auto& candidate : candidates) {
            ss << "  ";
            PrintOverload(ss, *candidate.overload, builtin_type);
            ss << std::endl;
        }
    }
    builder.Diagnostics().add_error(diag::System::Resolver, ss.str(), source);
    return nullptr;
}

const sem::Builtin* Impl::Match(sem::BuiltinType builtin_type,
                                const OverloadInfo& overload,
                                const std::vector<const sem::Type*>& args,
                                int& match_score) {
    // Score wait for argument <-> parameter count matches / mismatches
    constexpr int kScorePerParamArgMismatch = -1;
    constexpr int kScorePerMatchedParam = 2;
    constexpr int kScorePerMatchedOpenType = 1;
    constexpr int kScorePerMatchedOpenNumber = 1;

    auto num_parameters = overload.num_parameters;
    auto num_arguments = static_cast<decltype(num_parameters)>(args.size());

    bool overload_matched = true;

    if (num_parameters != num_arguments) {
        match_score += kScorePerParamArgMismatch * (std::max(num_parameters, num_arguments) -
                                                    std::min(num_parameters, num_arguments));
        overload_matched = false;
    }

    ClosedState closed(builder);

    std::vector<BuiltinPrototype::Parameter> parameters;

    auto num_params = std::min(num_parameters, num_arguments);
    for (uint32_t p = 0; p < num_params; p++) {
        auto& parameter = overload.parameters[p];
        auto* indices = parameter.matcher_indices;
        auto* type = Match(closed, overload, indices).Type(args[p]->UnwrapRef());
        if (type) {
            parameters.emplace_back(BuiltinPrototype::Parameter{type, parameter.usage});
            match_score += kScorePerMatchedParam;
        } else {
            overload_matched = false;
        }
    }

    if (overload_matched) {
        // Check all constrained open types matched
        for (uint32_t ot = 0; ot < overload.num_open_types; ot++) {
            auto& open_type = overload.open_types[ot];
            if (open_type.matcher_index != kNoMatcher) {
                auto* index = &open_type.matcher_index;
                if (Match(closed, overload, index).Type(closed.Type(ot))) {
                    match_score += kScorePerMatchedOpenType;
                } else {
                    overload_matched = false;
                }
            }
        }
    }

    if (overload_matched) {
        // Check all constrained open numbers matched
        for (uint32_t on = 0; on < overload.num_open_numbers; on++) {
            auto& open_number = overload.open_numbers[on];
            if (open_number.matcher_index != kNoMatcher) {
                auto* index = &open_number.matcher_index;
                if (Match(closed, overload, index).Num(closed.Num(on)).IsValid()) {
                    match_score += kScorePerMatchedOpenNumber;
                } else {
                    overload_matched = false;
                }
            }
        }
    }

    if (!overload_matched) {
        return nullptr;
    }

    // Build the return type
    const sem::Type* return_type = nullptr;
    if (auto* indices = overload.return_matcher_indices) {
        Any any;
        return_type = Match(closed, overload, indices).Type(&any);
        if (!return_type) {
            std::stringstream ss;
            PrintOverload(ss, overload, builtin_type);
            TINT_ICE(Resolver, builder.Diagnostics())
                << "MatchState.Match() returned null for " << ss.str();
            return nullptr;
        }
    } else {
        return_type = builder.create<sem::Void>();
    }

    BuiltinPrototype builtin;
    builtin.type = builtin_type;
    builtin.return_type = return_type;
    builtin.parameters = std::move(parameters);
    builtin.supported_stages = overload.supported_stages;
    builtin.is_deprecated = overload.is_deprecated;

    // De-duplicate builtins that are identical.
    return utils::GetOrCreate(builtins, builtin, [&] {
        std::vector<sem::Parameter*> params;
        params.reserve(builtin.parameters.size());
        for (auto& p : builtin.parameters) {
            params.emplace_back(builder.create<sem::Parameter>(
                nullptr, static_cast<uint32_t>(params.size()), p.type, ast::StorageClass::kNone,
                ast::Access::kUndefined, p.usage));
        }
        return builder.create<sem::Builtin>(builtin.type, builtin.return_type, std::move(params),
                                            builtin.supported_stages, builtin.is_deprecated);
    });
}

MatchState Impl::Match(ClosedState& closed,
                       const OverloadInfo& overload,
                       MatcherIndex const* matcher_indices) const {
    return MatchState(builder, closed, matchers, overload, matcher_indices);
}

void Impl::PrintOverload(std::ostream& ss,
                         const OverloadInfo& overload,
                         sem::BuiltinType builtin_type) const {
    ClosedState closed(builder);

    ss << builtin_type << "(";
    for (uint32_t p = 0; p < overload.num_parameters; p++) {
        auto& parameter = overload.parameters[p];
        if (p > 0) {
            ss << ", ";
        }
        if (parameter.usage != ParameterUsage::kNone) {
            ss << sem::str(parameter.usage) << ": ";
        }
        auto* indices = parameter.matcher_indices;
        ss << Match(closed, overload, indices).TypeName();
    }
    ss << ")";
    if (overload.return_matcher_indices) {
        ss << " -> ";
        auto* indices = overload.return_matcher_indices;
        ss << Match(closed, overload, indices).TypeName();
    }

    bool first = true;
    auto separator = [&] {
        ss << (first ? "  where: " : ", ");
        first = false;
    };
    for (uint32_t i = 0; i < overload.num_open_types; i++) {
        auto& open_type = overload.open_types[i];
        if (open_type.matcher_index != kNoMatcher) {
            separator();
            ss << open_type.name;
            auto* index = &open_type.matcher_index;
            ss << " is " << Match(closed, overload, index).TypeName();
        }
    }
    for (uint32_t i = 0; i < overload.num_open_numbers; i++) {
        auto& open_number = overload.open_numbers[i];
        if (open_number.matcher_index != kNoMatcher) {
            separator();
            ss << open_number.name;
            auto* index = &open_number.matcher_index;
            ss << " is " << Match(closed, overload, index).NumName();
        }
    }
}

const sem::Type* MatchState::Type(const sem::Type* ty) {
    MatcherIndex matcher_index = *matcher_indices_++;
    auto* matcher = matchers.type[matcher_index];
    return matcher->Match(*this, ty);
}

Number MatchState::Num(Number number) {
    MatcherIndex matcher_index = *matcher_indices_++;
    auto* matcher = matchers.number[matcher_index];
    return matcher->Match(*this, number);
}

std::string MatchState::TypeName() {
    MatcherIndex matcher_index = *matcher_indices_++;
    auto* matcher = matchers.type[matcher_index];
    return matcher->String(*this);
}

std::string MatchState::NumName() {
    MatcherIndex matcher_index = *matcher_indices_++;
    auto* matcher = matchers.number[matcher_index];
    return matcher->String(*this);
}

}  // namespace

std::unique_ptr<BuiltinTable> BuiltinTable::Create(ProgramBuilder& builder) {
    return std::make_unique<Impl>(builder);
}

BuiltinTable::~BuiltinTable() = default;

/// TypeInfo for the Any type declared in the anonymous namespace above
TINT_INSTANTIATE_TYPEINFO(Any);

}  // namespace tint
