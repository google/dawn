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

#ifndef SRC_TINT_TYPE_ARRAY_H_
#define SRC_TINT_TYPE_ARRAY_H_

#include <stdint.h>
#include <optional>
#include <string>
#include <variant>

#include "src/tint/type/array_count.h"
#include "src/tint/type/type.h"
#include "src/tint/utils/compiler_macros.h"
#include "src/tint/utils/unique_vector.h"

namespace tint::type {

/// Array holds the type information for Array nodes.
class Array final : public utils::Castable<Array, Type> {
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
          const ArrayCount* count,
          uint32_t align,
          uint32_t size,
          uint32_t stride,
          uint32_t implicit_stride);

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @return the array element type
    Type const* ElemType() const { return element_; }

    /// @returns the number of elements in the array.
    const ArrayCount* Count() const { return count_; }

    /// @returns the array count if the count is a const-expression, otherwise returns nullopt.
    inline std::optional<uint32_t> ConstantCount() const {
        if (auto* count = count_->As<ConstantArrayCount>()) {
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

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @param ctx the clone context
    /// @returns a clone of this type
    Array* Clone(CloneContext& ctx) const override;

  private:
    Type const* const element_;
    const ArrayCount* count_;
    const uint32_t align_;
    const uint32_t size_;
    const uint32_t stride_;
    const uint32_t implicit_stride_;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_ARRAY_H_
