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

#include "src/tint/lang/core/intrinsic/table.h"

#include <algorithm>
#include <limits>
#include <utility>

#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/core/intrinsic/core_table_data.h"
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/wgsl/ast/binary_expression.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/pipeline_stage_set.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"
#include "src/tint/lang/wgsl/sem/value_conversion.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::core::intrinsic {

const TableData::Number TableData::Number::any{Number::kAny};
const TableData::Number TableData::Number::invalid{Number::kInvalid};

TableData::Any::Any() : Base(0u, type::Flags{}) {}
TableData::Any::~Any() = default;

bool TableData::Any::Equals(const type::UniqueNode&) const {
    return false;
}

std::string TableData::Any::FriendlyName() const {
    return "<any>";
}

type::Type* TableData::Any::Clone(type::CloneContext&) const {
    return nullptr;
}

namespace {

// Aliases
using Any = TableData::Any;
using Number = TableData::Number;
using MatcherIndex = TableData::MatcherIndex;
using TypeMatcher = TableData::TypeMatcher;
using NumberMatcher = TableData::NumberMatcher;
using MatchState = TableData::MatchState;
using TemplateTypeInfo = TableData::TemplateTypeInfo;
using TemplateNumberInfo = TableData::TemplateNumberInfo;
using ParameterInfo = TableData::ParameterInfo;
using IntrinsicInfo = TableData::IntrinsicInfo;
using OverloadInfo = TableData::OverloadInfo;
using OverloadFlag = TableData::OverloadFlag;
using OverloadFlags = TableData::OverloadFlags;
using TemplateState = TableData::TemplateState;
constexpr const auto kNoMatcher = TableData::kNoMatcher;

/// The Vector `N` template argument value for arrays of parameters.
constexpr const size_t kNumFixedParams = 8;

/// The Vector `N` template argument value for arrays of overload candidates.
constexpr const size_t kNumFixedCandidates = 8;

////////////////////////////////////////////////////////////////////////////////
// Binding functions for use in the generated builtin_table.inl
// TODO(bclayton): See if we can move more of this hand-rolled code to the
// template
////////////////////////////////////////////////////////////////////////////////
using PipelineStage = ast::PipelineStage;

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
            size_t hash = Hash(i.parameters.Length());
            for (auto& p : i.parameters) {
                hash = HashCombine(hash, p.type, p.usage);
            }
            return Hash(hash, i.overload, i.return_type);
        }
    };

    const TableData::OverloadInfo* overload = nullptr;
    type::Type const* return_type = nullptr;
    Vector<Parameter, kNumFixedParams> parameters;
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

/// Impl is the private implementation of the Table interface.
class Impl : public Table {
  public:
    Impl(ProgramBuilder& b, const TableData& d);

    Builtin Lookup(core::Function builtin_type,
                   VectorRef<const type::Type*> args,
                   EvaluationStage earliest_eval_stage,
                   const Source& source) override;

    UnaryOperator Lookup(core::UnaryOp op,
                         const type::Type* arg,
                         EvaluationStage earliest_eval_stage,
                         const Source& source) override;

    BinaryOperator Lookup(core::BinaryOp op,
                          const type::Type* lhs,
                          const type::Type* rhs,
                          EvaluationStage earliest_eval_stage,
                          const Source& source,
                          bool is_compound) override;

    CtorOrConv Lookup(CtorConv type,
                      const type::Type* template_arg,
                      VectorRef<const type::Type*> args,
                      EvaluationStage earliest_eval_stage,
                      const Source& source) override;

  private:
    /// Candidate holds information about an overload evaluated for resolution.
    struct Candidate {
        /// The candidate overload
        const TableData::OverloadInfo* overload;
        /// The template types and numbers
        TemplateState templates;
        /// The parameter types for the candidate overload
        Vector<IntrinsicPrototype::Parameter, kNumFixedParams> parameters;
        /// The match-score of the candidate overload.
        /// A score of zero indicates an exact match.
        /// Non-zero scores are used for diagnostics when no overload matches.
        /// Lower scores are displayed first (top-most).
        size_t score;
    };

    /// A list of candidates
    using Candidates = Vector<Candidate, kNumFixedCandidates>;

