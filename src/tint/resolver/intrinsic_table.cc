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

#include "src/tint/resolver/intrinsic_table.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/tint/ast/binary_expression.h"
#include "src/tint/program_builder.h"
#include "src/tint/resolver/builtin_structs.h"
#include "src/tint/sem/evaluation_stage.h"
#include "src/tint/sem/pipeline_stage_set.h"
#include "src/tint/sem/value_constructor.h"
#include "src/tint/sem/value_conversion.h"
#include "src/tint/switch.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/abstract_numeric.h"
#include "src/tint/type/atomic.h"
#include "src/tint/type/depth_multisampled_texture.h"
#include "src/tint/type/depth_texture.h"
#include "src/tint/type/external_texture.h"
#include "src/tint/type/multisampled_texture.h"
#include "src/tint/type/sampled_texture.h"
#include "src/tint/type/storage_texture.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/utils/hash.h"
#include "src/tint/utils/hashmap.h"
#include "src/tint/utils/math.h"
#include "src/tint/utils/scoped_assignment.h"
#include "src/tint/utils/string_stream.h"

namespace tint::resolver {
namespace {

// Forward declarations
struct OverloadInfo;
class Matchers;
class NumberMatcher;
class TypeMatcher;

/// The utils::Vector `N` template argument value for arrays of parameters.
constexpr static const size_t kNumFixedParams = 8;

/// The utils::Vector `N` template argument value for arrays of overload candidates.
constexpr static const size_t kNumFixedCandidates = 8;

/// A special type that matches all TypeMatchers
class Any final : public utils::Castable<Any, type::Type> {
  public:
    Any() : Base(0u, type::Flags{}) {}
    ~Any() override = default;

    // Stub implementations for type::Type conformance.
    bool Equals(const type::UniqueNode&) const override { return false; }
    std::string FriendlyName() const override { return "<any>"; }
    type::Type* Clone(type::CloneContext&) const override { return nullptr; }
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

/// TemplateState holds the state of the template numbers and types.
/// Used by the MatchState.
class TemplateState {
  public:
    /// If the template type with index `idx` is undefined, then it is defined with the `ty` and
    /// Type() returns `ty`.
    /// If the template type is defined, and `ty` can be converted to the template type then the
    /// template type is returned.
    /// If the template type is defined, and the template type can be converted to `ty`, then the
    /// template type is replaced with `ty`, and `ty` is returned.
    /// If none of the above applies, then `ty` is a type mismatch for the template type, and
    /// nullptr is returned.
    const type::Type* Type(size_t idx, const type::Type* ty) {
        if (idx >= types_.Length()) {
            types_.Resize(idx + 1);
        }
        auto& t = types_[idx];
        if (t == nullptr) {
            t = ty;
            return ty;
        }
        ty = type::Type::Common(utils::Vector{t, ty});
        if (ty) {
            t = ty;
        }
        return ty;
    }

    /// If the number with index `idx` is undefined, then it is defined with the number `number` and
    /// Num() returns true. If the number is defined, then `Num()` returns true iff it is equal to
    /// `ty`.
    bool Num(size_t idx, Number number) {
        if (idx >= numbers_.Length()) {
            numbers_.Resize(idx + 1, Number::invalid);
        }
        auto& n = numbers_[idx];
        if (!n.IsValid()) {
            n = number.Value();
            return true;
        }
        return n.Value() == number.Value();
    }

    /// Type returns the template type with index `idx`, or nullptr if the type was not defined.
    const type::Type* Type(size_t idx) const {
        if (idx >= types_.Length()) {
            return nullptr;
        }
        return types_[idx];
    }

    /// SetType replaces the template type with index `idx` with type `ty`.
    void SetType(size_t idx, const type::Type* ty) {
        if (idx >= types_.Length()) {
            types_.Resize(idx + 1);
        }
        types_[idx] = ty;
    }

    /// Type returns the number type with index `idx`.
    Number Num(size_t idx) const {
        if (idx >= numbers_.Length()) {
            return Number::invalid;
        }
        return numbers_[idx];
    }

    /// @return the total number of type and number templates
    size_t Count() const { return types_.Length() + numbers_.Length(); }

  private:
    utils::Vector<const type::Type*, 4> types_;
    utils::Vector<Number, 2> numbers_;
};

/// Index type used for matcher indices
using MatcherIndex = uint8_t;

/// Index value used for template types / numbers that do not have a constraint
constexpr MatcherIndex kNoMatcher = std::numeric_limits<MatcherIndex>::max();

/// MatchState holds the state used to match an overload.
class MatchState {
  public:
    MatchState(ProgramBuilder& b,
               TemplateState& t,
               const Matchers& m,
               const OverloadInfo* o,
               MatcherIndex const* matcher_indices,
               sem::EvaluationStage s)
        : builder(b),
          templates(t),
          matchers(m),
          overload(o),
          earliest_eval_stage(s),
          matcher_indices_(matcher_indices) {}

    /// The program builder
    ProgramBuilder& builder;
    /// The template types and numbers
    TemplateState& templates;
    /// The type and number matchers
    Matchers const& matchers;
    /// The current overload being evaluated
    OverloadInfo const* overload;
    /// The earliest evaluation stage of the builtin call
    sem::EvaluationStage earliest_eval_stage;

    /// Type uses the next TypeMatcher from the matcher indices to match the type
    /// `ty`. If the type matches, the canonical expected type is returned. If the
    /// type `ty` does not match, then nullptr is returned.
    /// @note: The matcher indices are progressed on calling.
    const type::Type* Type(const type::Type* ty);

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
    /// Match may define and refine the template types and numbers in state.
    /// @param type the type to match
    /// @returns the canonicalized type on match, otherwise nullptr
    virtual const type::Type* Match(MatchState& state, const type::Type* type) const = 0;

    /// @return a string representation of the matcher. Used for printing error
    /// messages when no overload is found.
    virtual std::string String(MatchState* state) const = 0;
};

/// A NumberMatcher is the interface used to match a number or enumerator used
/// as part of an overload's parameter or return type.
class NumberMatcher {
  public:
    /// Destructor
    virtual ~NumberMatcher() = default;

    /// Checks whether the given number matches the matcher rules.
    /// Match may define template numbers in state.
    /// @param number the number to match
    /// @returns true if the argument type is as expected.
    virtual Number Match(MatchState& state, Number number) const = 0;

    /// @return a string representation of the matcher. Used for printing error
    /// messages when no overload is found.
    virtual std::string String(MatchState* state) const = 0;
};

/// TemplateTypeMatcher is a Matcher for a template type.
/// The TemplateTypeMatcher will initially match against any type, and then will only be further
/// constrained based on the conversion rules defined at https://www.w3.org/TR/WGSL/#conversion-rank
class TemplateTypeMatcher : public TypeMatcher {
  public:
    /// Constructor
    explicit TemplateTypeMatcher(size_t index) : index_(index) {}

    const type::Type* Match(MatchState& state, const type::Type* type) const override {
        if (type->Is<Any>()) {
            return state.templates.Type(index_);
        }
        if (auto* templates = state.templates.Type(index_, type)) {
            return templates;
        }
        return nullptr;
    }

    std::string String(MatchState* state) const override;

