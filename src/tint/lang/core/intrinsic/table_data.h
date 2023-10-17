// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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

/// Forward declaration
namespace tint::core::intrinsic {
struct TableData;
}  // namespace tint::core::intrinsic

namespace tint::core::intrinsic {

/// An enumerator of index namespaces.
enum class TableIndexNamespace {
    kTemplateType,
    kTemplateNumber,
    kTypeMatcher,
    kNumberMatcher,
    kTypeMatcherIndices,
    kNumberMatcherIndices,
    kParameter,
    kOverload,
    kConstEvalFunction,
};

/// A wrapper around an integer type, used to index intrinsic table data
/// @tparam T the index data type
/// @tparam N the index namespace
template <TableIndexNamespace N, typename T>
struct TableIndex {
    static_assert(std::is_integral_v<T> && std::is_unsigned_v<T>,
                  "T must be an unsigned integer type");

    /// The value used to represent an invalid index
    static constexpr T kInvalid = std::numeric_limits<T>::max();

    /// Constructor for invalid index
    constexpr TableIndex() : value(kInvalid) {}

    /// Constructor
    /// @param index the index value
    constexpr explicit TableIndex(T index) : value(index) {}

    /// @returns true if this index is not invalid
    bool IsValid() const { return value != kInvalid; }

    /// Equality operator
    /// @param other the index to compare against
    /// @returns true if this index is equal to @p other
    bool operator==(const TableIndex& other) { return value == other.value; }

    /// Inequality operator
    /// @param other the index to compare against
    /// @returns true if this index is equal to @p other
    bool operator!=(const TableIndex& other) { return value != other.value; }

    /// @param offset the offset to apply to this index
    /// @returns a new index offset by @p offset
    template <typename U>
    auto operator+(U offset) const {
        static_assert(std::is_integral_v<U> && std::is_unsigned_v<U>,
                      "T must be an unsigned integer type");
        auto new_value = value + offset;
        return TableIndex<N, decltype(new_value)>(new_value);
    }

    /// @param arr a C-style array
    /// @returns true if the integer type `T` has enough bits to index all the
    /// elements in the array @p arr.
    template <typename U, size_t COUNT>
    static constexpr bool CanIndex(U (&arr)[COUNT]) {
        (void)arr;  // The array isn't actually used
        /// kInvalid is the largest value representable by `T`. It is not a valid index.
        return COUNT < kInvalid;
    }

    /// The index value
    const T value = kInvalid;
};

/// Index type used to index TableData::template_types
using TemplateTypeIndex = TableIndex<TableIndexNamespace::kTemplateType, uint8_t>;

/// Index type used to index TableData::template_numbers
using TemplateNumberIndex = TableIndex<TableIndexNamespace::kTemplateNumber, uint8_t>;

/// Index type used to index TableData::type_matchers
using TypeMatcherIndex = TableIndex<TableIndexNamespace::kTypeMatcher, uint8_t>;

/// Index type used to index TableData::number_matchers
using NumberMatcherIndex = TableIndex<TableIndexNamespace::kNumberMatcher, uint8_t>;

/// Index type used to index TableData::type_matcher_indices
using TypeMatcherIndicesIndex = TableIndex<TableIndexNamespace::kTypeMatcherIndices, uint8_t>;

/// Index type used to index TableData::number_matcher_indices
using NumberMatcherIndicesIndex = TableIndex<TableIndexNamespace::kNumberMatcherIndices, uint8_t>;

/// Index type used to index TableData::parameters
using ParameterIndex = TableIndex<TableIndexNamespace::kParameter, uint16_t>;

/// Index type used to index TableData::overloads
using OverloadIndex = TableIndex<TableIndexNamespace::kOverload, uint16_t>;

/// Index type used to index TableData::const_eval_functions
using ConstEvalFunctionIndex = TableIndex<TableIndexNamespace::kConstEvalFunction, uint8_t>;

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

    /// Pointer to a list of type matcher indices that are used to match the parameter types.
    /// These indices are consumed by the matchers themselves.
    const TypeMatcherIndicesIndex type_matcher_indices;

    /// Pointer to a list of number matcher indices that are used to match the parameter types.
    /// These indices are consumed by the matchers themselves.
    const NumberMatcherIndicesIndex number_matcher_indices;
};

/// TemplateTypeInfo describes an template type
struct TemplateTypeInfo {
    /// Name of the template type (e.g. 'T')
    const char* name;
    /// Optional type matcher constraint.
    const TypeMatcherIndex matcher_index;
};

/// TemplateNumberInfo describes a template number
struct TemplateNumberInfo {
    /// Name of the template number (e.g. 'N')
    const char* name;
    /// Optional number matcher constraint.
    const NumberMatcherIndex matcher_index;
};

