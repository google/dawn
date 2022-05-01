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
#include <vector>

#include "src/tint/ast/attribute.h"

// Forward declarations
namespace tint::ast {
class Type;
}  // namespace tint::ast

namespace tint::ast {

/// A struct member statement.
class StructMember final : public Castable<StructMember, Node> {
  public:
    /// Create a new struct member statement
    /// @param pid the identifier of the program that owns this node
    /// @param src the source of this node for the struct member statement
    /// @param sym The struct member symbol
    /// @param type The struct member type
    /// @param attributes The struct member attributes
    StructMember(ProgramID pid,
                 const Source& src,
                 const Symbol& sym,
                 const ast::Type* type,
                 AttributeList attributes);
    /// Move constructor
    StructMember(StructMember&&);

    ~StructMember() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const StructMember* Clone(CloneContext* ctx) const override;

    /// The symbol
    const Symbol symbol;

    /// The type
    const ast::Type* const type;

    /// The attributes
    const AttributeList attributes;
};

/// A list of struct members
using StructMemberList = std::vector<const StructMember*>;

}  // namespace tint::ast

#endif  // SRC_TINT_AST_STRUCT_MEMBER_H_
