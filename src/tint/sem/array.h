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

// Forward declarations
namespace tint::sem {
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
struct OverrideArrayCount {
    /// The `override` variable.
    const GlobalVariable* variable;
};

/// The variant of an ArrayCount when the array is is runtime-sized.
/// Example:
/// ```
/// type arr = array<i32>
/// ```
struct RuntimeArrayCount {};

/// An array count is either a constant-expression value, an override identifier, or runtime-sized.
using ArrayCount = std::variant<ConstantArrayCount, OverrideArrayCount, RuntimeArrayCount>;

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
inline bool operator==(const OverrideArrayCount& a, const OverrideArrayCount& b) {
    return a.variable == b.variable;
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
          typename = std::enable_if_t<std::is_same_v<T, ConstantArrayCount> ||
                                      std::is_same_v<T, OverrideArrayCount> ||
                                      std::is_same_v<T, RuntimeArrayCount>>>
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
    static const char* kErrExpectedConstantCount;

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

    /// @returns true if this array is sized using an override variable
    bool IsOverrideSized() const { return std::holds_alternative<OverrideArrayCount>(count_); }

    /// @returns true if this array is runtime sized
    bool IsRuntimeSized() const { return std::holds_alternative<RuntimeArrayCount>(count_); }

    /// @returns true if constructible as per
    /// https://gpuweb.github.io/gpuweb/wgsl/#constructible-types
    bool IsConstructible() const override;

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
    const bool constructible_;
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

/// Custom std::hash specialization for tint::sem::OverrideArrayCount.
template <>
class hash<tint::sem::OverrideArrayCount> {
  public:
    /// @param count the count to hash
    /// @return the hash value
    inline std::size_t operator()(const tint::sem::OverrideArrayCount& count) const {
        return std::hash<decltype(count.variable)>()(count.variable);
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
