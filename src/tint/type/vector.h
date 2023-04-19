// Copyright 2022 The Tint Authors.
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

#ifndef SRC_TINT_TYPE_VECTOR_H_
#define SRC_TINT_TYPE_VECTOR_H_

#include <string>

#include "src/tint/type/type.h"

namespace tint::type {

/// A vector type.
class Vector : public utils::Castable<Vector, Type> {
  public:
    /// Constructor
    /// @param subtype the vector element type
    /// @param size the number of elements in the vector
    /// @param packed the optional 'packed' modifier
    Vector(Type const* subtype, uint32_t size, bool packed = false);

    /// Destructor
    ~Vector() override;

    /// @param other the other node to compare against
    /// @returns true if the this type is equal to @p other
    bool Equals(const UniqueNode& other) const override;

    /// @returns the type of the vector elements
    const Type* type() const { return subtype_; }

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    std::string FriendlyName() const override;

    /// @returns the number of elements in the vector
    uint32_t Width() const { return width_; }

    /// @returns the size in bytes of the type. This may include tail padding.
    uint32_t Size() const override;

    /// @returns the alignment in bytes of the type. This may include tail padding.
    uint32_t Align() const override;

    /// @returns `true` if this vector is packed, false otherwise
    bool Packed() const { return packed_; }

    /// @param width the width of the vector
    /// @returns the size in bytes of a vector of the given width.
    static uint32_t SizeOf(uint32_t width);

    /// @param width the width of the vector
    /// @returns the alignment in bytes of a vector of the given width.
    static uint32_t AlignOf(uint32_t width);

    /// @param ctx the clone context
    /// @returns a clone of this type
    Vector* Clone(CloneContext& ctx) const override;

  private:
    Type const* const subtype_;
    const uint32_t width_;
    const bool packed_;
};

}  // namespace tint::type

#endif  // SRC_TINT_TYPE_VECTOR_H_
