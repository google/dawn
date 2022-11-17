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

#ifndef SRC_TINT_SEM_ARRAY_H_
#define SRC_TINT_SEM_ARRAY_H_

#include <stdint.h>
#include <optional>
#include <string>
#include <variant>

#include "src/tint/sem/node.h"
#include "src/tint/sem/type.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/unique_vector.h"

// Forward declarations
namespace tint::sem {
class Expression;
class GlobalVariable;
}  // namespace tint::sem

namespace tint::sem {

/// The variant of an ArrayCount when the array is a const-expression.
/// Example:
/// ```
/// const N = 123;
/// type arr = array<i32, N>
/// ```
struct ConstantArrayCount {
    /// The array count constant-expression value.
    uint32_t value;
};

/// The variant of an ArrayCount when the count is a named override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N>
/// ```
struct NamedOverrideArrayCount {
    /// The `override` variable.
    const GlobalVariable* variable;
};

/// The variant of an ArrayCount when the count is an unnamed override variable.
/// Example:
/// ```
/// override N : i32;
/// type arr = array<i32, N*2>
/// ```
struct UnnamedOverrideArrayCount {
    /// The unnamed override expression.
    /// Note: Each AST expression gets a unique semantic expression node, so two equivalent AST
    /// expressions will not result in the same `expr` pointer. This property is important to ensure
    /// that two array declarations with equivalent AST expressions do not compare equal.
    /// For example, consider:
    /// ```
    /// override size : u32;
    /// var<workgroup> a : array<f32, size * 2>;
    /// var<workgroup> b : array<f32, size * 2>;
    /// ```
    // The array count for `a` and `b` have equivalent AST expressions, but the types for `a` and
    // `b` must not compare equal.
    const Expression* expr;
};

/// The variant of an ArrayCount when the array is is runtime-sized.
/// Example:
/// ```
/// type arr = array<i32>
/// ```
struct RuntimeArrayCount {};

/// An array count is either a constant-expression value, a named override identifier, an unnamed
/// override identifier, or runtime-sized.
using ArrayCount = std::variant<ConstantArrayCount,
                                NamedOverrideArrayCount,
                                UnnamedOverrideArrayCount,
                                RuntimeArrayCount>;

/// Equality operator
/// @param a the LHS ConstantArrayCount
/// @param b the RHS ConstantArrayCount
/// @returns true if @p a is equal to @p b
inline bool operator==(const ConstantArrayCount& a, const ConstantArrayCount& b) {
    return a.value == b.value;
}

/// Equality operator
/// @param a the LHS OverrideArrayCount
/// @param b the RHS OverrideArrayCount
/// @returns true if @p a is equal to @p b
inline bool operator==(const NamedOverrideArrayCount& a, const NamedOverrideArrayCount& b) {
    return a.variable == b.variable;
}

/// Equality operator
/// @param a the LHS OverrideArrayCount
/// @param b the RHS OverrideArrayCount
/// @returns true if @p a is equal to @p b
inline bool operator==(const UnnamedOverrideArrayCount& a, const UnnamedOverrideArrayCount& b) {
    return a.expr == b.expr;
}

/// Equality operator
/// @returns true
inline bool operator==(const RuntimeArrayCount&, const RuntimeArrayCount&) {
    return true;
}

/// Equality operator
/// @param a the LHS ArrayCount
/// @param b the RHS count
/// @returns true if @p a is equal to @p b
template <typename T,
          typename = std::enable_if_t<
              std::is_same_v<T, ConstantArrayCount> || std::is_same_v<T, NamedOverrideArrayCount> ||
              std::is_same_v<T, UnnamedOverrideArrayCount> || std::is_same_v<T, RuntimeArrayCount>>>
inline bool operator==(const ArrayCount& a, const T& b) {
    TINT_BEGIN_DISABLE_WARNING(UNREACHABLE_CODE);
    return std::visit(
        [&](auto count) {
            if constexpr (std::is_same_v<std::decay_t<decltype(count)>, T>) {
                return count == b;
            }
            return false;
        },
        a);
    TINT_END_DISABLE_WARNING(UNREACHABLE_CODE);
}

/// Array holds the semantic information for Array nodes.
class Array final : public Castable<Array, Type> {
  public:
    /// An error message string stating that the array count was expected to be a constant
    /// expression. Used by multiple writers and transforms.
    static const char* const kErrExpectedConstantCount;

    /// Constructor
    /// @param element the array element type
    /// @param count the number of elements in the array.
    /// @param align the byte alignment of the array
    /// @param size the byte size of the array. The size will be 0 if the array element count is
    ///        pipeline overridable.
    /// @param stride the number of bytes from the start of one element of the
    ///        array to the start of the next element
    /// @param implicit_stride the number of bytes from the start of one element
    /// of the array to the start of the next element, if there was no `@stride`
    /// attribute applied.
    Array(Type const* element,
          ArrayCount count,
          uint32_t align,
          uint32_t size,
          uint32_t stride,
          uint32_t implicit_stride);

