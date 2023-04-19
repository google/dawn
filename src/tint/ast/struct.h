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

#ifndef SRC_TINT_AST_STRUCT_H_
#define SRC_TINT_AST_STRUCT_H_

#include <string>
#include <utility>

#include "src/tint/ast/attribute.h"
#include "src/tint/ast/struct_member.h"
#include "src/tint/ast/type_decl.h"
#include "src/tint/utils/vector.h"

namespace tint::ast {

/// A struct statement.
class Struct final : public utils::Castable<Struct, TypeDecl> {
  public:
    /// Create a new struct statement
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node for the import statement
    /// @param name The name of the structure
    /// @param members The struct members
    /// @param attributes The struct attributes
    Struct(ProgramID pid,
           NodeID nid,
           const Source& src,
           const Identifier* name,
           utils::VectorRef<const StructMember*> members,
           utils::VectorRef<const Attribute*> attributes);

    /// Destructor
    ~Struct() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const Struct* Clone(CloneContext* ctx) const override;

    /// The members
    const utils::Vector<const StructMember*, 8> members;

    /// The struct attributes
    const utils::Vector<const Attribute*, 4> attributes;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_STRUCT_H_