    /// Callback function when no overloads match.
    using OnNoMatch = std::function<void(VectorRef<Candidate>)>;

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
                                      VectorRef<const type::Type*> args,
                                      EvaluationStage earliest_eval_stage,
                                      TemplateState templates,
                                      const OnNoMatch& on_no_match) const;

    /// Evaluates the single overload for the provided argument types.
    /// @param overload the overload being considered
    /// @param args the argument types
    /// @param templates initial template state. This may contain explicitly specified template
    ///                  arguments. For example `vec3<f32>()` would have the first template-type
    ///                  template as `f32`.
    /// @returns the evaluated Candidate information.
    Candidate ScoreOverload(const TableData::OverloadInfo* overload,
                            VectorRef<const type::Type*> args,
                            EvaluationStage earliest_eval_stage,
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
                               VectorRef<const type::Type*> args,
                               TemplateState templates) const;

    /// Match constructs a new MatchState
    /// @param templates the template state used for matcher evaluation
    /// @param overload the overload being evaluated
    /// @param matcher_indices pointer to a list of matcher indices
    MatchState Match(TemplateState& templates,
                     const TableData::OverloadInfo* overload,
                     MatcherIndex const* matcher_indices,
                     EvaluationStage earliest_eval_stage) const;

    // Prints the overload for emitting diagnostics
    void PrintOverload(StringStream& ss,
                       const TableData::OverloadInfo* overload,
                       const char* intrinsic_name) const;

    // Prints the list of candidates for emitting diagnostics
    void PrintCandidates(StringStream& ss,
                         VectorRef<Candidate> candidates,
                         const char* intrinsic_name) const;

    /// Raises an error when no overload is a clear winner of overload resolution
    void ErrAmbiguousOverload(const char* intrinsic_name,
                              VectorRef<const type::Type*> args,
                              TemplateState templates,
                              VectorRef<Candidate> candidates) const;

    ProgramBuilder& builder;
    const TableData& data;
    Hashmap<IntrinsicPrototype, sem::Builtin*, 64, IntrinsicPrototype::Hasher> builtins;
    Hashmap<IntrinsicPrototype, sem::ValueConstructor*, 16, IntrinsicPrototype::Hasher>
        constructors;
    Hashmap<IntrinsicPrototype, sem::ValueConversion*, 16, IntrinsicPrototype::Hasher> converters;
};

