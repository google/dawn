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

#ifndef SRC_TINT_LANG_CORE_INTRINSIC_TABLE_DATA_H_
#define SRC_TINT_LANG_CORE_INTRINSIC_TABLE_DATA_H_

#include <stdint.h>
#include <limits>
#include <string>

#include "src/tint/lang/core/constant/eval.h"
#include "src/tint/lang/core/evaluation_stage.h"
#include "src/tint/lang/core/parameter_usage.h"
#include "src/tint/utils/containers/enum_set.h"
#include "src/tint/utils/containers/slice.h"

namespace tint::type {
class Manager;
}  // namespace tint::type

namespace tint::core::intrinsic {

/// TableData holds the immutable data that holds the intrinsic data for a language.
struct TableData {
    /// Index type used for matcher indices
    using MatcherIndex = uint8_t;

    /// Index value used for template types / numbers that do not have a constraint
    static constexpr MatcherIndex kNoMatcher = std::numeric_limits<MatcherIndex>::max();

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

    /// An enum set of OverloadFlag, used by OperatorInfo
    using OverloadFlags = tint::EnumSet<OverloadFlag>;

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
        constant::Eval::Function const const_eval_fn;
    };

    /// IntrinsicInfo describes a builtin function or operator overload
    struct IntrinsicInfo {
        /// Number of overloads of the intrinsic
        const uint8_t num_overloads;
        /// Pointer to the start of the overloads for the function
        OverloadInfo const* const overloads;
    };

    /// Number is an 32 bit unsigned integer, which can be in one of three states:
    /// * Invalid - Number has not been assigned a value
    /// * Valid   - a fixed integer value
    /// * Any     - matches any other non-invalid number
    class Number {
        enum State {
            kInvalid,
            kValid,
            kAny,
        };

        constexpr explicit Number(State state) : state_(state) {}

      public:
        /// A special number representing any number
        static const Number any;
        /// An invalid number
        static const Number invalid;

        /// Constructed as a valid number with the value v
        /// @param v the value for the number
        explicit constexpr Number(uint32_t v) : value_(v), state_(kValid) {}

        /// @returns the value of the number
        inline uint32_t Value() const { return value_; }

        /// @returns the true if the number is valid
        inline bool IsValid() const { return state_ == kValid; }

        /// @returns the true if the number is any
        inline bool IsAny() const { return state_ == kAny; }

        /// Assignment operator.
        /// The number becomes valid, with the value n
        /// @param n the new value for the number
        /// @returns this so calls can be chained
        inline Number& operator=(uint32_t n) {
            value_ = n;
            state_ = kValid;
            return *this;
        }

      private:
        uint32_t value_ = 0;
        State state_ = kInvalid;
    };

    /// A special type that matches all TypeMatchers
    class Any final : public Castable<Any, type::Type> {
      public:
        Any();
        ~Any() override;

        /// @copydoc type::UniqueNode::Equals
        bool Equals(const type::UniqueNode& other) const override;
        /// @copydoc type::Type::FriendlyName
        std::string FriendlyName() const override;
        /// @copydoc type::Type::Clone
        type::Type* Clone(type::CloneContext& ctx) const override;
    };

    /// TemplateState holds the state of the template numbers and types.
    /// Used by the MatchState.
    class TemplateState {
      public:
        /// If the template type with index @p idx is undefined, then it is defined with the @p ty
        /// and Type() returns @p ty. If the template type is defined, and @p ty can be converted to
        /// the template type then the template type is returned. If the template type is defined,
        /// and the template type can be converted to @p ty, then the template type is replaced with
        /// @p ty, and @p ty is returned. If none of the above applies, then @p ty is a type
        /// mismatch for the template type, and nullptr is returned.
        /// @param idx the index of the template type
        /// @param ty the type
        /// @returns true on match or newly defined
        const type::Type* Type(size_t idx, const type::Type* ty) {
            if (idx >= types_.Length()) {
                types_.Resize(idx + 1);
            }
            auto& t = types_[idx];
            if (t == nullptr) {
                t = ty;
                return ty;
            }
            ty = type::Type::Common(Vector{t, ty});
            if (ty) {
                t = ty;
            }
            return ty;
        }

        /// If the number with index @p idx is undefined, then it is defined with the number
        /// `number` and Num() returns true. If the number is defined, then `Num()` returns true iff
        /// it is equal to @p ty.
        /// @param idx the index of the template number
        /// @param number the number
        /// @returns true on match or newly defined
        bool Num(size_t idx, TableData::Number number) {
            if (idx >= numbers_.Length()) {
                numbers_.Resize(idx + 1, TableData::Number::invalid);
            }
            auto& n = numbers_[idx];
            if (!n.IsValid()) {
                n = number.Value();
                return true;
            }
            return n.Value() == number.Value();
        }

