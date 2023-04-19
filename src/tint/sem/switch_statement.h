// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_TINT_SEM_SWITCH_STATEMENT_H_
#define SRC_TINT_SEM_SWITCH_STATEMENT_H_

#include <vector>

#include "src/tint/sem/block_statement.h"

// Forward declarations
namespace tint::ast {
class CaseStatement;
class CaseSelector;
class SwitchStatement;
}  // namespace tint::ast
namespace tint::constant {
class Value;
}  // namespace tint::constant
namespace tint::sem {
class CaseStatement;
class CaseSelector;
class ValueExpression;
}  // namespace tint::sem

namespace tint::sem {

/// Holds semantic information about an switch statement
class SwitchStatement final : public utils::Castable<SwitchStatement, CompoundStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this switch statement
    /// @param parent the owning statement
    /// @param function the owning function
    SwitchStatement(const ast::SwitchStatement* declaration,
                    const CompoundStatement* parent,
                    const sem::Function* function);

    /// Destructor
    ~SwitchStatement() override;

    /// @return the AST node for this statement
    const ast::SwitchStatement* Declaration() const;

    /// @returns the case statements for this switch
    std::vector<const CaseStatement*>& Cases() { return cases_; }

    /// @returns the case statements for this switch
    const std::vector<const CaseStatement*>& Cases() const { return cases_; }

  private:
    std::vector<const CaseStatement*> cases_;
};

/// Holds semantic information about a switch case statement
class CaseStatement final : public utils::Castable<CaseStatement, CompoundStatement> {
  public:
    /// Constructor
    /// @param declaration the AST node for this case statement
    /// @param parent the owning statement
    /// @param function the owning function
    CaseStatement(const ast::CaseStatement* declaration,
                  const CompoundStatement* parent,
                  const sem::Function* function);

    /// Destructor
    ~CaseStatement() override;

    /// @return the AST node for this statement
    const ast::CaseStatement* Declaration() const;

    /// @param body the case body block statement
    void SetBlock(const BlockStatement* body) { body_ = body; }

    /// @returns the case body block statement
    const BlockStatement* Body() const { return body_; }

    /// @returns the selectors for the case
    std::vector<const CaseSelector*>& Selectors() { return selectors_; }

    /// @returns the selectors for the case
    const std::vector<const CaseSelector*>& Selectors() const { return selectors_; }

  private:
    const BlockStatement* body_ = nullptr;
    std::vector<const CaseSelector*> selectors_;
};

/// Holds semantic information about a switch case selector
class CaseSelector final : public utils::Castable<CaseSelector, Node> {
  public:
    /// Constructor
    /// @param decl the selector declaration
    /// @param val the case selector value, nullptr for a default selector
    explicit CaseSelector(const ast::CaseSelector* decl, const constant::Value* val = nullptr);

    /// Destructor
    ~CaseSelector() override;

    /// @returns true if this is a default selector
    bool IsDefault() const { return val_ == nullptr; }

    /// @returns the case selector declaration
    const ast::CaseSelector* Declaration() const;

    /// @returns the selector constant value, or nullptr if this is the default selector
    const constant::Value* Value() const { return val_; }

  private:
    const ast::CaseSelector* const decl_;
    const constant::Value* const val_;
};

}  // namespace tint::sem

#endif  // SRC_TINT_SEM_SWITCH_STATEMENT_H_