  private:
    size_t index_;
};

/// TemplateNumberMatcher is a Matcher for a template number.
/// The TemplateNumberMatcher will match against any number (so long as it is
/// consistent for all uses in the overload)
class TemplateNumberMatcher : public NumberMatcher {
  public:
    explicit TemplateNumberMatcher(size_t index) : index_(index) {}

    Number Match(MatchState& state, Number number) const override {
        if (number.IsAny()) {
            return state.templates.Num(index_);
        }
        return state.templates.Num(index_, number) ? number : Number::invalid;
    }

    std::string String(MatchState* state) const override;

  private:
    size_t index_;
};

////////////////////////////////////////////////////////////////////////////////
// Binding functions for use in the generated builtin_table.inl
// TODO(bclayton): See if we can move more of this hand-rolled code to the
// template
////////////////////////////////////////////////////////////////////////////////
using TexelFormat = builtin::TexelFormat;
using Access = builtin::Access;
using AddressSpace = builtin::AddressSpace;
using ParameterUsage = sem::ParameterUsage;
using PipelineStage = ast::PipelineStage;

/// Unique flag bits for overloads
enum class OverloadFlag {
    kIsBuiltin,                 // The overload is a builtin ('fn')
    kIsOperator,                // The overload is an operator ('op')
    kIsConstructor,             // The overload is a value constructor ('ctor')
    kIsConverter,               // The overload is a value converter ('conv')
    kSupportsVertexPipeline,    // The overload can be used in vertex shaders
    kSupportsFragmentPipeline,  // The overload can be used in fragment shaders
    kSupportsComputePipeline,   // The overload can be used in compute shaders
    kMustUse,                   // The overload cannot be called as a statement
    kIsDeprecated,              // The overload is deprecated
};

// An enum set of OverloadFlag, used by OperatorInfo
using OverloadFlags = utils::EnumSet<OverloadFlag>;

bool match_bool(MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<Any, type::Bool>();
}

const type::AbstractFloat* build_fa(MatchState& state) {
    return state.builder.create<type::AbstractFloat>();
}

bool match_fa(MatchState& state, const type::Type* ty) {
    return (state.earliest_eval_stage <= sem::EvaluationStage::kConstant) &&
           ty->IsAnyOf<Any, type::AbstractNumeric>();
}

const type::AbstractInt* build_ia(MatchState& state) {
    return state.builder.create<type::AbstractInt>();
}

bool match_ia(MatchState& state, const type::Type* ty) {
    return (state.earliest_eval_stage <= sem::EvaluationStage::kConstant) &&
           ty->IsAnyOf<Any, type::AbstractInt>();
}

const type::Bool* build_bool(MatchState& state) {
    return state.builder.create<type::Bool>();
}

const type::F16* build_f16(MatchState& state) {
    return state.builder.create<type::F16>();
}

bool match_f16(MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<Any, type::F16, type::AbstractNumeric>();
}

const type::F32* build_f32(MatchState& state) {
    return state.builder.create<type::F32>();
}

bool match_f32(MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<Any, type::F32, type::AbstractNumeric>();
}

const type::I32* build_i32(MatchState& state) {
    return state.builder.create<type::I32>();
}

bool match_i32(MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<Any, type::I32, type::AbstractInt>();
}

const type::U32* build_u32(MatchState& state) {
    return state.builder.create<type::U32>();
}

bool match_u32(MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<Any, type::U32, type::AbstractInt>();
}

bool match_vec(MatchState&, const type::Type* ty, Number& N, const type::Type*& T) {
    if (ty->Is<Any>()) {
        N = Number::any;
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
bool match_vec(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<Any>()) {
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

const type::Vector* build_vec(MatchState& state, Number N, const type::Type* el) {
    return state.builder.create<type::Vector>(el, N.Value());
}

template <uint32_t N>
const type::Vector* build_vec(MatchState& state, const type::Type* el) {
    return state.builder.create<type::Vector>(el, N);
}

constexpr auto match_vec2 = match_vec<2>;
constexpr auto match_vec3 = match_vec<3>;
constexpr auto match_vec4 = match_vec<4>;

constexpr auto build_vec2 = build_vec<2>;
constexpr auto build_vec3 = build_vec<3>;
constexpr auto build_vec4 = build_vec<4>;

bool match_packedVec3(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<Any>()) {
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

const type::Vector* build_packedVec3(MatchState& state, const type::Type* el) {
    return state.builder.create<type::Vector>(el, 3u, /* packed */ true);
}

bool match_mat(MatchState&, const type::Type* ty, Number& M, Number& N, const type::Type*& T) {
    if (ty->Is<Any>()) {
        M = Number::any;
        N = Number::any;
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
bool match_mat(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<Any>()) {
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

const type::Matrix* build_mat(MatchState& state, Number C, Number R, const type::Type* T) {
    auto* column_type = state.builder.create<type::Vector>(T, R.Value());
    return state.builder.create<type::Matrix>(column_type, C.Value());
}

template <uint32_t C, uint32_t R>
const type::Matrix* build_mat(MatchState& state, const type::Type* T) {
    auto* column_type = state.builder.create<type::Vector>(T, R);
    return state.builder.create<type::Matrix>(column_type, C);
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

bool match_array(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<Any>()) {
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

const type::Array* build_array(MatchState& state, const type::Type* el) {
    return state.builder.create<type::Array>(
        el,
        /* count */ state.builder.create<type::RuntimeArrayCount>(),
        /* align */ 0u,
        /* size */ 0u,
        /* stride */ 0u,
        /* stride_implicit */ 0u);
}

bool match_ptr(MatchState&, const type::Type* ty, Number& S, const type::Type*& T, Number& A) {
    if (ty->Is<Any>()) {
        S = Number::any;
        T = ty;
        A = Number::any;
        return true;
    }

    if (auto* p = ty->As<type::Pointer>()) {
        S = Number(static_cast<uint32_t>(p->AddressSpace()));
        T = p->StoreType();
        A = Number(static_cast<uint32_t>(p->Access()));
        return true;
    }
    return false;
}

const type::Pointer* build_ptr(MatchState& state, Number S, const type::Type* T, Number& A) {
    return state.builder.create<type::Pointer>(T, static_cast<builtin::AddressSpace>(S.Value()),
                                               static_cast<builtin::Access>(A.Value()));
}

bool match_atomic(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<Any>()) {
        T = ty;
        return true;
    }

    if (auto* a = ty->As<type::Atomic>()) {
        T = a->Type();
        return true;
    }
    return false;
}

const type::Atomic* build_atomic(MatchState& state, const type::Type* T) {
    return state.builder.create<type::Atomic>(T);
}

bool match_sampler(MatchState&, const type::Type* ty) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is([](const type::Sampler* s) { return s->kind() == type::SamplerKind::kSampler; });
}

const type::Sampler* build_sampler(MatchState& state) {
    return state.builder.create<type::Sampler>(type::SamplerKind::kSampler);
}

bool match_sampler_comparison(MatchState&, const type::Type* ty) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is(
        [](const type::Sampler* s) { return s->kind() == type::SamplerKind::kComparisonSampler; });
}

const type::Sampler* build_sampler_comparison(MatchState& state) {
    return state.builder.create<type::Sampler>(type::SamplerKind::kComparisonSampler);
}

bool match_texture(MatchState&,
                   const type::Type* ty,
                   type::TextureDimension dim,
                   const type::Type*& T) {
    if (ty->Is<Any>()) {
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

#define DECLARE_SAMPLED_TEXTURE(suffix, dim)                                        \
    bool JOIN(match_texture_, suffix)(MatchState & state, const type::Type* ty,     \
                                      const type::Type*& T) {                       \
        return match_texture(state, ty, dim, T);                                    \
    }                                                                               \
    const type::SampledTexture* JOIN(build_texture_, suffix)(MatchState & state,    \
                                                             const type::Type* T) { \
        return state.builder.create<type::SampledTexture>(dim, T);                  \
    }

DECLARE_SAMPLED_TEXTURE(1d, type::TextureDimension::k1d)
DECLARE_SAMPLED_TEXTURE(2d, type::TextureDimension::k2d)
DECLARE_SAMPLED_TEXTURE(2d_array, type::TextureDimension::k2dArray)
DECLARE_SAMPLED_TEXTURE(3d, type::TextureDimension::k3d)
DECLARE_SAMPLED_TEXTURE(cube, type::TextureDimension::kCube)
DECLARE_SAMPLED_TEXTURE(cube_array, type::TextureDimension::kCubeArray)
#undef DECLARE_SAMPLED_TEXTURE

bool match_texture_multisampled(MatchState&,
                                const type::Type* ty,
                                type::TextureDimension dim,
                                const type::Type*& T) {
    if (ty->Is<Any>()) {
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

#define DECLARE_MULTISAMPLED_TEXTURE(suffix, dim)                                            \
    bool JOIN(match_texture_multisampled_, suffix)(MatchState & state, const type::Type* ty, \
                                                   const type::Type*& T) {                   \
        return match_texture_multisampled(state, ty, dim, T);                                \
    }                                                                                        \
    const type::MultisampledTexture* JOIN(build_texture_multisampled_, suffix)(              \
        MatchState & state, const type::Type* T) {                                           \
        return state.builder.create<type::MultisampledTexture>(dim, T);                      \
    }

DECLARE_MULTISAMPLED_TEXTURE(2d, type::TextureDimension::k2d)
#undef DECLARE_MULTISAMPLED_TEXTURE

bool match_texture_depth(MatchState&, const type::Type* ty, type::TextureDimension dim) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is([&](const type::DepthTexture* t) { return t->dim() == dim; });
}

#define DECLARE_DEPTH_TEXTURE(suffix, dim)                                              \
    bool JOIN(match_texture_depth_, suffix)(MatchState & state, const type::Type* ty) { \
        return match_texture_depth(state, ty, dim);                                     \
    }                                                                                   \
    const type::DepthTexture* JOIN(build_texture_depth_, suffix)(MatchState & state) {  \
        return state.builder.create<type::DepthTexture>(dim);                           \
    }

DECLARE_DEPTH_TEXTURE(2d, type::TextureDimension::k2d)
DECLARE_DEPTH_TEXTURE(2d_array, type::TextureDimension::k2dArray)
DECLARE_DEPTH_TEXTURE(cube, type::TextureDimension::kCube)
DECLARE_DEPTH_TEXTURE(cube_array, type::TextureDimension::kCubeArray)
#undef DECLARE_DEPTH_TEXTURE

bool match_texture_depth_multisampled_2d(MatchState&, const type::Type* ty) {
    if (ty->Is<Any>()) {
        return true;
    }
    return ty->Is([&](const type::DepthMultisampledTexture* t) {
        return t->dim() == type::TextureDimension::k2d;
    });
}

type::DepthMultisampledTexture* build_texture_depth_multisampled_2d(MatchState& state) {
    return state.builder.create<type::DepthMultisampledTexture>(type::TextureDimension::k2d);
}

bool match_texture_storage(MatchState&,
                           const type::Type* ty,
                           type::TextureDimension dim,
                           Number& F,
                           Number& A) {
    if (ty->Is<Any>()) {
        F = Number::any;
        A = Number::any;
        return true;
    }
    if (auto* v = ty->As<type::StorageTexture>()) {
        if (v->dim() == dim) {
            F = Number(static_cast<uint32_t>(v->texel_format()));
            A = Number(static_cast<uint32_t>(v->access()));
            return true;
        }
    }
    return false;
}

#define DECLARE_STORAGE_TEXTURE(suffix, dim)                                                       \
    bool JOIN(match_texture_storage_, suffix)(MatchState & state, const type::Type* ty, Number& F, \
                                              Number& A) {                                         \
        return match_texture_storage(state, ty, dim, F, A);                                        \
    }                                                                                              \
    const type::StorageTexture* JOIN(build_texture_storage_, suffix)(MatchState & state, Number F, \
                                                                     Number A) {                   \
        auto format = static_cast<TexelFormat>(F.Value());                                         \
        auto access = static_cast<Access>(A.Value());                                              \
        auto* T = type::StorageTexture::SubtypeFor(format, state.builder.Types());                 \
        return state.builder.create<type::StorageTexture>(dim, format, access, T);                 \
    }

DECLARE_STORAGE_TEXTURE(1d, type::TextureDimension::k1d)
DECLARE_STORAGE_TEXTURE(2d, type::TextureDimension::k2d)
DECLARE_STORAGE_TEXTURE(2d_array, type::TextureDimension::k2dArray)
DECLARE_STORAGE_TEXTURE(3d, type::TextureDimension::k3d)
#undef DECLARE_STORAGE_TEXTURE

bool match_texture_external(MatchState&, const type::Type* ty) {
    return ty->IsAnyOf<Any, type::ExternalTexture>();
}

const type::ExternalTexture* build_texture_external(MatchState& state) {
    return state.builder.create<type::ExternalTexture>();
}

// Builtin types starting with a _ prefix cannot be declared in WGSL, so they
// can only be used as return types. Because of this, they must only match Any,
// which is used as the return type matcher.
bool match_modf_result(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (!ty->Is<Any>()) {
        return false;
    }
    T = ty;
    return true;
}
bool match_modf_result_vec(MatchState&, const type::Type* ty, Number& N, const type::Type*& T) {
    if (!ty->Is<Any>()) {
        return false;
    }
    N = Number::any;
    T = ty;
    return true;
}
bool match_frexp_result(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (!ty->Is<Any>()) {
        return false;
    }
    T = ty;
    return true;
}
bool match_frexp_result_vec(MatchState&, const type::Type* ty, Number& N, const type::Type*& T) {
    if (!ty->Is<Any>()) {
        return false;
    }
    N = Number::any;
    T = ty;
    return true;
}

bool match_atomic_compare_exchange_result(MatchState&, const type::Type* ty, const type::Type*& T) {
    if (ty->Is<Any>()) {
        T = ty;
        return true;
    }
    return false;
}

const type::Struct* build_modf_result(MatchState& state, const type::Type* el) {
    return CreateModfResult(state.builder, el);
}

const type::Struct* build_modf_result_vec(MatchState& state, Number& n, const type::Type* el) {
    auto* vec = state.builder.create<type::Vector>(el, n.Value());
    return CreateModfResult(state.builder, vec);
}

const type::Struct* build_frexp_result(MatchState& state, const type::Type* el) {
    return CreateFrexpResult(state.builder, el);
}

const type::Struct* build_frexp_result_vec(MatchState& state, Number& n, const type::Type* el) {
    auto* vec = state.builder.create<type::Vector>(el, n.Value());
    return CreateFrexpResult(state.builder, vec);
}

const type::Struct* build_atomic_compare_exchange_result(MatchState& state, const type::Type* ty) {
    return CreateAtomicCompareExchangeResult(state.builder, ty);
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

/// TemplateTypeInfo describes an template type
struct TemplateTypeInfo {
    /// Name of the template type (e.g. 'T')
    const char* name;
    /// Optional type matcher constraint.
    /// Either an index in Matchers::type, or kNoMatcher
    const MatcherIndex matcher_index;
};

/// TemplateNumberInfo describes a template number
struct TemplateNumberInfo {
    /// Name of the template number (e.g. 'N')
    const char* name;
    /// Optional number matcher constraint.
    /// Either an index in Matchers::number, or kNoMatcher
    const MatcherIndex matcher_index;
};

/// OverloadInfo describes a single function overload
struct OverloadInfo {
    /// Total number of parameters for the overload
    const uint8_t num_parameters;
    /// Total number of template types for the overload
    const uint8_t num_template_types;
    /// Total number of template numbers for the overload
    const uint8_t num_template_numbers;
    /// Pointer to the first template type
    TemplateTypeInfo const* const template_types;
    /// Pointer to the first template number
    TemplateNumberInfo const* const template_numbers;
    /// Pointer to the first parameter
    ParameterInfo const* const parameters;
    /// Pointer to a list of matcher indices that index on Matchers::type and
    /// Matchers::number, used to build the return type. If the function has no
    /// return type then this is null
    MatcherIndex const* const return_matcher_indices;
    /// The flags for the overload
    OverloadFlags flags;
    /// The function used to evaluate the overload at shader-creation time.
    ConstEval::Function const const_eval_fn;
};

/// IntrinsicInfo describes a builtin function or operator overload
struct IntrinsicInfo {
    /// Number of overloads of the intrinsic
    const uint8_t num_overloads;
    /// Pointer to the start of the overloads for the function
    OverloadInfo const* const overloads;
};

#include "intrinsic_table.inl"

/// IntrinsicPrototype describes a fully matched intrinsic.
struct IntrinsicPrototype {
    /// Parameter describes a single parameter
    struct Parameter {
        /// Parameter type
        const type::Type* const type;
        /// Parameter usage
        ParameterUsage const usage = ParameterUsage::kNone;
    };

    /// Hasher provides a hash function for the IntrinsicPrototype
    struct Hasher {
        /// @param i the IntrinsicPrototype to create a hash for
        /// @return the hash value
        inline std::size_t operator()(const IntrinsicPrototype& i) const {
            size_t hash = utils::Hash(i.parameters.Length());
            for (auto& p : i.parameters) {
                hash = utils::HashCombine(hash, p.type, p.usage);
            }
            return utils::Hash(hash, i.overload, i.return_type);
        }
    };

    const OverloadInfo* overload = nullptr;
    type::Type const* return_type = nullptr;
    utils::Vector<Parameter, kNumFixedParams> parameters;
};

/// Equality operator for IntrinsicPrototype
bool operator==(const IntrinsicPrototype& a, const IntrinsicPrototype& b) {
    if (a.overload != b.overload || a.return_type != b.return_type ||
        a.parameters.Length() != b.parameters.Length()) {
        return false;
    }
    for (size_t i = 0; i < a.parameters.Length(); i++) {
        auto& pa = a.parameters[i];
        auto& pb = b.parameters[i];
        if (pa.type != pb.type || pa.usage != pb.usage) {
            return false;
        }
    }
    return true;
}

/// Impl is the private implementation of the IntrinsicTable interface.
class Impl : public IntrinsicTable {
  public:
    explicit Impl(ProgramBuilder& builder);

    Builtin Lookup(builtin::Function builtin_type,
                   utils::VectorRef<const type::Type*> args,
                   sem::EvaluationStage earliest_eval_stage,
                   const Source& source) override;

    UnaryOperator Lookup(ast::UnaryOp op,
                         const type::Type* arg,
                         sem::EvaluationStage earliest_eval_stage,
                         const Source& source) override;

    BinaryOperator Lookup(ast::BinaryOp op,
                          const type::Type* lhs,
                          const type::Type* rhs,
                          sem::EvaluationStage earliest_eval_stage,
                          const Source& source,
                          bool is_compound) override;

    CtorOrConv Lookup(CtorConvIntrinsic type,
                      const type::Type* template_arg,
                      utils::VectorRef<const type::Type*> args,
                      sem::EvaluationStage earliest_eval_stage,
                      const Source& source) override;

  private:
    /// Candidate holds information about an overload evaluated for resolution.
    struct Candidate {
        /// The candidate overload
        const OverloadInfo* overload;
        /// The template types and numbers
        TemplateState templates;
        /// The parameter types for the candidate overload
        utils::Vector<IntrinsicPrototype::Parameter, kNumFixedParams> parameters;
        /// The match-score of the candidate overload.
        /// A score of zero indicates an exact match.
        /// Non-zero scores are used for diagnostics when no overload matches.
        /// Lower scores are displayed first (top-most).
        size_t score;
    };

    /// A list of candidates
    using Candidates = utils::Vector<Candidate, kNumFixedCandidates>;

    /// Callback function when no overloads match.
    using OnNoMatch = std::function<void(utils::VectorRef<Candidate>)>;

    /// Sorts the candidates based on their score, with the lowest (best-ranking) scores first.
    static inline void SortCandidates(Candidates& candidates) {
        std::stable_sort(candidates.begin(), candidates.end(),
                         [&](const Candidate& a, const Candidate& b) { return a.score < b.score; });
    }

    /// Attempts to find a single intrinsic overload that matches the provided argument types.
    /// @param intrinsic the intrinsic being called
    /// @param intrinsic_name the name of the intrinsic
    /// @param args the argument types
    /// @param templates initial template state. This may contain explicitly specified template
    ///                  arguments. For example `vec3<f32>()` would have the first template-type
    ///                  defined as `f32`.
    /// @param on_no_match an error callback when no intrinsic overloads matched the provided
    ///                    arguments.
    /// @returns the matched intrinsic. If no intrinsic could be matched then IntrinsicPrototype
    ///          will hold nullptrs for IntrinsicPrototype::overload and
    ///          IntrinsicPrototype::return_type.
    IntrinsicPrototype MatchIntrinsic(const IntrinsicInfo& intrinsic,
                                      const char* intrinsic_name,
                                      utils::VectorRef<const type::Type*> args,
                                      sem::EvaluationStage earliest_eval_stage,
                                      TemplateState templates,
                                      const OnNoMatch& on_no_match) const;

    /// Evaluates the single overload for the provided argument types.
    /// @param overload the overload being considered
    /// @param args the argument types
    /// @param templates initial template state. This may contain explicitly specified template
    ///                  arguments. For example `vec3<f32>()` would have the first template-type
    ///                  template as `f32`.
    /// @returns the evaluated Candidate information.
    Candidate ScoreOverload(const OverloadInfo* overload,
                            utils::VectorRef<const type::Type*> args,
                            sem::EvaluationStage earliest_eval_stage,
                            const TemplateState& templates) const;

    /// Performs overload resolution given the list of candidates, by ranking the conversions of
    /// arguments to the each of the candidate's parameter types.
    /// @param candidates the list of candidate overloads
    /// @param intrinsic_name the name of the intrinsic
    /// @param args the argument types
    /// @param templates initial template state. This may contain explicitly specified template
    ///                  arguments. For example `vec3<f32>()` would have the first template-type
    ///                  template as `f32`.
    /// @see https://www.w3.org/TR/WGSL/#overload-resolution-section
    /// @returns the resolved Candidate.
    Candidate ResolveCandidate(Candidates&& candidates,
                               const char* intrinsic_name,
                               utils::VectorRef<const type::Type*> args,
                               TemplateState templates) const;

    /// Match constructs a new MatchState
    /// @param templates the template state used for matcher evaluation
    /// @param overload the overload being evaluated
    /// @param matcher_indices pointer to a list of matcher indices
    MatchState Match(TemplateState& templates,
                     const OverloadInfo* overload,
                     MatcherIndex const* matcher_indices,
                     sem::EvaluationStage earliest_eval_stage) const;

    // Prints the overload for emitting diagnostics
    void PrintOverload(utils::StringStream& ss,
                       const OverloadInfo* overload,
                       const char* intrinsic_name) const;

    // Prints the list of candidates for emitting diagnostics
    void PrintCandidates(utils::StringStream& ss,
                         utils::VectorRef<Candidate> candidates,
                         const char* intrinsic_name) const;

    /// Raises an error when no overload is a clear winner of overload resolution
    void ErrAmbiguousOverload(const char* intrinsic_name,
                              utils::VectorRef<const type::Type*> args,
                              TemplateState templates,
                              utils::VectorRef<Candidate> candidates) const;

    ProgramBuilder& builder;
    Matchers matchers;
    utils::Hashmap<IntrinsicPrototype, sem::Builtin*, 64, IntrinsicPrototype::Hasher> builtins;
    utils::Hashmap<IntrinsicPrototype, sem::ValueConstructor*, 16, IntrinsicPrototype::Hasher>
        constructors;
    utils::Hashmap<IntrinsicPrototype, sem::ValueConversion*, 16, IntrinsicPrototype::Hasher>
        converters;
};

/// @return a string representing a call to a builtin with the given argument
/// types.
std::string CallSignature(const char* intrinsic_name,
                          utils::VectorRef<const type::Type*> args,
                          const type::Type* template_arg = nullptr) {
    utils::StringStream ss;
    ss << intrinsic_name;
    if (template_arg) {
        ss << "<" << template_arg->FriendlyName() << ">";
    }
    ss << "(";
    {
        bool first = true;
        for (auto* arg : args) {
            if (!first) {
                ss << ", ";
            }
            first = false;
            ss << arg->UnwrapRef()->FriendlyName();
        }
    }
    ss << ")";

    return ss.str();
}

std::string TemplateTypeMatcher::String(MatchState* state) const {
    return state->overload->template_types[index_].name;
}

std::string TemplateNumberMatcher::String(MatchState* state) const {
    return state->overload->template_numbers[index_].name;
}

Impl::Impl(ProgramBuilder& b) : builder(b) {}

Impl::Builtin Impl::Lookup(builtin::Function builtin_type,
                           utils::VectorRef<const type::Type*> args,
                           sem::EvaluationStage earliest_eval_stage,
                           const Source& source) {
    const char* intrinsic_name = builtin::str(builtin_type);

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&](utils::VectorRef<Candidate> candidates) {
        utils::StringStream ss;
        ss << "no matching call to " << CallSignature(intrinsic_name, args) << std::endl;
        if (!candidates.IsEmpty()) {
            ss << std::endl
               << candidates.Length() << " candidate function"
               << (candidates.Length() > 1 ? "s:" : ":") << std::endl;
            PrintCandidates(ss, candidates, intrinsic_name);
        }
        builder.Diagnostics().add_error(diag::System::Resolver, ss.str(), source);
    };

    // Resolve the intrinsic overload
    auto match = MatchIntrinsic(kBuiltins[static_cast<size_t>(builtin_type)], intrinsic_name, args,
                                earliest_eval_stage, TemplateState{}, on_no_match);
    if (!match.overload) {
        return {};
    }

    // De-duplicate builtins that are identical.
    auto* sem = builtins.GetOrCreate(match, [&] {
        utils::Vector<sem::Parameter*, kNumFixedParams> params;
        params.Reserve(match.parameters.Length());
        for (auto& p : match.parameters) {
            params.Push(builder.create<sem::Parameter>(
                nullptr, static_cast<uint32_t>(params.Length()), p.type,
                builtin::AddressSpace::kUndefined, builtin::Access::kUndefined, p.usage));
        }
        sem::PipelineStageSet supported_stages;
        auto& overload = *match.overload;
        if (overload.flags.Contains(OverloadFlag::kSupportsVertexPipeline)) {
            supported_stages.Add(ast::PipelineStage::kVertex);
        }
        if (overload.flags.Contains(OverloadFlag::kSupportsFragmentPipeline)) {
            supported_stages.Add(ast::PipelineStage::kFragment);
        }
        if (overload.flags.Contains(OverloadFlag::kSupportsComputePipeline)) {
            supported_stages.Add(ast::PipelineStage::kCompute);
        }
        auto eval_stage = overload.const_eval_fn ? sem::EvaluationStage::kConstant
                                                 : sem::EvaluationStage::kRuntime;
        return builder.create<sem::Builtin>(builtin_type, match.return_type, std::move(params),
                                            eval_stage, supported_stages,
                                            overload.flags.Contains(OverloadFlag::kIsDeprecated),
                                            overload.flags.Contains(OverloadFlag::kMustUse));
    });
    return Builtin{sem, match.overload->const_eval_fn};
}

IntrinsicTable::UnaryOperator Impl::Lookup(ast::UnaryOp op,
                                           const type::Type* arg,
                                           sem::EvaluationStage earliest_eval_stage,
                                           const Source& source) {
    auto [intrinsic_index, intrinsic_name] = [&]() -> std::pair<size_t, const char*> {
        switch (op) {
            case ast::UnaryOp::kComplement:
                return {kUnaryOperatorComplement, "operator ~ "};
            case ast::UnaryOp::kNegation:
                return {kUnaryOperatorMinus, "operator - "};
            case ast::UnaryOp::kNot:
                return {kUnaryOperatorNot, "operator ! "};
            default:
                return {0, "<unknown>"};
        }
    }();

    utils::Vector args{arg};

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&, name = intrinsic_name](utils::VectorRef<Candidate> candidates) {
        utils::StringStream ss;
        ss << "no matching overload for " << CallSignature(name, args) << std::endl;
        if (!candidates.IsEmpty()) {
            ss << std::endl
               << candidates.Length() << " candidate operator"
               << (candidates.Length() > 1 ? "s:" : ":") << std::endl;
            PrintCandidates(ss, candidates, name);
        }
        builder.Diagnostics().add_error(diag::System::Resolver, ss.str(), source);
    };

    // Resolve the intrinsic overload
    auto match = MatchIntrinsic(kUnaryOperators[intrinsic_index], intrinsic_name, args,
                                earliest_eval_stage, TemplateState{}, on_no_match);
    if (!match.overload) {
        return {};
    }

    return UnaryOperator{
        match.return_type,
        match.parameters[0].type,
        match.overload->const_eval_fn,
    };
}

IntrinsicTable::BinaryOperator Impl::Lookup(ast::BinaryOp op,
                                            const type::Type* lhs,
                                            const type::Type* rhs,
                                            sem::EvaluationStage earliest_eval_stage,
                                            const Source& source,
                                            bool is_compound) {
    auto [intrinsic_index, intrinsic_name] = [&]() -> std::pair<size_t, const char*> {
        switch (op) {
            case ast::BinaryOp::kAnd:
                return {kBinaryOperatorAnd, is_compound ? "operator &= " : "operator & "};
            case ast::BinaryOp::kOr:
                return {kBinaryOperatorOr, is_compound ? "operator |= " : "operator | "};
            case ast::BinaryOp::kXor:
                return {kBinaryOperatorXor, is_compound ? "operator ^= " : "operator ^ "};
            case ast::BinaryOp::kLogicalAnd:
                return {kBinaryOperatorLogicalAnd, "operator && "};
            case ast::BinaryOp::kLogicalOr:
                return {kBinaryOperatorLogicalOr, "operator || "};
            case ast::BinaryOp::kEqual:
                return {kBinaryOperatorEqual, "operator == "};
            case ast::BinaryOp::kNotEqual:
                return {kBinaryOperatorNotEqual, "operator != "};
            case ast::BinaryOp::kLessThan:
                return {kBinaryOperatorLessThan, "operator < "};
            case ast::BinaryOp::kGreaterThan:
                return {kBinaryOperatorGreaterThan, "operator > "};
            case ast::BinaryOp::kLessThanEqual:
                return {kBinaryOperatorLessThanEqual, "operator <= "};
            case ast::BinaryOp::kGreaterThanEqual:
                return {kBinaryOperatorGreaterThanEqual, "operator >= "};
            case ast::BinaryOp::kShiftLeft:
                return {kBinaryOperatorShiftLeft, is_compound ? "operator <<= " : "operator << "};
            case ast::BinaryOp::kShiftRight:
                return {kBinaryOperatorShiftRight, is_compound ? "operator >>= " : "operator >> "};
            case ast::BinaryOp::kAdd:
                return {kBinaryOperatorPlus, is_compound ? "operator += " : "operator + "};
            case ast::BinaryOp::kSubtract:
                return {kBinaryOperatorMinus, is_compound ? "operator -= " : "operator - "};
            case ast::BinaryOp::kMultiply:
                return {kBinaryOperatorStar, is_compound ? "operator *= " : "operator * "};
            case ast::BinaryOp::kDivide:
                return {kBinaryOperatorDivide, is_compound ? "operator /= " : "operator / "};
            case ast::BinaryOp::kModulo:
                return {kBinaryOperatorModulo, is_compound ? "operator %= " : "operator % "};
            default:
                return {0, "<unknown>"};
        }
    }();

    utils::Vector args{lhs, rhs};

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&, name = intrinsic_name](utils::VectorRef<Candidate> candidates) {
        utils::StringStream ss;
        ss << "no matching overload for " << CallSignature(name, args) << std::endl;
        if (!candidates.IsEmpty()) {
            ss << std::endl
               << candidates.Length() << " candidate operator"
               << (candidates.Length() > 1 ? "s:" : ":") << std::endl;
            PrintCandidates(ss, candidates, name);
        }
        builder.Diagnostics().add_error(diag::System::Resolver, ss.str(), source);
    };

    // Resolve the intrinsic overload
    auto match = MatchIntrinsic(kBinaryOperators[intrinsic_index], intrinsic_name, args,
                                earliest_eval_stage, TemplateState{}, on_no_match);
    if (!match.overload) {
        return {};
    }

    return BinaryOperator{
        match.return_type,
        match.parameters[0].type,
        match.parameters[1].type,
        match.overload->const_eval_fn,
    };
}

IntrinsicTable::CtorOrConv Impl::Lookup(CtorConvIntrinsic type,
                                        const type::Type* template_arg,
                                        utils::VectorRef<const type::Type*> args,
                                        sem::EvaluationStage earliest_eval_stage,
                                        const Source& source) {
    auto name = str(type);

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&](utils::VectorRef<Candidate> candidates) {
        utils::StringStream ss;
        ss << "no matching constructor for " << CallSignature(name, args, template_arg)
           << std::endl;
        Candidates ctor, conv;
        for (auto candidate : candidates) {
            if (candidate.overload->flags.Contains(OverloadFlag::kIsConstructor)) {
                ctor.Push(candidate);
            } else {
                conv.Push(candidate);
            }
        }
        if (!ctor.IsEmpty()) {
            ss << std::endl
               << ctor.Length() << " candidate constructor" << (ctor.Length() > 1 ? "s:" : ":")
               << std::endl;
            PrintCandidates(ss, ctor, name);
        }
        if (!conv.IsEmpty()) {
            ss << std::endl
               << conv.Length() << " candidate conversion" << (conv.Length() > 1 ? "s:" : ":")
               << std::endl;
            PrintCandidates(ss, conv, name);
        }
        builder.Diagnostics().add_error(diag::System::Resolver, ss.str(), source);
    };

    // If a template type was provided, then close the 0'th type with this.
    TemplateState templates;
    if (template_arg) {
        templates.Type(0, template_arg);
    }

    // Resolve the intrinsic overload
    auto match = MatchIntrinsic(kConstructorsAndConverters[static_cast<size_t>(type)], name, args,
                                earliest_eval_stage, templates, on_no_match);
    if (!match.overload) {
        return {};
    }

    // Was this overload a constructor or conversion?
    if (match.overload->flags.Contains(OverloadFlag::kIsConstructor)) {
        utils::Vector<sem::Parameter*, 8> params;
        params.Reserve(match.parameters.Length());
        for (auto& p : match.parameters) {
            params.Push(builder.create<sem::Parameter>(
                nullptr, static_cast<uint32_t>(params.Length()), p.type,
                builtin::AddressSpace::kUndefined, builtin::Access::kUndefined, p.usage));
        }
        auto eval_stage = match.overload->const_eval_fn ? sem::EvaluationStage::kConstant
                                                        : sem::EvaluationStage::kRuntime;
        auto* target = constructors.GetOrCreate(match, [&]() {
            return builder.create<sem::ValueConstructor>(match.return_type, std::move(params),
                                                         eval_stage);
        });
        return CtorOrConv{target, match.overload->const_eval_fn};
    }

    // Conversion.
    auto* target = converters.GetOrCreate(match, [&]() {
        auto param = builder.create<sem::Parameter>(
            nullptr, 0u, match.parameters[0].type, builtin::AddressSpace::kUndefined,
            builtin::Access::kUndefined, match.parameters[0].usage);
        auto eval_stage = match.overload->const_eval_fn ? sem::EvaluationStage::kConstant
                                                        : sem::EvaluationStage::kRuntime;
        return builder.create<sem::ValueConversion>(match.return_type, param, eval_stage);
    });
    return CtorOrConv{target, match.overload->const_eval_fn};
}

IntrinsicPrototype Impl::MatchIntrinsic(const IntrinsicInfo& intrinsic,
                                        const char* intrinsic_name,
                                        utils::VectorRef<const type::Type*> args,
                                        sem::EvaluationStage earliest_eval_stage,
                                        TemplateState templates,
                                        const OnNoMatch& on_no_match) const {
    size_t num_matched = 0;
    size_t match_idx = 0;
    utils::Vector<Candidate, kNumFixedCandidates> candidates;
    candidates.Reserve(intrinsic.num_overloads);
    for (size_t overload_idx = 0; overload_idx < static_cast<size_t>(intrinsic.num_overloads);
         overload_idx++) {
        auto candidate =
            ScoreOverload(&intrinsic.overloads[overload_idx], args, earliest_eval_stage, templates);
        if (candidate.score == 0) {
            match_idx = overload_idx;
            num_matched++;
        }
        candidates.Push(std::move(candidate));
    }

    // How many candidates matched?
    if (num_matched == 0) {
        // Sort the candidates with the most promising first
        SortCandidates(candidates);
        on_no_match(std::move(candidates));
        return {};
    }

    Candidate match;

    if (num_matched == 1) {
        match = std::move(candidates[match_idx]);
    } else {
        match = ResolveCandidate(std::move(candidates), intrinsic_name, args, std::move(templates));
        if (!match.overload) {
            // Ambiguous overload. ResolveCandidate() will have already raised an error diagnostic.
            return {};
        }
    }

    // Build the return type
    const type::Type* return_type = nullptr;
    if (auto* indices = match.overload->return_matcher_indices) {
        Any any;
        return_type =
            Match(match.templates, match.overload, indices, earliest_eval_stage).Type(&any);
        if (TINT_UNLIKELY(!return_type)) {
            TINT_ICE(Resolver, builder.Diagnostics()) << "MatchState.Match() returned null";
            return {};
        }
    } else {
        return_type = builder.create<type::Void>();
    }

    return IntrinsicPrototype{match.overload, return_type, std::move(match.parameters)};
}

Impl::Candidate Impl::ScoreOverload(const OverloadInfo* overload,
                                    utils::VectorRef<const type::Type*> args,
                                    sem::EvaluationStage earliest_eval_stage,
                                    const TemplateState& in_templates) const {
    // Penalty weights for overload mismatching.
    // This scoring is used to order the suggested overloads in diagnostic on overload mismatch, and
    // has no impact for a correct program.
    // The overloads with the lowest score will be displayed first (top-most).
    constexpr int kMismatchedParamCountPenalty = 3;
    constexpr int kMismatchedParamTypePenalty = 2;
    constexpr int kMismatchedTemplateCountPenalty = 1;
    constexpr int kMismatchedTemplateTypePenalty = 1;
    constexpr int kMismatchedTemplateNumberPenalty = 1;

    size_t num_parameters = static_cast<size_t>(overload->num_parameters);
    size_t num_arguments = static_cast<size_t>(args.Length());

    size_t score = 0;

    if (num_parameters != num_arguments) {
        score += kMismatchedParamCountPenalty * (std::max(num_parameters, num_arguments) -
                                                 std::min(num_parameters, num_arguments));
    }

    if (score == 0) {
        // Check that all of the template arguments provided are actually expected by the overload.
        size_t expected_templates = overload->num_template_types + overload->num_template_numbers;
        size_t provided_templates = in_templates.Count();
        if (provided_templates > expected_templates) {
            score += kMismatchedTemplateCountPenalty * (provided_templates - expected_templates);
        }
    }

    // Make a mutable copy of the input templates so we can implicitly match more templated
    // arguments.
    TemplateState templates(in_templates);

    // Invoke the matchers for each parameter <-> argument pair.
    // If any arguments cannot be matched, then `score` will be increased.
    // If the overload has any template types or numbers then these will be set based on the
    // argument types. Template types may be refined by constraining with later argument types. For
    // example calling `F<T>(T, T)` with the argument types (abstract-int, i32) will first set T to
    // abstract-int when matching the first argument, and then constrained down to i32 when matching
    // the second argument.
    // Note that inferred template types are not tested against their matchers at this point.
    auto num_params = std::min(num_parameters, num_arguments);
    for (size_t p = 0; p < num_params; p++) {
        auto& parameter = overload->parameters[p];
        auto* indices = parameter.matcher_indices;
        if (!Match(templates, overload, indices, earliest_eval_stage).Type(args[p]->UnwrapRef())) {
            score += kMismatchedParamTypePenalty;
        }
    }

    if (score == 0) {
        // Check all constrained template types matched their constraint matchers.
        // If the template type *does not* match any of the types in the constraint matcher, then
        // `score` is incremented. If the template type *does* match a type, then the template type
        // is replaced with the first matching type. The order of types in the template matcher is
        // important here, which can be controlled with the [[precedence(N)]] decorations on the
        // types in intrinsics.def.
        for (size_t ot = 0; ot < overload->num_template_types; ot++) {
            auto* matcher_index = &overload->template_types[ot].matcher_index;
            if (*matcher_index != kNoMatcher) {
                if (auto* template_type = templates.Type(ot)) {
                    if (auto* ty = Match(templates, overload, matcher_index, earliest_eval_stage)
                                       .Type(template_type)) {
                        // Template type matched one of the types in the template type's matcher.
                        // Replace the template type with this type.
                        templates.SetType(ot, ty);
                        continue;
                    }
                }
                score += kMismatchedTemplateTypePenalty;
            }
        }
    }

    if (score == 0) {
        // Check all constrained open numbers matched.
        // Unlike template types, numbers are not constrained, so we're just checking that the
        // inferred number matches the constraints on the overload. Increments `score` if the
        // template numbers do not match their constraint matchers.
        for (size_t on = 0; on < overload->num_template_numbers; on++) {
            auto* matcher_index = &overload->template_numbers[on].matcher_index;
            if (*matcher_index != kNoMatcher) {
                auto template_num = templates.Num(on);
                if (!template_num.IsValid() ||
                    !Match(templates, overload, matcher_index, earliest_eval_stage)
                         .Num(template_num)
                         .IsValid()) {
                    score += kMismatchedTemplateNumberPenalty;
                }
            }
        }
    }

    // Now that all the template types have been finalized, we can construct the parameters.
    utils::Vector<IntrinsicPrototype::Parameter, kNumFixedParams> parameters;
    if (score == 0) {
        parameters.Reserve(num_params);
        for (size_t p = 0; p < num_params; p++) {
            auto& parameter = overload->parameters[p];
            auto* indices = parameter.matcher_indices;
            auto* ty =
                Match(templates, overload, indices, earliest_eval_stage).Type(args[p]->UnwrapRef());
            parameters.Emplace(ty, parameter.usage);
        }
    }

    return Candidate{overload, templates, parameters, score};
}

Impl::Candidate Impl::ResolveCandidate(Impl::Candidates&& candidates,
                                       const char* intrinsic_name,
                                       utils::VectorRef<const type::Type*> args,
                                       TemplateState templates) const {
    utils::Vector<uint32_t, kNumFixedParams> best_ranks;
    best_ranks.Resize(args.Length(), 0xffffffff);
    size_t num_matched = 0;
    Candidate* best = nullptr;
    for (auto& candidate : candidates) {
        if (candidate.score > 0) {
            continue;  // Candidate has already been ruled out.
        }
        bool some_won = false;   // An argument ranked less than the 'best' overload's argument
        bool some_lost = false;  // An argument ranked more than the 'best' overload's argument
        for (size_t i = 0; i < args.Length(); i++) {
            auto rank = type::Type::ConversionRank(args[i], candidate.parameters[i].type);
            if (best_ranks[i] > rank) {
                best_ranks[i] = rank;
                some_won = true;
            } else if (best_ranks[i] < rank) {
                some_lost = true;
            }
        }
        // If no arguments of this candidate ranked worse than the previous best candidate, then
        // this candidate becomes the new best candidate.
        // If no arguments of this candidate ranked better than the previous best candidate, then
        // this candidate is removed from the list of matches.
        // If neither of the above apply, then we have two candidates with no clear winner, which
        // results in an ambiguous overload error. In this situation the loop ends with
        // `num_matched > 1`.
        if (some_won) {
            // One or more arguments of this candidate ranked better than the previous best
            // candidate's argument(s).
            num_matched++;
            if (!some_lost) {
                // All arguments were at as-good or better than the previous best.
                if (best) {
                    // Mark the previous best candidate as no longer being in the running, by
                    // setting its score to a non-zero value. We pick 1 as this is the closest to 0
                    // (match) as we can get.
                    best->score = 1;
                    num_matched--;
                }
                // This candidate is the new best.
                best = &candidate;
            }
        } else {
            // No arguments ranked better than the current best.
            // Change the score of this candidate to a non-zero value, so that it's not considered a
            // match.
            candidate.score = 1;
        }
    }

    if (num_matched > 1) {
        // Re-sort the candidates with the most promising first
        SortCandidates(candidates);
        // Raise an error
        ErrAmbiguousOverload(intrinsic_name, args, templates, candidates);
        return {};
    }

    return std::move(*best);
}

MatchState Impl::Match(TemplateState& templates,
                       const OverloadInfo* overload,
                       MatcherIndex const* matcher_indices,
                       sem::EvaluationStage earliest_eval_stage) const {
    return MatchState(builder, templates, matchers, overload, matcher_indices, earliest_eval_stage);
}

void Impl::PrintOverload(utils::StringStream& ss,
                         const OverloadInfo* overload,
                         const char* intrinsic_name) const {
    TemplateState templates;

    // TODO(crbug.com/tint/1730): Use input evaluation stage to output only relevant overloads.
    auto earliest_eval_stage = sem::EvaluationStage::kConstant;

    ss << intrinsic_name;

    bool print_template_type = false;
    if (overload->num_template_types > 0) {
        if (overload->flags.Contains(OverloadFlag::kIsConverter)) {
            // Print for conversions
            // e.g. vec3<T>(vec3<U>) -> vec3<f32>
            print_template_type = true;
        } else if ((overload->num_parameters == 0) &&
                   overload->flags.Contains(OverloadFlag::kIsConstructor)) {
            // Print for constructors with no params
            // e.g. vec2<T>() -> vec2<T>
            print_template_type = true;
        }
    }
    if (print_template_type) {
        ss << "<";
        ss << overload->template_types[0].name;
        ss << ">";
    }
    ss << "(";
    for (size_t p = 0; p < overload->num_parameters; p++) {
        auto& parameter = overload->parameters[p];
        if (p > 0) {
            ss << ", ";
        }
        if (parameter.usage != ParameterUsage::kNone) {
            ss << sem::str(parameter.usage) << ": ";
        }
        auto* indices = parameter.matcher_indices;
        ss << Match(templates, overload, indices, earliest_eval_stage).TypeName();
    }
    ss << ")";
    if (overload->return_matcher_indices) {
        ss << " -> ";
        auto* indices = overload->return_matcher_indices;
        ss << Match(templates, overload, indices, earliest_eval_stage).TypeName();
    }

    bool first = true;
    auto separator = [&] {
        ss << (first ? "  where: " : ", ");
        first = false;
    };
    for (size_t i = 0; i < overload->num_template_types; i++) {
        auto& template_type = overload->template_types[i];
        if (template_type.matcher_index != kNoMatcher) {
            separator();
            ss << template_type.name;
            auto* index = &template_type.matcher_index;
            ss << " is " << Match(templates, overload, index, earliest_eval_stage).TypeName();
        }
    }
    for (size_t i = 0; i < overload->num_template_numbers; i++) {
        auto& template_number = overload->template_numbers[i];
        if (template_number.matcher_index != kNoMatcher) {
            separator();
            ss << template_number.name;
            auto* index = &template_number.matcher_index;
            ss << " is " << Match(templates, overload, index, earliest_eval_stage).NumName();
        }
    }
}

void Impl::PrintCandidates(utils::StringStream& ss,
                           utils::VectorRef<Candidate> candidates,
                           const char* intrinsic_name) const {
    for (auto& candidate : candidates) {
        ss << "  ";
        PrintOverload(ss, candidate.overload, intrinsic_name);
        ss << std::endl;
    }
}

const type::Type* MatchState::Type(const type::Type* ty) {
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
    return matcher->String(this);
}

std::string MatchState::NumName() {
    MatcherIndex matcher_index = *matcher_indices_++;
    auto* matcher = matchers.number[matcher_index];
    return matcher->String(this);
}

void Impl::ErrAmbiguousOverload(const char* intrinsic_name,
                                utils::VectorRef<const type::Type*> args,
                                TemplateState templates,
                                utils::VectorRef<Candidate> candidates) const {
    utils::StringStream ss;
    ss << "ambiguous overload while attempting to match " << intrinsic_name;
    for (size_t i = 0; i < std::numeric_limits<size_t>::max(); i++) {
        if (auto* ty = templates.Type(i)) {
            ss << ((i == 0) ? "<" : ", ") << ty->FriendlyName();
        } else {
            if (i > 0) {
                ss << ">";
            }
            break;
        }
    }
    ss << "(";
    bool first = true;
    for (auto* arg : args) {
        if (!first) {
            ss << ", ";
        }
        first = false;
        ss << arg->FriendlyName();
    }
    ss << "):\n";
    for (auto& candidate : candidates) {
        if (candidate.score == 0) {
            ss << "  ";
            PrintOverload(ss, candidate.overload, intrinsic_name);
            ss << std::endl;
        }
    }
    TINT_ICE(Resolver, builder.Diagnostics()) << ss.str();
}

}  // namespace

std::unique_ptr<IntrinsicTable> IntrinsicTable::Create(ProgramBuilder& builder) {
    return std::make_unique<Impl>(builder);
}

IntrinsicTable::~IntrinsicTable() = default;

}  // namespace tint::resolver

/// TypeInfo for the Any type declared in the anonymous namespace above
TINT_INSTANTIATE_TYPEINFO(tint::resolver::Any);
