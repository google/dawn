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

#ifndef SRC_TINT_AST_CASE_STATEMENT_H_
#define SRC_TINT_AST_CASE_STATEMENT_H_

#include <vector>

#include "src/tint/ast/block_statement.h"
#include "src/tint/ast/case_selector.h"

namespace tint::ast {

/// A case statement
class CaseStatement final : public utils::Castable<CaseStatement, Statement> {
  public:
    /// Constructor
    /// @param pid the identifier of the program that owns this node
    /// @param nid the unique node identifier
    /// @param src the source of this node
    /// @param selectors the case selectors
    /// @param body the case body
    CaseStatement(ProgramID pid,
                  NodeID nid,
                  const Source& src,
                  utils::VectorRef<const CaseSelector*> selectors,
                  const BlockStatement* body);

    /// Destructor
    ~CaseStatement() override;

    /// Clones this node and all transitive child nodes using the `CloneContext`
    /// `ctx`.
    /// @param ctx the clone context
    /// @return the newly cloned node
    const CaseStatement* Clone(CloneContext* ctx) const override;

    /// @returns true if this item contains a default selector
    bool ContainsDefault() const;

    /// The case selectors, empty if none set
    const utils::Vector<const CaseSelector*, 4> selectors;

    /// The case body
    const BlockStatement* const body;
};

}  // namespace tint::ast

#endif  // SRC_TINT_AST_CASE_STATEMENT_H_
