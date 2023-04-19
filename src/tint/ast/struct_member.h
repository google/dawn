// Copyright 2020 The Tint Authors.
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

#ifndef SRC_TINT_AST_STRUCT_MEMBER_H_
#define SRC_TINT_AST_STRUCT_MEMBER_H_

#include <utility>

#include "src/tint/ast/attribute.h"
#include "src/tint/ast/type.h"

// Forward declarations
namespace tint::ast {
class Identifier;
}  // namespace tint::ast

namespace tint::ast {

/// A struct member statement.
class StructMember final : public utils::Castable<StructMember, Node> {
  public:
    /// Create a new struct member statement
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node for the struct member statement
    /// @param name The struct member name
    /// @param type The struct member type
    /// @param attributes The struct member attributes
    StructMember(ProgramID pid,
                 NodeID nid,
                 const Source& src,
                 const Identifier* name,
                 Type type,
                 utils::VectorRef<const Attribute*> attributes);

    /// Destructor
    ~StructMember() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const StructMember* Clone(CloneContext* ctx) const override;

    /// The member name
    const Identifier* const name;

    /// The type
    const Type type;

    /// The attributes
    const utils::Vector<const Attribute*, 4> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_STRUCT_MEMBER_H_