/// OverloadInfo describes a single function overload
struct OverloadInfo {
    /// The flags for the overload
    const OverloadFlags flags;
    /// Total number of parameters for the overload
    const uint8_t num_parameters;
    /// Total number of template types for the overload
    const uint8_t num_template_types;
    /// Total number of template numbers for the overload
    const uint8_t num_template_numbers;
    /// Index of the first template type in TableData::type_matchers
    const TemplateTypeIndex template_types;
    /// Index of the first template number in TableData::number_matchers
    const TemplateNumberIndex template_numbers;
    /// Index of the first parameter in TableData::parameters
    const ParameterIndex parameters;
    /// Index of a list of type matcher indices that are used to build the return type.
    const TypeMatcherIndicesIndex return_type_matcher_indices;
    /// Index of a list of number matcher indices that are used to build the return type.
    const NumberMatcherIndicesIndex return_number_matcher_indices;
    /// The function used to evaluate the overload at shader-creation time.
    const ConstEvalFunctionIndex const_eval_fn;
};

/// IntrinsicInfo describes a builtin function or operator overload
struct IntrinsicInfo {
    /// Number of overloads of the intrinsic
    const uint8_t num_overloads;
    /// Index of the first overload for the function
    const OverloadIndex overloads;
};

/// A IntrinsicInfo with no overloads
static constexpr IntrinsicInfo kNoOverloads{0, OverloadIndex(OverloadIndex::kInvalid)};

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
class Any final : public Castable<Any, core::type::Type> {
  public:
    Any();
    ~Any() override;

    /// @copydoc core::type::UniqueNode::Equals
    bool Equals(const core::type::UniqueNode& other) const override;
    /// @copydoc core::type::Type::FriendlyName
    std::string FriendlyName() const override;
    /// @copydoc core::type::Type::Clone
    core::type::Type* Clone(core::type::CloneContext& ctx) const override;
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
    const core::type::Type* Type(size_t idx, const core::type::Type* ty) {
        if (idx >= types_.Length()) {
            types_.Resize(idx + 1);
        }
        auto& t = types_[idx];
        if (t == nullptr) {
            t = ty;
            return ty;
        }
        ty = core::type::Type::Common(Vector{t, ty});
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

    /// @param idx the index of the template type
    /// @returns the template type with index @p idx, or nullptr if the type was not
    /// defined.
    const core::type::Type* Type(size_t idx) const {
        if (idx >= types_.Length()) {
            return nullptr;
        }
        return types_[idx];
    }

    /// SetType replaces the template type with index @p idx with type @p ty.
    /// @param idx the index of the template type
    /// @param ty the new type for the template
    void SetType(size_t idx, const core::type::Type* ty) {
        if (idx >= types_.Length()) {
            types_.Resize(idx + 1);
        }
        types_[idx] = ty;
    }

    /// @returns the number type with index @p idx.
    /// @param idx the index of the template number
    Number Num(size_t idx) const {
        if (idx >= numbers_.Length()) {
            return Number::invalid;
        }
        return numbers_[idx];
    }

    /// @return the total number of type and number templates
    size_t Count() const { return types_.Length() + numbers_.Length(); }

  private:
    Vector<const core::type::Type*, 4> types_;
    Vector<Number, 2> numbers_;
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
    /// @param type_matcher_indices the remaining type matcher indices
    /// @param number_matcher_indices the remaining number matcher indices
    /// @param s the required evaluation stage of the overload
    MatchState(core::type::Manager& ty_mgr,
               SymbolTable& syms,
               TemplateState& t,
               const TableData& d,
               const OverloadInfo& o,
               const TypeMatcherIndex* type_matcher_indices,
               const NumberMatcherIndex* number_matcher_indices,
               EvaluationStage s)
        : types(ty_mgr),
          symbols(syms),
          templates(t),
          data(d),
          overload(o),
          earliest_eval_stage(s),
          type_matcher_indices_(type_matcher_indices),
          number_matcher_indices_(number_matcher_indices) {}

    /// The type manager
    core::type::Manager& types;

    /// The symbol manager
    SymbolTable& symbols;

    /// The template types and numbers
    TemplateState& templates;

    /// The table data
    const TableData& data;

    /// The current overload being evaluated
    const OverloadInfo& overload;

    /// The earliest evaluation stage of the builtin call
    EvaluationStage earliest_eval_stage;

    /// Type uses the next TypeMatcher from the matcher indices to match the type @p ty.
    /// @param ty the type to try matching
    /// @returns the canonical expected type if the type matches, otherwise nullptr.
    /// @note: The matcher indices are progressed on calling.
    inline const core::type::Type* Type(const core::type::Type* ty);

    /// Num uses the next NumMatcher from the matcher indices to match @p number.
    /// @param number the number to try matching
    /// @returns the canonical expected number if the number matches, otherwise an invalid
    /// number.
    /// @note: The matcher indices are progressed on calling.
    inline Number Num(Number number);

    /// @returns a string representation of the next TypeMatcher from the matcher indices.
    /// @note: The matcher indices are progressed on calling.
    inline std::string TypeName();

    /// @returns a string representation of the next NumberMatcher from the matcher indices.
    /// @note: The matcher indices are progressed on calling.
    inline std::string NumName();

  private:
    const TypeMatcherIndex* type_matcher_indices_ = nullptr;
    const NumberMatcherIndex* number_matcher_indices_ = nullptr;
};

/// A TypeMatcher is the interface used to match an type used as part of an
/// overload's parameter or return type.
struct TypeMatcher {
    /// Checks whether the given type matches the matcher rules, and returns the
    /// expected, canonicalized type on success.
    /// Match may define and refine the template types and numbers in state.
    /// The parameter `type` is the type to match
    /// Returns the canonicalized type on match, otherwise nullptr
    using MatchFn = const core::type::Type*(MatchState& state, const core::type::Type* type);

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

/// TableData holds the immutable data that holds the intrinsic data for a language.
struct TableData {
    /// @param idx the index of the TemplateTypeInfo in the table data
    /// @returns the TemplateTypeInfo with the given index
    template <typename T>
    const TemplateTypeInfo& operator[](
        TableIndex<TableIndexNamespace::kTemplateType, T> idx) const {
        return template_types[idx.value];
    }

