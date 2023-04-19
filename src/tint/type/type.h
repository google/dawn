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

#ifndef SRC_TINT_TYPE_TYPE_H_
#define SRC_TINT_TYPE_TYPE_H_

#include <functional>
#include <string>

#include "src/tint/type/clone_context.h"
#include "src/tint/type/unique_node.h"
#include "src/tint/utils/enum_set.h"
#include "src/tint/utils/vector.h"

// Forward declarations
namespace tint {
class ProgramBuilder;
class SymbolTable;
}  // namespace tint

namespace tint::type {

enum Flag {
    /// Type is constructable.
    /// @see https://gpuweb.github.io/gpuweb/wgsl/#constructible-types
    kConstructable,
    /// Type has a creation-fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    kCreationFixedFootprint,
    /// Type has a fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    kFixedFootprint,
};

/// An alias to utils::EnumSet<Flag>
using Flags = utils::EnumSet<Flag>;

/// Base class for a type in the system
class Type : public utils::Castable<Type, UniqueNode> {
  public:
    /// Destructor
    ~Type() override;

    /// @returns the name for this type that closely resembles how it would be
    /// declared in WGSL.
    virtual std::string FriendlyName() const = 0;

    /// @returns the inner most pointee type if this is a pointer, `this`
    /// otherwise
    const Type* UnwrapPtr() const;

    /// @returns the inner type if this is a reference, `this` otherwise
    const Type* UnwrapRef() const;

    /// @returns the size in bytes of the type. This may include tail padding.
    /// @note opaque types will return a size of 0.
    virtual uint32_t Size() const;

    /// @returns the alignment in bytes of the type. This may include tail
    /// padding.
    /// @note opaque types will return a size of 0.
    virtual uint32_t Align() const;

    /// @param ctx the clone context
    /// @returns a clone of this type created in the provided context
    virtual Type* Clone(CloneContext& ctx) const = 0;

    /// @returns the flags on the type
    type::Flags Flags() { return flags_; }

    /// @returns true if type is constructable
    /// https://gpuweb.github.io/gpuweb/wgsl/#constructible-types
    inline bool IsConstructible() const { return flags_.Contains(Flag::kConstructable); }

    /// @returns true has a creation-fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    inline bool HasCreationFixedFootprint() const {
        return flags_.Contains(Flag::kCreationFixedFootprint);
    }

    /// @returns true has a fixed footprint.
    /// @see https://www.w3.org/TR/WGSL/#fixed-footprint-types
    inline bool HasFixedFootprint() const { return flags_.Contains(Flag::kFixedFootprint); }

    /// @returns true if this type is a scalar
    bool is_scalar() const;
    /// @returns true if this type is a numeric scalar
    bool is_numeric_scalar() const;
    /// @returns true if this type is a float scalar
    bool is_float_scalar() const;
    /// @returns true if this type is a float matrix
    bool is_float_matrix() const;
    /// @returns true if this type is a square float matrix
    bool is_square_float_matrix() const;
    /// @returns true if this type is a float vector
    bool is_float_vector() const;
    /// @returns true if this type is a float scalar or vector
    bool is_float_scalar_or_vector() const;
    /// @returns true if this type is a float scalar or vector or matrix
    bool is_float_scalar_or_vector_or_matrix() const;
    /// @returns true if this type is an integer scalar
    bool is_integer_scalar() const;
    /// @returns true if this type is a signed integer scalar
    bool is_signed_integer_scalar() const;
    /// @returns true if this type is an unsigned integer scalar
    bool is_unsigned_integer_scalar() const;
    /// @returns true if this type is a signed integer vector
    bool is_signed_integer_vector() const;
    /// @returns true if this type is an unsigned vector
    bool is_unsigned_integer_vector() const;
    /// @returns true if this type is an unsigned scalar or vector
    bool is_unsigned_integer_scalar_or_vector() const;
    /// @returns true if this type is a signed scalar or vector
    bool is_signed_integer_scalar_or_vector() const;
    /// @returns true if this type is an integer scalar or vector
    bool is_integer_scalar_or_vector() const;
    /// @returns true if this type is an abstract integer vector
    bool is_abstract_integer_vector() const;
    /// @returns true if this type is an abstract float vector
    bool is_abstract_float_vector() const;
    /// @returns true if this type is an abstract integer scalar or vector
    bool is_abstract_integer_scalar_or_vector() const;
    /// @returns true if this type is an abstract float scalar or vector
    bool is_abstract_float_scalar_or_vector() const;
    /// @returns true if this type is a boolean vector
    bool is_bool_vector() const;
    /// @returns true if this type is boolean scalar or vector
    bool is_bool_scalar_or_vector() const;
    /// @returns true if this type is a numeric vector
    bool is_numeric_vector() const;
    /// @returns true if this type is a vector of scalar type
    bool is_scalar_vector() const;
    /// @returns true if this type is a numeric scale or vector
    bool is_numeric_scalar_or_vector() const;
    /// @returns true if this type is a handle type
    bool is_handle() const;