/// @return a string representing a call to a builtin with the given argument
/// types.
std::string CallSignature(const char* intrinsic_name,
                          VectorRef<const type::Type*> args,
                          const type::Type* template_arg = nullptr) {
    StringStream ss;
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

Impl::Impl(ProgramBuilder& b, const TableData& d) : builder(b), data(d) {}

Impl::Builtin Impl::Lookup(core::Function builtin_type,
                           VectorRef<const type::Type*> args,
                           EvaluationStage earliest_eval_stage,
                           const Source& source) {
    const char* intrinsic_name = core::str(builtin_type);

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&](VectorRef<Candidate> candidates) {
        StringStream ss;
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
    auto match = MatchIntrinsic(data.builtins[static_cast<size_t>(builtin_type)], intrinsic_name,
                                args, earliest_eval_stage, TemplateState{}, on_no_match);
    if (!match.overload) {
        return {};
    }

    // De-duplicate builtins that are identical.
    auto* sem = builtins.GetOrCreate(match, [&] {
        Vector<sem::Parameter*, kNumFixedParams> params;
        params.Reserve(match.parameters.Length());
        for (auto& p : match.parameters) {
            params.Push(builder.create<sem::Parameter>(
                nullptr, static_cast<uint32_t>(params.Length()), p.type,
                core::AddressSpace::kUndefined, core::Access::kUndefined, p.usage));
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
        auto eval_stage =
            overload.const_eval_fn ? EvaluationStage::kConstant : EvaluationStage::kRuntime;
        return builder.create<sem::Builtin>(builtin_type, match.return_type, std::move(params),
                                            eval_stage, supported_stages,
                                            overload.flags.Contains(OverloadFlag::kIsDeprecated),
                                            overload.flags.Contains(OverloadFlag::kMustUse));
    });
    return Builtin{sem, match.overload->const_eval_fn};
}

Table::UnaryOperator Impl::Lookup(core::UnaryOp op,
                                  const type::Type* arg,
                                  EvaluationStage earliest_eval_stage,
                                  const Source& source) {
    auto [intrinsic_info, intrinsic_name] = [&]() -> std::pair<const IntrinsicInfo*, const char*> {
        switch (op) {
            case core::UnaryOp::kComplement:
                return {&data.unary_complement, "operator ~ "};
            case core::UnaryOp::kNegation:
                return {&data.unary_minus, "operator - "};
            case core::UnaryOp::kNot:
                return {&data.unary_not, "operator ! "};
            default:
                break;
        }
        TINT_UNREACHABLE() << "invalid unary op: " << op;
        return {};
    }();
    if (!intrinsic_info) {
        return {};
    }

    Vector args{arg};

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&, name = intrinsic_name](VectorRef<Candidate> candidates) {
        StringStream ss;
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
    auto match = MatchIntrinsic(*intrinsic_info, intrinsic_name, args, earliest_eval_stage,
                                TemplateState{}, on_no_match);
    if (!match.overload) {
        return {};
    }

    return UnaryOperator{
        match.return_type,
        match.parameters[0].type,
        match.overload->const_eval_fn,
    };
}

Table::BinaryOperator Impl::Lookup(core::BinaryOp op,
                                   const type::Type* lhs,
                                   const type::Type* rhs,
                                   EvaluationStage earliest_eval_stage,
                                   const Source& source,
                                   bool is_compound) {
    auto [intrinsic_info, intrinsic_name] = [&]() -> std::pair<const IntrinsicInfo*, const char*> {
        switch (op) {
            case core::BinaryOp::kAnd:
                return {&data.binary_and, is_compound ? "operator &= " : "operator & "};
            case core::BinaryOp::kOr:
                return {&data.binary_or, is_compound ? "operator |= " : "operator | "};
            case core::BinaryOp::kXor:
                return {&data.binary_xor, is_compound ? "operator ^= " : "operator ^ "};
            case core::BinaryOp::kLogicalAnd:
                return {&data.binary_logical_and, "operator && "};
            case core::BinaryOp::kLogicalOr:
                return {&data.binary_logical_or, "operator || "};
            case core::BinaryOp::kEqual:
                return {&data.binary_equal, "operator == "};
            case core::BinaryOp::kNotEqual:
                return {&data.binary_not_equal, "operator != "};
            case core::BinaryOp::kLessThan:
                return {&data.binary_less_than, "operator < "};
            case core::BinaryOp::kGreaterThan:
                return {&data.binary_greater_than, "operator > "};
            case core::BinaryOp::kLessThanEqual:
                return {&data.binary_less_than_equal, "operator <= "};
            case core::BinaryOp::kGreaterThanEqual:
                return {&data.binary_greater_than_equal, "operator >= "};
            case core::BinaryOp::kShiftLeft:
                return {&data.binary_shift_left, is_compound ? "operator <<= " : "operator << "};
            case core::BinaryOp::kShiftRight:
                return {&data.binary_shift_right, is_compound ? "operator >>= " : "operator >> "};
            case core::BinaryOp::kAdd:
                return {&data.binary_plus, is_compound ? "operator += " : "operator + "};
            case core::BinaryOp::kSubtract:
                return {&data.binary_minus, is_compound ? "operator -= " : "operator - "};
            case core::BinaryOp::kMultiply:
                return {&data.binary_star, is_compound ? "operator *= " : "operator * "};
            case core::BinaryOp::kDivide:
                return {&data.binary_divide, is_compound ? "operator /= " : "operator / "};
            case core::BinaryOp::kModulo:
                return {&data.binary_modulo, is_compound ? "operator %= " : "operator % "};
        }
        TINT_UNREACHABLE() << "unhandled BinaryOp: " << op;
        return {};
    }();
    if (!intrinsic_info) {
        return {};
    }

    Vector args{lhs, rhs};

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&, name = intrinsic_name](VectorRef<Candidate> candidates) {
        StringStream ss;
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
    auto match = MatchIntrinsic(*intrinsic_info, intrinsic_name, args, earliest_eval_stage,
                                TemplateState{}, on_no_match);
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

Table::CtorOrConv Impl::Lookup(CtorConv type,
                               const type::Type* template_arg,
                               VectorRef<const type::Type*> args,
                               EvaluationStage earliest_eval_stage,
                               const Source& source) {
    auto name = str(type);

    // Generates an error when no overloads match the provided arguments
    auto on_no_match = [&](VectorRef<Candidate> candidates) {
        StringStream ss;
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
    auto match = MatchIntrinsic(data.ctor_conv[static_cast<size_t>(type)], name, args,
                                earliest_eval_stage, templates, on_no_match);
    if (!match.overload) {
        return {};
    }

    // Was this overload a constructor or conversion?
    if (match.overload->flags.Contains(OverloadFlag::kIsConstructor)) {
        Vector<sem::Parameter*, 8> params;
        params.Reserve(match.parameters.Length());
        for (auto& p : match.parameters) {
            params.Push(builder.create<sem::Parameter>(
                nullptr, static_cast<uint32_t>(params.Length()), p.type,
                core::AddressSpace::kUndefined, core::Access::kUndefined, p.usage));
        }
        auto eval_stage =
            match.overload->const_eval_fn ? EvaluationStage::kConstant : EvaluationStage::kRuntime;
        auto* target = constructors.GetOrCreate(match, [&] {
            return builder.create<sem::ValueConstructor>(match.return_type, std::move(params),
                                                         eval_stage);
        });
        return CtorOrConv{target, match.overload->const_eval_fn};
    }

    // Conversion.
    auto* target = converters.GetOrCreate(match, [&] {
        auto param = builder.create<sem::Parameter>(
            nullptr, 0u, match.parameters[0].type, core::AddressSpace::kUndefined,
            core::Access::kUndefined, match.parameters[0].usage);
        auto eval_stage =
            match.overload->const_eval_fn ? EvaluationStage::kConstant : EvaluationStage::kRuntime;
        return builder.create<sem::ValueConversion>(match.return_type, param, eval_stage);
    });
    return CtorOrConv{target, match.overload->const_eval_fn};
}

IntrinsicPrototype Impl::MatchIntrinsic(const IntrinsicInfo& intrinsic,
                                        const char* intrinsic_name,
                                        VectorRef<const type::Type*> args,
                                        EvaluationStage earliest_eval_stage,
                                        TemplateState templates,
                                        const OnNoMatch& on_no_match) const {
    size_t num_matched = 0;
    size_t match_idx = 0;
    Vector<Candidate, kNumFixedCandidates> candidates;
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
            TINT_ICE() << "MatchState.Match() returned null";
            return {};
        }
    } else {
        return_type = builder.create<type::Void>();
    }

    return IntrinsicPrototype{match.overload, return_type, std::move(match.parameters)};
}

Impl::Candidate Impl::ScoreOverload(const TableData::OverloadInfo* overload,
                                    VectorRef<const type::Type*> args,
                                    EvaluationStage earliest_eval_stage,
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
    Vector<IntrinsicPrototype::Parameter, kNumFixedParams> parameters;
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
                                       VectorRef<const type::Type*> args,
                                       TemplateState templates) const {
    Vector<uint32_t, kNumFixedParams> best_ranks;
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
                       const TableData::OverloadInfo* overload,
                       MatcherIndex const* matcher_indices,
                       EvaluationStage earliest_eval_stage) const {
    return MatchState{builder.Types(), builder.Symbols(), templates,          data,
                      overload,        matcher_indices,   earliest_eval_stage};
}

void Impl::PrintOverload(StringStream& ss,
                         const TableData::OverloadInfo* overload,
                         const char* intrinsic_name) const {
    TemplateState templates;

    // TODO(crbug.com/tint/1730): Use input evaluation stage to output only relevant overloads.
    auto earliest_eval_stage = EvaluationStage::kConstant;

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
            ss << ToString(parameter.usage) << ": ";
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

void Impl::PrintCandidates(StringStream& ss,
                           VectorRef<Candidate> candidates,
                           const char* intrinsic_name) const {
    for (auto& candidate : candidates) {
        ss << "  ";
        PrintOverload(ss, candidate.overload, intrinsic_name);
        ss << std::endl;
    }
}

void Impl::ErrAmbiguousOverload(const char* intrinsic_name,
                                VectorRef<const type::Type*> args,
                                TemplateState templates,
                                VectorRef<Candidate> candidates) const {
    StringStream ss;
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
    TINT_ICE() << ss.str();
}

}  // namespace

std::unique_ptr<Table> Table::Create(ProgramBuilder& builder) {
    return std::make_unique<Impl>(builder, CoreTableData());
}

Table::~Table() = default;

}  // namespace tint::core::intrinsic

/// TypeInfo for the Any type declared in the anonymous namespace above
TINT_INSTANTIATE_TYPEINFO(tint::core::intrinsic::Any);