        /// @param idx the index of the template type
        /// @returns the template type with index @p idx, or nullptr if the type was not
        /// defined.
        const type::Type* Type(size_t idx) const {
            if (idx >= types_.Length()) {
                return nullptr;
            }
            return types_[idx];
        }

        /// SetType replaces the template type with index @p idx with type @p ty.
        /// @param idx the index of the template type
        /// @param ty the new type for the template
        void SetType(size_t idx, const type::Type* ty) {
            if (idx >= types_.Length()) {
                types_.Resize(idx + 1);
            }
            types_[idx] = ty;
        }

        /// @returns the number type with index @p idx.
        /// @param idx the index of the template number
        TableData::Number Num(size_t idx) const {
            if (idx >= numbers_.Length()) {
                return TableData::Number::invalid;
            }
            return numbers_[idx];
        }

        /// @return the total number of type and number templates
        size_t Count() const { return types_.Length() + numbers_.Length(); }

      private:
        Vector<const type::Type*, 4> types_;
        Vector<TableData::Number, 2> numbers_;
    };

    /// The current overload match state
    /// MatchState holds the state used to match an overload.
    class MatchState {
      public:
        /// Constructor
        /// @param ty_mgr the type manager
        /// @param syms the symbol table
        /// @param t the template state
        /// @param d the table data
        /// @param o the current overload
        /// @param matcher_indices the remaining matcher indices
        /// @param s the required evaluation stage of the overload
        MatchState(type::Manager& ty_mgr,
                   SymbolTable& syms,
                   TemplateState& t,
                   const TableData& d,
                   const OverloadInfo* o,
                   MatcherIndex const* matcher_indices,
                   EvaluationStage s)
            : types(ty_mgr),
              symbols(syms),
              templates(t),
              data(d),
              overload(o),
              earliest_eval_stage(s),
              matcher_indices_(matcher_indices) {}

        /// The type manager
        type::Manager& types;

        /// The symbol manager
        SymbolTable& symbols;

        /// The template types and numbers
        TemplateState& templates;

        /// The table data
        TableData const& data;

        /// The current overload being evaluated
        OverloadInfo const* const overload;

        /// The earliest evaluation stage of the builtin call
        EvaluationStage earliest_eval_stage;

        /// Type uses the next TypeMatcher from the matcher indices to match the type @p ty.
        /// @param ty the type to try matching
        /// @returns the canonical expected type if the type matches, otherwise nullptr.
        /// @note: The matcher indices are progressed on calling.
        const type::Type* Type(const type::Type* ty) {
            MatcherIndex matcher_index = *matcher_indices_++;
            auto& matcher = data.type_matchers[matcher_index];
            return matcher.match(*this, ty);
        }

        /// Num uses the next NumMatcher from the matcher indices to match @p number.
        /// @param number the number to try matching
        /// @returns the canonical expected number if the number matches, otherwise an invalid
        /// number.
        /// @note: The matcher indices are progressed on calling.
        Number Num(Number number) {
            MatcherIndex matcher_index = *matcher_indices_++;
            auto& matcher = data.number_matchers[matcher_index];
            return matcher.match(*this, number);
        }

        /// @returns a string representation of the next TypeMatcher from the matcher indices.
        /// @note: The matcher indices are progressed on calling.
        std::string TypeName() {
            MatcherIndex matcher_index = *matcher_indices_++;
            auto& matcher = data.type_matchers[matcher_index];
            return matcher.string(this);
        }

        /// @returns a string representation of the next NumberMatcher from the matcher indices.
        /// @note: The matcher indices are progressed on calling.
        std::string NumName() {
            MatcherIndex matcher_index = *matcher_indices_++;
            auto& matcher = data.number_matchers[matcher_index];
            return matcher.string(this);
        }

      private:
        MatcherIndex const* matcher_indices_ = nullptr;
    };

    /// A TypeMatcher is the interface used to match an type used as part of an
    /// overload's parameter or return type.
    struct TypeMatcher {
        /// Checks whether the given type matches the matcher rules, and returns the
        /// expected, canonicalized type on success.
        /// Match may define and refine the template types and numbers in state.
        /// The parameter `type` is the type to match
        /// Returns the canonicalized type on match, otherwise nullptr
        using MatchFn = const type::Type*(MatchState& state, const type::Type* type);

        /// @see #MatchFn
        MatchFn* const match;

        /// Returns a string representation of the matcher.
        /// Used for printing error messages when no overload is found.
        using StringFn = std::string(MatchState* state);

        /// @see #StringFn
        StringFn* const string;
    };

    /// A NumberMatcher is the interface used to match a number or enumerator used
    /// as part of an overload's parameter or return type.
    struct NumberMatcher {
        /// Checks whether the given number matches the matcher rules.
        /// Match may define template numbers in state.
        /// The parameter `number` is the number to match
        /// Returns true if the argument type is as expected.
        using MatchFn = Number(MatchState& state, Number number);