    /// @returns true if this type is an abstract-numeric or if the type holds an element that is an
    /// abstract-numeric.
    bool HoldsAbstract() const;

    /// kNoConversion is returned from ConversionRank() when the implicit conversion is not
    /// permitted.
    static constexpr uint32_t kNoConversion = 0xffffffffu;

    /// ConversionRank returns the implicit conversion rank when attempting to convert `from` to
    /// `to`. Lower ranks are preferred over higher ranks.
    /// @param from the source type
    /// @param to the destination type
    /// @returns the rank value for converting from type `from` to type `to`, or #kNoConversion if
    /// the implicit conversion is not allowed.
    /// @see https://www.w3.org/TR/WGSL/#conversion-rank
    static uint32_t ConversionRank(const Type* from, const Type* to);

    /// @param ty the type to obtain the element type from
    /// @param count if not null, then this is assigned the number of child elements in the type.
    /// For example, the count of an `array<vec3<f32>, 5>` type would be 5.
    /// @returns
    ///   * the element type if `ty` is a vector or array
    ///   * the column type if `ty` is a matrix
    ///   * `ty` if `ty` is none of the above
    static const Type* ElementOf(const Type* ty, uint32_t* count = nullptr);

    /// @param ty the type to obtain the deepest element type from
    /// @param count if not null, then this is assigned the full number of most deeply nested
    /// elements in the type. For example, the count of an `array<vec3<f32>, 5>` type would be 15.
    /// @returns
    ///   * the element type if `ty` is a vector
    ///   * the matrix element type if `ty` is a matrix
    ///   * the deepest element type if `ty` is an array
    ///   * `ty` if `ty` is none of the above
    static const Type* DeepestElementOf(const Type* ty, uint32_t* count = nullptr);

    /// @param types the list of types
    /// @returns the lowest-ranking type that all types in `types` can be implicitly converted to,
    ///          or nullptr if there is no consistent common type across all types in `types`.
    /// @see https://www.w3.org/TR/WGSL/#conversion-rank
    static const Type* Common(utils::VectorRef<const Type*> types);

  protected:
    /// Constructor
    /// @param hash the immutable hash for the node
    /// @param flags the flags of this type
    Type(size_t hash, type::Flags flags);

    /// The flags of this type.
    const type::Flags flags_;
};

}  // namespace tint::type

namespace std {

/// std::hash specialization for tint::type::Type
template <>
struct hash<tint::type::Type> {
    /// @param type the type to obtain a hash from
    /// @returns the hash of the type
    size_t operator()(const tint::type::Type& type) const { return type.unique_hash; }
};

/// std::equal_to specialization for tint::type::Type
template <>
struct equal_to<tint::type::Type> {
    /// @param a the first type to compare
    /// @param b the second type to compare
    /// @returns true if the two types are equal
    bool operator()(const tint::type::Type& a, const tint::type::Type& b) const {
        return a.Equals(b);
    }
};

}  // namespace std

#endif  // SRC_TINT_TYPE_TYPE_H_
