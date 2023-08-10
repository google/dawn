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
#include "src/tint/lang/core/intrinsic/table_data.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/void.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/macros/scoped_assignment.h"
#include "src/tint/utils/math/hash.h"
#include "src/tint/utils/math/math.h"
#include "src/tint/utils/rtti/switch.h"
#include "src/tint/utils/text/string_stream.h"

namespace tint::core::intrinsic {

const Number Number::any{Number::kAny};
const Number Number::invalid{Number::kInvalid};

Any::Any() : Base(0u, core::type::Flags{}) {}
Any::~Any() = default;

bool Any::Equals(const core::type::UniqueNode&) const {
    return false;
}

std::string Any::FriendlyName() const {
    return "<any>";
}

core::type::Type* Any::Clone(core::type::CloneContext&) const {
    return nullptr;
}

namespace {

/// The Vector `N` template argument value for arrays of parameters.
constexpr const size_t kNumFixedParams = decltype(Table::Overload{}.parameters)::static_length;

/// The Vector `N` template argument value for arrays of overload candidates.
constexpr const size_t kNumFixedCandidates = 8;

/// Impl is the private implementation of the Table interface.
class Impl : public Table {
  public:
    Impl(const TableData& td, core::type::Manager& tys, SymbolTable& syms, diag::List& d);

    Result<Overload> Lookup(core::Function builtin_type,
                            VectorRef<const core::type::Type*> args,
                            EvaluationStage earliest_eval_stage,
                            const Source& source) override;

    Result<Overload> Lookup(core::UnaryOp op,
                            const core::type::Type* arg,
                            EvaluationStage earliest_eval_stage,
                            const Source& source) override;

    Result<Overload> Lookup(core::BinaryOp op,
                            const core::type::Type* lhs,
                            const core::type::Type* rhs,
                            EvaluationStage earliest_eval_stage,
                            const Source& source,
                            bool is_compound) override;

    Result<Overload> Lookup(CtorConv type,
                            const core::type::Type* template_arg,
                            VectorRef<const core::type::Type*> args,
                            EvaluationStage earliest_eval_stage,
                            const Source& source) override;

