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

#ifndef SRC_TINT_AST_DIAGNOSTIC_RULE_NAME_H_
#define SRC_TINT_AST_DIAGNOSTIC_RULE_NAME_H_

#include <string>

#include "src/tint/ast/node.h"

// Forward declarations
namespace tint::ast {
class Identifier;
}  // namespace tint::ast

namespace tint::ast {

/// A diagnostic rule name used for diagnostic directives and attributes.
class DiagnosticRuleName final : public utils::Castable<DiagnosticRuleName, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param name the rule name
    DiagnosticRuleName(ProgramID pid, NodeID nid, const Source& src, const Identifier* name);

    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param category the rule category.
    /// @param name the rule name
    DiagnosticRuleName(ProgramID pid,
                       NodeID nid,
                       const Source& src,
                       const Identifier* category,
                       const Identifier* name);

    /// Clones this node and all transitive child nodes using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const DiagnosticRuleName* Clone(CloneContext* ctx) const override;

    /// @return the full name of this diagnostic rule, either as `name` or `category.name`.
    std::string String() const;

    /// The diagnostic rule category (category.name)
    Identifier const* const category = nullptr;

    /// The diagnostic rule name.
    Identifier const* const name;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_DIAGNOSTIC_RULE_NAME_H_
