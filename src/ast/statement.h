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

#ifndef SRC_AST_STATEMENT_H_
#define SRC_AST_STATEMENT_H_

#include <memory>
#include <vector>

#include "src/ast/node.h"

namespace tint {
namespace ast {

class AssignmentStatement;
class BreakStatement;
class CaseStatement;
class ContinueStatement;
class ElseStatement;
class FallthroughStatement;
class IfStatement;
class KillStatement;
class LoopStatement;
class NopStatement;
class ReturnStatement;
class SwitchStatement;
class UnlessStatement;
class VariableDeclStatement;

/// Base statement class
class Statement : public Node {
 public:
  ~Statement() override;

  /// @returns true if this is an assign statement
  virtual bool IsAssign() const;
  /// @returns true if this is a break statement
  virtual bool IsBreak() const;
  /// @returns true if this is a case statement
  virtual bool IsCase() const;
  /// @returns true if this is a continue statement
  virtual bool IsContinue() const;
  /// @returns true if this is an else statement
  virtual bool IsElse() const;
  /// @returns true if this is a fallthrough statement
  virtual bool IsFallthrough() const;
  /// @returns true if this is an if statement
  virtual bool IsIf() const;
  /// @returns true if this is a kill statement
  virtual bool IsKill() const;
  /// @returns true if this is a loop statement
  virtual bool IsLoop() const;
  /// @returns true if this is a nop statement
  virtual bool IsNop() const;
  /// @returns true if this is a return statement
  virtual bool IsReturn() const;
  /// @returns true if this is a switch statement
  virtual bool IsSwitch() const;
  /// @returns true if this is an unless statement
  virtual bool IsUnless() const;
  /// @returns true if this is an variable statement
  virtual bool IsVariableDecl() const;

  /// @returns the statement as a const assign statement
  const AssignmentStatement* AsAssign() const;
  /// @returns the statement as a const break statement
  const BreakStatement* AsBreak() const;
  /// @returns the statement as a const case statement
  const CaseStatement* AsCase() const;
  /// @returns the statement as a const continue statement
  const ContinueStatement* AsContinue() const;
  /// @returns the statement as a const else statement
  const ElseStatement* AsElse() const;
  /// @returns the statement as a const fallthrough statement
  const FallthroughStatement* AsFallthrough() const;
  /// @returns the statement as a const if statement
  const IfStatement* AsIf() const;
  /// @returns the statement as a const kill statement
  const KillStatement* AsKill() const;
  /// @returns the statement as a const loop statement
  const LoopStatement* AsLoop() const;
  /// @returns the statement as a const nop statement
  const NopStatement* AsNop() const;
  /// @returns the statement as a const return statement
  const ReturnStatement* AsReturn() const;
  /// @returns the statement as a const switch statement
  const SwitchStatement* AsSwitch() const;
  /// @returns the statement as a const unless statement
  const UnlessStatement* AsUnless() const;
  /// @returns the statement as a const variable statement
  const VariableDeclStatement* AsVariableDecl() const;

  /// @returns the statement as an assign statement
  AssignmentStatement* AsAssign();
  /// @returns the statement as a break statement
  BreakStatement* AsBreak();
  /// @returns the statement as a case statement
  CaseStatement* AsCase();
  /// @returns the statement as a continue statement
  ContinueStatement* AsContinue();
  /// @returns the statement as a else statement
  ElseStatement* AsElse();
  /// @returns the statement as a fallthrough statement
  FallthroughStatement* AsFallthrough();
  /// @returns the statement as a if statement
  IfStatement* AsIf();
  /// @returns the statement as a kill statement
  KillStatement* AsKill();
  /// @returns the statement as a loop statement
  LoopStatement* AsLoop();
  /// @returns the statement as a nop statement
  NopStatement* AsNop();
  /// @returns the statement as a return statement
  ReturnStatement* AsReturn();
  /// @returns the statement as a switch statement
  SwitchStatement* AsSwitch();
  /// @returns the statement as an unless statement
  UnlessStatement* AsUnless();
  /// @returns the statement as an variable statement
  VariableDeclStatement* AsVariableDecl();

 protected:
  /// Constructor
  Statement();
  /// Constructor
  /// @param source the source of the expression
  explicit Statement(const Source& source);
  /// Move constructor
  Statement(Statement&&);

 private:
  Statement(const Statement&) = delete;
};

/// A list of unique statements
using StatementList = std::vector<std::unique_ptr<Statement>>;

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_STATEMENT_H_