  private:
    /// Candidate holds information about an overload evaluated for resolution.
    struct Candidate {
        /// The candidate overload
        const OverloadInfo* overload;
        /// The template types and numbers
        TemplateState templates;
        /// The parameter types for the candidate overload
        Vector<Table::Overload::Parameter, kNumFixedParams> parameters;
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
    /// @returns the matched intrinsic
    Result<Table::Overload> MatchIntrinsic(const IntrinsicInfo& intrinsic,
                                           const char* intrinsic_name,
                                           VectorRef<const core::type::Type*> args,
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
    Candidate ScoreOverload(const OverloadInfo& overload,
                            VectorRef<const core::type::Type*> args,
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
                               VectorRef<const core::type::Type*> args,
                               TemplateState templates) const;

    /// Match constructs a new MatchState
    /// @param templates the template state used for matcher evaluation
    /// @param overload the overload being evaluated
    /// @param type_matcher_indices pointer to a list of type matcher indices
    /// @param number_matcher_indices pointer to a list of number matcher indices
    MatchState Match(TemplateState& templates,
                     const OverloadInfo& overload,
                     const TypeMatcherIndex* type_matcher_indices,
                     const NumberMatcherIndex* number_matcher_indices,
                     EvaluationStage earliest_eval_stage) const;

    // Prints the overload for emitting diagnostics
    void PrintOverload(StringStream& ss,
                       const OverloadInfo& overload,
                       const char* intrinsic_name) const;

    // Prints the list of candidates for emitting diagnostics
    void PrintCandidates(StringStream& ss,
                         VectorRef<Candidate> candidates,
                         const char* intrinsic_name) const;

    /// Raises an error when no overload is a clear winner of overload resolution
    void ErrAmbiguousOverload(const char* intrinsic_name,
                              VectorRef<const core::type::Type*> args,
                              TemplateState templates,
                              VectorRef<Candidate> candidates) const;

    const TableData& data;
    core::type::Manager& types;
    SymbolTable& symbols;
    diag::List& diags;
};

/// @return a string representing a call to a builtin with the given argument
/// types.
std::string CallSignature(const char* intrinsic_name,
                          VectorRef<const core::type::Type*> args,
                          const core::type::Type* template_arg = nullptr) {
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

Impl::Impl(const TableData& td, core::type::Manager& tys, SymbolTable& syms, diag::List& d)
    : data(td), types(tys), symbols(syms), diags(d) {}

Result<Table::Overload> Impl::Lookup(core::Function builtin_type,
                                     VectorRef<const core::type::Type*> args,
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
        diags.add_error(diag::System::Intrinsics, ss.str(), source);
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(data.builtins[static_cast<size_t>(builtin_type)], intrinsic_name, args,
                          earliest_eval_stage, TemplateState{}, on_no_match);
}

Result<Table::Overload> Impl::Lookup(core::UnaryOp op,
                                     const core::type::Type* arg,
                                     EvaluationStage earliest_eval_stage,
                                     const Source& source) {
    const IntrinsicInfo* intrinsic_info = nullptr;
    const char* intrinsic_name = nullptr;
    switch (op) {
        case core::UnaryOp::kComplement:
            intrinsic_info = &data.unary_complement;
            intrinsic_name = "operator ~ ";
            break;
        case core::UnaryOp::kNegation:
            intrinsic_info = &data.unary_minus;
            intrinsic_name = "operator - ";
            break;
        case core::UnaryOp::kNot:
            intrinsic_info = &data.unary_not;
            intrinsic_name = "operator ! ";
            break;
        default:
            TINT_UNREACHABLE() << "invalid unary op: " << op;
            return Failure;
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
        diags.add_error(diag::System::Intrinsics, ss.str(), source);
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(*intrinsic_info, intrinsic_name, args, earliest_eval_stage,
                          TemplateState{}, on_no_match);
}

Result<Table::Overload> Impl::Lookup(core::BinaryOp op,
                                     const core::type::Type* lhs,
                                     const core::type::Type* rhs,
                                     EvaluationStage earliest_eval_stage,
                                     const Source& source,
                                     bool is_compound) {
    const IntrinsicInfo* intrinsic_info = nullptr;
    const char* intrinsic_name = nullptr;
    switch (op) {
        case core::BinaryOp::kAnd:
            intrinsic_info = &data.binary_and;
            intrinsic_name = is_compound ? "operator &= " : "operator & ";
            break;
        case core::BinaryOp::kOr:
            intrinsic_info = &data.binary_or;
            intrinsic_name = is_compound ? "operator |= " : "operator | ";
            break;
        case core::BinaryOp::kXor:
            intrinsic_info = &data.binary_xor;
            intrinsic_name = is_compound ? "operator ^= " : "operator ^ ";
            break;
        case core::BinaryOp::kLogicalAnd:
            intrinsic_info = &data.binary_logical_and;
            intrinsic_name = "operator && ";
            break;
        case core::BinaryOp::kLogicalOr:
            intrinsic_info = &data.binary_logical_or;
            intrinsic_name = "operator || ";
            break;
        case core::BinaryOp::kEqual:
            intrinsic_info = &data.binary_equal;
            intrinsic_name = "operator == ";
            break;
        case core::BinaryOp::kNotEqual:
            intrinsic_info = &data.binary_not_equal;
            intrinsic_name = "operator != ";
            break;
        case core::BinaryOp::kLessThan:
            intrinsic_info = &data.binary_less_than;
            intrinsic_name = "operator < ";
            break;
        case core::BinaryOp::kGreaterThan:
            intrinsic_info = &data.binary_greater_than;
            intrinsic_name = "operator > ";
            break;
        case core::BinaryOp::kLessThanEqual:
            intrinsic_info = &data.binary_less_than_equal;
            intrinsic_name = "operator <= ";
            break;
        case core::BinaryOp::kGreaterThanEqual:
            intrinsic_info = &data.binary_greater_than_equal;
            intrinsic_name = "operator >= ";
            break;
        case core::BinaryOp::kShiftLeft:
            intrinsic_info = &data.binary_shift_left;
            intrinsic_name = is_compound ? "operator <<= " : "operator << ";
            break;
        case core::BinaryOp::kShiftRight:
            intrinsic_info = &data.binary_shift_right;
            intrinsic_name = is_compound ? "operator >>= " : "operator >> ";
            break;
        case core::BinaryOp::kAdd:
            intrinsic_info = &data.binary_plus;
            intrinsic_name = is_compound ? "operator += " : "operator + ";
            break;
        case core::BinaryOp::kSubtract:
            intrinsic_info = &data.binary_minus;
            intrinsic_name = is_compound ? "operator -= " : "operator - ";
            break;
        case core::BinaryOp::kMultiply:
            intrinsic_info = &data.binary_star;
            intrinsic_name = is_compound ? "operator *= " : "operator * ";
            break;
        case core::BinaryOp::kDivide:
            intrinsic_info = &data.binary_divide;
            intrinsic_name = is_compound ? "operator /= " : "operator / ";
            break;
        case core::BinaryOp::kModulo:
            intrinsic_info = &data.binary_modulo;
            intrinsic_name = is_compound ? "operator %= " : "operator % ";
            break;
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
        diags.add_error(diag::System::Intrinsics, ss.str(), source);
    };

    // Resolve the intrinsic overload
    return MatchIntrinsic(*intrinsic_info, intrinsic_name, args, earliest_eval_stage,
                          TemplateState{}, on_no_match);
}

Result<Table::Overload> Impl::Lookup(CtorConv type,
                                     const core::type::Type* template_arg,
                                     VectorRef<const core::type::Type*> args,
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
        diags.add_error(diag::System::Intrinsics, ss.str(), source);
    };

    // If a template type was provided, then close the 0'th type with this.
    TemplateState templates;
    if (template_arg) {
        templates.Type(0, template_arg);
    }

    // Resolve the intrinsic overload
    return MatchIntrinsic(data.ctor_conv[static_cast<size_t>(type)], name, args,
                          earliest_eval_stage, templates, on_no_match);
}

Result<Table::Overload> Impl::MatchIntrinsic(const IntrinsicInfo& intrinsic,
                                             const char* intrinsic_name,
                                             VectorRef<const core::type::Type*> args,
                                             EvaluationStage earliest_eval_stage,
                                             TemplateState templates,
                                             const OnNoMatch& on_no_match) const {
    size_t num_matched = 0;
    size_t match_idx = 0;
    Vector<Candidate, kNumFixedCandidates> candidates;
    candidates.Reserve(intrinsic.num_overloads);
    for (size_t overload_idx = 0; overload_idx < static_cast<size_t>(intrinsic.num_overloads);
         overload_idx++) {
        auto& overload = data[intrinsic.overloads + overload_idx];
        auto candidate = ScoreOverload(overload, args, earliest_eval_stage, templates);
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
        return Failure;
    }

    Candidate match;

    if (num_matched == 1) {
        match = std::move(candidates[match_idx]);
    } else {
        match = ResolveCandidate(std::move(candidates), intrinsic_name, args, std::move(templates));
        if (!match.overload) {
            // Ambiguous overload. ResolveCandidate() will have already raised an error diagnostic.
            return Failure;
        }
    }

    // Build the return type
    const core::type::Type* return_type = nullptr;
    if (auto* type_indices = data[match.overload->return_type_matcher_indices]) {
        auto* number_indices = data[match.overload->return_number_matcher_indices];
        Any any;
        return_type = Match(match.templates, *match.overload, type_indices, number_indices,
                            earliest_eval_stage)
                          .Type(&any);
        if (TINT_UNLIKELY(!return_type)) {
            TINT_ICE() << "MatchState.Match() returned null";
            return Failure;
        }
    } else {
        return_type = types.void_();
    }

    return Table::Overload{match.overload, return_type, std::move(match.parameters),
                           data[match.overload->const_eval_fn]};
}

Impl::Candidate Impl::ScoreOverload(const OverloadInfo& overload,
                                    VectorRef<const core::type::Type*> args,
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

    size_t num_parameters = static_cast<size_t>(overload.num_parameters);
    size_t num_arguments = static_cast<size_t>(args.Length());

    size_t score = 0;

    if (num_parameters != num_arguments) {
        score += kMismatchedParamCountPenalty * (std::max(num_parameters, num_arguments) -
                                                 std::min(num_parameters, num_arguments));
    }

    if (score == 0) {
        // Check that all of the template arguments provided are actually expected by the overload.
        size_t expected_templates = overload.num_template_types + overload.num_template_numbers;
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
        auto& parameter = data[overload.parameters + p];
        auto* type_indices = data[parameter.type_matcher_indices];
        auto* number_indices = data[parameter.number_matcher_indices];
        if (!Match(templates, overload, type_indices, number_indices, earliest_eval_stage)
                 .Type(args[p]->UnwrapRef())) {
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
        for (size_t ot = 0; ot < overload.num_template_types; ot++) {
            auto* matcher_idx = &data[overload.template_types + ot].matcher_index;
            if (matcher_idx->IsValid()) {
                if (auto* type = templates.Type(ot)) {
                    if (auto* ty =
                            Match(templates, overload, matcher_idx, nullptr, earliest_eval_stage)
                                .Type(type)) {
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
        for (size_t on = 0; on < overload.num_template_numbers; on++) {
            auto* matcher_idx = &data[overload.template_numbers + on].matcher_index;
            if (matcher_idx->IsValid()) {
                auto number = templates.Num(on);
                if (!number.IsValid() ||
                    !Match(templates, overload, nullptr, matcher_idx, earliest_eval_stage)
                         .Num(number)
                         .IsValid()) {
                    score += kMismatchedTemplateNumberPenalty;
                }
            }
        }
    }

    // Now that all the template types have been finalized, we can construct the parameters.
    Vector<Table::Overload::Parameter, kNumFixedParams> parameters;
    if (score == 0) {
        parameters.Reserve(num_params);
        for (size_t p = 0; p < num_params; p++) {
            auto& parameter = data[overload.parameters + p];
            auto* type_indices = data[parameter.type_matcher_indices];
            auto* number_indices = data[parameter.number_matcher_indices];
            auto* ty = Match(templates, overload, type_indices, number_indices, earliest_eval_stage)
                           .Type(args[p]->UnwrapRef());
            parameters.Emplace(ty, parameter.usage);
        }
    }

    return Candidate{&overload, templates, parameters, score};
}

Impl::Candidate Impl::ResolveCandidate(Impl::Candidates&& candidates,
                                       const char* intrinsic_name,
                                       VectorRef<const core::type::Type*> args,
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
            auto rank = core::type::Type::ConversionRank(args[i], candidate.parameters[i].type);
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
                       const OverloadInfo& overload,
                       const TypeMatcherIndex* type_matcher_indices,
                       const NumberMatcherIndex* number_matcher_indices,
                       EvaluationStage earliest_eval_stage) const {
    return MatchState{types,
                      symbols,
                      templates,
                      data,
                      overload,
                      type_matcher_indices,
                      number_matcher_indices,
                      earliest_eval_stage};
}

void Impl::PrintOverload(StringStream& ss,
                         const OverloadInfo& overload,
                         const char* intrinsic_name) const {
    TemplateState templates;

    // TODO(crbug.com/tint/1730): Use input evaluation stage to output only relevant overloads.
    auto earliest_eval_stage = EvaluationStage::kConstant;

    ss << intrinsic_name;

    bool print_template_type = false;
    if (overload.num_template_types > 0) {
        if (overload.flags.Contains(OverloadFlag::kIsConverter)) {
            // Print for conversions
            // e.g. vec3<T>(vec3<U>) -> vec3<f32>
            print_template_type = true;
        } else if ((overload.num_parameters == 0) &&
                   overload.flags.Contains(OverloadFlag::kIsConstructor)) {
            // Print for constructors with no params
            // e.g. vec2<T>() -> vec2<T>
            print_template_type = true;
        }
    }
    if (print_template_type) {
        ss << "<";
        ss << data[overload.template_types].name;
        ss << ">";
    }
    ss << "(";
    for (size_t p = 0; p < overload.num_parameters; p++) {
        auto& parameter = data[overload.parameters + p];
        if (p > 0) {
            ss << ", ";
        }
        if (parameter.usage != ParameterUsage::kNone) {
            ss << ToString(parameter.usage) << ": ";
        }
        auto* type_indices = data[parameter.type_matcher_indices];
        auto* number_indices = data[parameter.number_matcher_indices];
        ss << Match(templates, overload, type_indices, number_indices, earliest_eval_stage)
                  .TypeName();
    }
    ss << ")";
    if (overload.return_type_matcher_indices.IsValid()) {
        ss << " -> ";
        auto* type_indices = data[overload.return_type_matcher_indices];
        auto* number_indices = data[overload.return_number_matcher_indices];
        ss << Match(templates, overload, type_indices, number_indices, earliest_eval_stage)
                  .TypeName();
    }

    bool first = true;
    auto separator = [&] {
        ss << (first ? "  where: " : ", ");
        first = false;
    };
    for (size_t i = 0; i < overload.num_template_types; i++) {
        auto& template_type = data[overload.template_types + i];
        if (template_type.matcher_index.IsValid()) {
            separator();
            ss << template_type.name;
            auto* index = &template_type.matcher_index;
            ss << " is "
               << Match(templates, overload, index, nullptr, earliest_eval_stage).TypeName();
        }
    }
    for (size_t i = 0; i < overload.num_template_numbers; i++) {
        auto& template_number = data[overload.template_numbers + i];
        if (template_number.matcher_index.IsValid()) {
            separator();
            ss << template_number.name;
            auto* index = &template_number.matcher_index;
            ss << " is "
               << Match(templates, overload, nullptr, index, earliest_eval_stage).NumName();
        }
    }
}

void Impl::PrintCandidates(StringStream& ss,
                           VectorRef<Candidate> candidates,
                           const char* intrinsic_name) const {
    for (auto& candidate : candidates) {
        ss << "  ";
        PrintOverload(ss, *candidate.overload, intrinsic_name);
        ss << std::endl;
    }
}

void Impl::ErrAmbiguousOverload(const char* intrinsic_name,
                                VectorRef<const core::type::Type*> args,
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
            PrintOverload(ss, *candidate.overload, intrinsic_name);
            ss << std::endl;
        }
    }
    TINT_ICE() << ss.str();
}

}  // namespace

std::unique_ptr<Table> Table::Create(const TableData& data,
                                     core::type::Manager& types,
                                     SymbolTable& symbols,
                                     diag::List& diags) {
    return std::make_unique<Impl>(data, types, symbols, diags);
}

Table::~Table() = default;

}  // namespace tint::core::intrinsic

/// TypeInfo for the Any type declared in the anonymous namespace above
TINT_INSTANTIATE_TYPEINFO(tint::core::intrinsic::Any);