    /// @param idx the index of the TemplateNumberInfo in the table data
    /// @returns the TemplateNumberInfo with the given index
    template <typename T>
    const TemplateNumberInfo& operator[](
        TableIndex<TableIndexNamespace::kTemplateNumber, T> idx) const {
        return template_numbers[idx.value];
    }

    /// @param idx the index of the TypeMatcherIndex in the table data
    /// @returns the TypeMatcherIndex with the given index
    template <typename T>
    const TypeMatcherIndex* operator[](
        TableIndex<TableIndexNamespace::kTypeMatcherIndices, T> idx) const {
        return idx.IsValid() ? &type_matcher_indices[idx.value] : nullptr;
    }

    /// @param idx the index of the NumberMatcherIndex in the table data
    /// @returns the NumberMatcherIndex with the given index
    template <typename T>
    const NumberMatcherIndex* operator[](
        TableIndex<TableIndexNamespace::kNumberMatcherIndices, T> idx) const {
        return idx.IsValid() ? &number_matcher_indices[idx.value] : nullptr;
    }

    /// @param idx the index of the TypeMatcher in the table data
    /// @returns the TypeMatcher with the given index
    template <typename T>
    const TypeMatcher& operator[](TableIndex<TableIndexNamespace::kTypeMatcher, T> idx) const {
        return type_matchers[idx.value];
    }

    /// @param idx the index of the NumberMatcher in the table data
    /// @returns the NumberMatcher with the given index
    template <typename T>
    const NumberMatcher& operator[](TableIndex<TableIndexNamespace::kNumberMatcher, T> idx) const {
        return number_matchers[idx.value];
    }

    /// @param idx the index of the ParameterInfo in the table data
    /// @returns the ParameterInfo with the given index
    template <typename T>
    const ParameterInfo& operator[](TableIndex<TableIndexNamespace::kParameter, T> idx) const {
        return parameters[idx.value];
    }

    /// @param idx the index of the OverloadInfo in the table data
    /// @returns the OverloadInfo with the given index
    template <typename T>
    const OverloadInfo& operator[](TableIndex<TableIndexNamespace::kOverload, T> idx) const {
        return overloads[idx.value];
    }

    /// @param idx the index of the constant::Eval::Function in the table data
    /// @returns the constant::Eval::Function with the given index
    template <typename T>
    constant::Eval::Function operator[](
        TableIndex<TableIndexNamespace::kConstEvalFunction, T> idx) const {
        return idx.IsValid() ? const_eval_functions[idx.value] : nullptr;
    }

