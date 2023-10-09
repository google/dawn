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

#ifndef SRC_TINT_LANG_WGSL_RESOLVER_INCOMPLETE_TYPE_H_
#define SRC_TINT_LANG_WGSL_RESOLVER_INCOMPLETE_TYPE_H_

#include <string>

#include "src/tint/lang/core/builtin_type.h"
#include "src/tint/lang/core/type/type.h"

namespace tint::resolver {

/// Represents a expression that resolved to the name of a type without explicit template arguments.
/// This is a placeholder type that on successful resolving, is replaced with the type built using
/// the inferred template arguments.
/// For example, given the expression `vec3(1i, 2i, 3i)`:
/// * The IdentifierExpression for `vec3` is resolved first, to an IncompleteType as it does not
///   know the vector element type.
/// * Next, the CallExpression replaces the IncompleteType with a core::type::Vector once it can
///   infer the element type from the call's arguments.
class IncompleteType : public Castable<IncompleteType, core::type::Type> {
  public:
    /// Constructor
    /// @param b the incomplete builtin type
    explicit IncompleteType(core::BuiltinType b);

    /// Destructor
    ~IncompleteType() override;

    /// The incomplete builtin type
    const core::BuiltinType builtin = core::BuiltinType::kUndefined;

    /// @copydoc core::type::Type::FriendlyName
    std::string FriendlyName() const override;

    /// @copydoc core::type::Type::Size
    uint32_t Size() const override;

    /// @copydoc core::type::Type::Align
    uint32_t Align() const override;

    /// @copydoc core::type::Type::Clone
    core::type::Type* Clone(core::type::CloneContext& ctx) const override;

    /// @copydoc core::type::Type::Elements
    core::type::TypeAndCount Elements(const Type* type_if_invalid = nullptr,
                                      uint32_t count_if_invalid = 0) const override;

    /// @copydoc core::type::Type::Element
    const Type* Element(uint32_t index) const override;

    /// @copydoc core::type::UniqueNode::Equals
    bool Equals(const UniqueNode& other) const override;
};

}  // namespace tint::resolver

#endif  // SRC_TINT_LANG_WGSL_RESOLVER_INCOMPLETE_TYPE_H_