    /// @returns a hash of the type.
    size_t Hash() const override;

    /// @param other the other type to compare against
    /// @returns true if the this type is equal to the given type
    bool Equals(const Type& other) const override;

    /// @return the array element type
    Type const* ElemType() const { return element_; }

    /// @returns the number of elements in the array.
    const ArrayCount& Count() const { return count_; }

    /// @returns the array count if the count is a const-expression, otherwise returns nullopt.
    inline std::optional<uint32_t> ConstantCount() const {
        if (auto* count = std::get_if<ConstantArrayCount>(&count_)) {
            return count->value;
        }
        return std::nullopt;
    }

    /// @returns the byte alignment of the array
    /// @note this may differ from the alignment of a structure member of this
    /// array type, if the member is annotated with the `@align(n)` attribute.
    uint32_t Align() const override;

    /// @returns the byte size of the array
    /// @note this may differ from the size of a structure member of this array
    /// type, if the member is annotated with the `@size(n)` attribute.
    uint32_t Size() const override;

    /// @returns the number of bytes from the start of one element of the
    /// array to the start of the next element
    uint32_t Stride() const { return stride_; }

    /// @returns the number of bytes from the start of one element of the
    /// array to the start of the next element, if there was no `@stride`
    /// attribute applied
    uint32_t ImplicitStride() const { return implicit_stride_; }

    /// @returns true if the value returned by Stride() matches the element's
    /// natural stride
    bool IsStrideImplicit() const { return stride_ == implicit_stride_; }

    /// @returns true if this array is sized using an const-expression
    bool IsConstantSized() const { return std::holds_alternative<ConstantArrayCount>(count_); }

    /// @returns true if this array is sized using a named override variable
    bool IsNamedOverrideSized() const {
        return std::holds_alternative<NamedOverrideArrayCount>(count_);
    }

    /// @returns true if this array is sized using an unnamed override variable
    bool IsUnnamedOverrideSized() const {
        return std::holds_alternative<UnnamedOverrideArrayCount>(count_);
    }

    /// @returns true if this array is sized using a named or unnamed override variable
    bool IsOverrideSized() const { return IsNamedOverrideSized() || IsUnnamedOverrideSized(); }

    /// @returns true if this array is runtime sized
    bool IsRuntimeSized() const { return std::holds_alternative<RuntimeArrayCount>(count_); }

    /// Records that this array type (transitively) references the given override variable.
    /// @param var the module-scope override variable
    void AddTransitivelyReferencedOverride(const GlobalVariable* var) {
        referenced_overrides_.Add(var);
    }

    /// @returns all transitively referenced override variables
    const utils::UniqueVector<const GlobalVariable*, 4>& TransitivelyReferencedOverrides() const {
        return referenced_overrides_;
    }

    /// @param symbols the program's symbol table
    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName(const SymbolTable& symbols) const override;

  private:
    Type const* const element_;
    const ArrayCount count_;
    const uint32_t align_;
    const uint32_t size_;
    const uint32_t stride_;
    const uint32_t implicit_stride_;
    utils::UniqueVector<const GlobalVariable*, 4> referenced_overrides_;
};

}  // namespace tint::sem

namespace std {

/// Custom std::hash specialization for tint::sem::ConstantArrayCount.
template <>
class hash<tint::sem::ConstantArrayCount> {
  public:
    /// @param count the count to hash
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::ConstantArrayCount& count) const {
        return std::hash<decltype(count.value)>()(count.value);
    }
};

/// Custom std::hash specialization for tint::sem::NamedOverrideArrayCount.
template <>
class hash<tint::sem::NamedOverrideArrayCount> {
  public:
    /// @param count the count to hash
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::NamedOverrideArrayCount& count) const {
        return std::hash<decltype(count.variable)>()(count.variable);
    }
};

/// Custom std::hash specialization for tint::sem::UnnamedOverrideArrayCount.
template <>
class hash<tint::sem::UnnamedOverrideArrayCount> {
  public:
    /// @param count the count to hash
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::UnnamedOverrideArrayCount& count) const {
        return std::hash<decltype(count.expr)>()(count.expr);
    }
};

/// Custom std::hash specialization for tint::sem::RuntimeArrayCount.
template <>
class hash<tint::sem::RuntimeArrayCount> {
  public:
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::RuntimeArrayCount&) const { return 42; }
};

}  // namespace std

#endif  // SRC_TINT_SEM_ARRAY_H_