    /// The list of type infos used by the intrinsic overloads
    const Slice<const TemplateTypeInfo> template_types;
    /// The list of number infos used by the intrinsic overloads
    const Slice<const TemplateNumberInfo> template_numbers;
    /// The list of type matcher indices
    const Slice<const TypeMatcherIndex> type_matcher_indices;
    /// The list of number matcher indices
    const Slice<const NumberMatcherIndex> number_matcher_indices;
    /// The list of type matchers used by the intrinsic overloads
    const Slice<const TypeMatcher> type_matchers;
    /// The list of number matchers used by the intrinsic overloads
    const Slice<const NumberMatcher> number_matchers;
    /// The list of parameters used by the intrinsic overloads
    const Slice<const ParameterInfo> parameters;
    /// The list of overloads used by the intrinsics
    const Slice<const OverloadInfo> overloads;
    /// The list of constant evaluation functions used by the intrinsics
    const Slice<const constant::Eval::Function> const_eval_functions;
    /// The type constructor and convertor intrinsics
    const Slice<const IntrinsicInfo> ctor_conv;
    /// The builtin function intrinsic
    const Slice<const IntrinsicInfo> builtins;
    /// The IntrinsicInfo for the binary operator 'plus'
    const IntrinsicInfo& binary_plus;
    /// The IntrinsicInfo for the binary operator 'minus'
    const IntrinsicInfo& binary_minus;
    /// The IntrinsicInfo for the binary operator 'star'
    const IntrinsicInfo& binary_star;
    /// The IntrinsicInfo for the binary operator 'divide'
    const IntrinsicInfo& binary_divide;
    /// The IntrinsicInfo for the binary operator 'modulo'
    const IntrinsicInfo& binary_modulo;
    /// The IntrinsicInfo for the binary operator 'xor'
    const IntrinsicInfo& binary_xor;
    /// The IntrinsicInfo for the binary operator 'and'
    const IntrinsicInfo& binary_and;
    /// The IntrinsicInfo for the binary operator 'or'
    const IntrinsicInfo& binary_or;
    /// The IntrinsicInfo for the binary operator 'logical_and'
    const IntrinsicInfo& binary_logical_and;
    /// The IntrinsicInfo for the binary operator 'logical_or'
    const IntrinsicInfo& binary_logical_or;
    /// The IntrinsicInfo for the binary operator 'equal'
    const IntrinsicInfo& binary_equal;
    /// The IntrinsicInfo for the binary operator 'not_equal'
    const IntrinsicInfo& binary_not_equal;
    /// The IntrinsicInfo for the binary operator 'less_than'
    const IntrinsicInfo& binary_less_than;
    /// The IntrinsicInfo for the binary operator 'greater_than'
    const IntrinsicInfo& binary_greater_than;
    /// The IntrinsicInfo for the binary operator 'less_than_equal'
    const IntrinsicInfo& binary_less_than_equal;
    /// The IntrinsicInfo for the binary operator 'greater_than_equal'
    const IntrinsicInfo& binary_greater_than_equal;
    /// The IntrinsicInfo for the binary operator 'shift_left'
    const IntrinsicInfo& binary_shift_left;
    /// The IntrinsicInfo for the binary operator 'shift_right'
    const IntrinsicInfo& binary_shift_right;
    /// The IntrinsicInfo for the unary operator 'not'
    const IntrinsicInfo& unary_not;
    /// The IntrinsicInfo for the unary operator 'complement'
    const IntrinsicInfo& unary_complement;
    /// The IntrinsicInfo for the unary operator 'minus'
    const IntrinsicInfo& unary_minus;
};

const core::type::Type* MatchState::Type(const core::type::Type* ty) {
    TypeMatcherIndex matcher_index{(*type_matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    return matcher.match(*this, ty);
}

Number MatchState::Num(Number number) {
    NumberMatcherIndex matcher_index{(*number_matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    return matcher.match(*this, number);
}

std::string MatchState::TypeName() {
    TypeMatcherIndex matcher_index{(*type_matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    return matcher.string(this);
}

std::string MatchState::NumName() {
    NumberMatcherIndex matcher_index{(*number_matcher_indices_++).value};
    auto& matcher = data[matcher_index];
    return matcher.string(this);
}

/// TemplateTypeMatcher is a Matcher for a template type.
/// The TemplateTypeMatcher will initially match against any type, and then will only be further
/// constrained based on the conversion rules defined at
/// https://www.w3.org/TR/WGSL/#conversion-rank
template <size_t INDEX>
struct TemplateTypeMatcher {
    /// The TypeMatcher for the template type with the index `INDEX`
    static constexpr TypeMatcher matcher{
        /* match */
        [](MatchState& state, const core::type::Type* type) -> const core::type::Type* {
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
            return state->data[state->overload.template_types + INDEX].name;
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
        [](MatchState& state, Number number) -> Number {
            if (number.IsAny()) {
                return state.templates.Num(INDEX);
            }
            return state.templates.Num(INDEX, number) ? number : Number::invalid;
        },
        /* string */
        [](MatchState* state) -> std::string {
            return state->data[state->overload.template_numbers + INDEX].name;
        },
    };
};

}  // namespace tint::core::intrinsic

#endif  // SRC_TINT_LANG_CORE_INTRINSIC_TABLE_DATA_H_
