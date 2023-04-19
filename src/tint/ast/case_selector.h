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

#ifndef SRC_TINT_AST_CASE_SELECTOR_H_
#define SRC_TINT_AST_CASE_SELECTOR_H_

#include <vector>

#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/expression.h"

namespace tint::ast {

/// A case selector
class CaseSelector final : public utils::Castable<CaseSelector, Node> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param expr the selector expression, |nullptr| for a `default` selector
    CaseSelector(ProgramID pid, NodeID nid, const Source& src, const Expression* expr = nullptr);

    /// Destructor
    ~CaseSelector() override;

    /// @returns true if this is a default statement
    bool IsDefault() const { return expr == nullptr; }

    /// Clones this node and all transitive child nodes using the `CloneContext` `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const CaseSelector* Clone(CloneContext* ctx) const override;

    /// The selector, nullptr for a default selector
    const Expression* const expr = nullptr;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_CASE_SELECTOR_H_