        /// @see #MatchFn
        MatchFn* const match;

        /// Returns a string representation of the matcher.
        /// Used for printing error messages when no overload is found.
        using StringFn = std::string(MatchState* state);

        /// @see #StringFn
        StringFn* const string;
    };

    /// TemplateTypeMatcher is a Matcher for a template type.
    /// The TemplateTypeMatcher will initially match against any type, and then will only be further
    /// constrained based on the conversion rules defined at
    /// https://www.w3.org/TR/WGSL/#conversion-rank
    template <size_t INDEX>
    struct TemplateTypeMatcher {
        /// The TypeMatcher for the template type with the index `INDEX`
        static constexpr TypeMatcher matcher{
            /* match */
            [](MatchState& state, const type::Type* type) -> const type::Type* {
                if (type->Is<Any>()) {
                    return state.templates.Type(INDEX);
                }
                if (auto* templates = state.templates.Type(INDEX, type)) {
                    return templates;
                }
                return nullptr;
            },
            /* string */
            [](MatchState* state) -> std::string {
                return state->overload->template_types[INDEX].name;
            },
        };
    };

    /// TemplateNumberMatcher is a Matcher for a template number.
    /// The TemplateNumberMatcher will match against any number (so long as it is
    /// consistent for all uses in the overload)
    template <size_t INDEX>
    struct TemplateNumberMatcher {
        /// The NumberMatcher for the template number with the index `INDEX`
        static constexpr NumberMatcher matcher{
            /* match */
            [](TableData::MatchState& state, TableData::Number number) -> TableData::Number {
                if (number.IsAny()) {
                    return state.templates.Num(INDEX);
                }
                return state.templates.Num(INDEX, number) ? number : TableData::Number::invalid;
            },
            /* string */
            [](TableData::MatchState* state) -> std::string {
                return state->overload->template_numbers[INDEX].name;
            },
        };
    };

    /// The list of type matchers used by the intrinsic overloads
    Slice<TypeMatcher const> const type_matchers;
    /// The list of number matchers used by the intrinsic overloads
    Slice<NumberMatcher const> const number_matchers;
    /// The type constructor and convertor intrinsic overloads
    Slice<IntrinsicInfo const> const ctor_conv;
    /// The builtin function intrinsic overloads
    Slice<IntrinsicInfo const> const builtins;
    /// The IntrinsicInfo for the binary operator 'plus'
    IntrinsicInfo const& binary_plus;
    /// The IntrinsicInfo for the binary operator 'minus'
    IntrinsicInfo const& binary_minus;
    /// The IntrinsicInfo for the binary operator 'star'
    IntrinsicInfo const& binary_star;
    /// The IntrinsicInfo for the binary operator 'divide'
    IntrinsicInfo const& binary_divide;
    /// The IntrinsicInfo for the binary operator 'modulo'
    IntrinsicInfo const& binary_modulo;
    /// The IntrinsicInfo for the binary operator 'xor'
    IntrinsicInfo const& binary_xor;
    /// The IntrinsicInfo for the binary operator 'and'
    IntrinsicInfo const& binary_and;
    /// The IntrinsicInfo for the binary operator 'or'
    IntrinsicInfo const& binary_or;
    /// The IntrinsicInfo for the binary operator 'logical_and'
    IntrinsicInfo const& binary_logical_and;
    /// The IntrinsicInfo for the binary operator 'logical_or'
    IntrinsicInfo const& binary_logical_or;
    /// The IntrinsicInfo for the binary operator 'equal'
    IntrinsicInfo const& binary_equal;
    /// The IntrinsicInfo for the binary operator 'not_equal'
    IntrinsicInfo const& binary_not_equal;
    /// The IntrinsicInfo for the binary operator 'less_than'
    IntrinsicInfo const& binary_less_than;
    /// The IntrinsicInfo for the binary operator 'greater_than'
    IntrinsicInfo const& binary_greater_than;
    /// The IntrinsicInfo for the binary operator 'less_than_equal'
    IntrinsicInfo const& binary_less_than_equal;
    /// The IntrinsicInfo for the binary operator 'greater_than_equal'
    IntrinsicInfo const& binary_greater_than_equal;
    /// The IntrinsicInfo for the binary operator 'shift_left'
    IntrinsicInfo const& binary_shift_left;
    /// The IntrinsicInfo for the binary operator 'shift_right'
    IntrinsicInfo const& binary_shift_right;
    /// The IntrinsicInfo for the unary operator 'not'
    IntrinsicInfo const& unary_not;
    /// The IntrinsicInfo for the unary operator 'complement'
    IntrinsicInfo const& unary_complement;
    /// The IntrinsicInfo for the unary operator 'minus'
    IntrinsicInfo const& unary_minus;
};

}  // namespace tint::core::intrinsic

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_TABLE_DATA_H_
