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

#ifndef SRC_TYPE_DETERMINER_H_
#define SRC_TYPE_DETERMINER_H_

#include <string>
#include <unordered_map>

#include "src/ast/module.h"
#include "src/context.h"
#include "src/scope_stack.h"

namespace tint {
namespace ast {

class ArrayAccessorExpression;
class AsExpression;
class BinaryExpression;
class CallExpression;
class CastExpression;
class ConstructorExpression;
class Function;
class IdentifierExpression;
class MemberAccessorExpression;
class UnaryDerivativeExpression;
class UnaryMethodExpression;
class UnaryOpExpression;
class Variable;

}  // namespace ast

/// Determines types for all items in the given tint module
class TypeDeterminer {
 public:
  /// Constructor
  /// @param ctx the tint context
  explicit TypeDeterminer(Context* ctx);
  ~TypeDeterminer();

  /// @returns error messages from the type determiner
  const std::string& error() { return error_; }

  /// Runs the type determiner
  /// @param mod the module to update with typing information
  /// @returns true if the type determiner was successful
  bool Determine(ast::Module* mod);
  /// Determines type information for functions
  /// @param funcs the functions to check
  /// @returns true if the determination was successful
  bool DetermineFunctions(const ast::FunctionList& funcs);
  /// Determines type information for a function
  /// @param func the function to check
  /// @returns true if the determination was successful
  bool DetermineFunction(ast::Function* func);
  /// Determines type information for a set of statements
  /// @param stmts the statements to check
  /// @returns true if the determination was successful
  bool DetermineStatements(const ast::StatementList& stmts);
  /// Determines type information for a statement
  /// @param stmt the statement to check
  /// @returns true if the determination was successful
  bool DetermineResultType(ast::Statement* stmt);
  /// Determines type information for an expression
  /// @param expr the expression to check
  /// @returns true if the determination was successful
  bool DetermineResultType(ast::Expression* expr);
  /// Determines the storage class for variables. This assumes that it is only
  /// called for things in function scope, not module scope.
  /// @param stmt the statement to check
  /// @returns false on error
  bool DetermineVariableStorageClass(ast::Statement* stmt);

  /// Testing method to set a given variable into the type stack
  /// @param var the variable to set
  void RegisterVariableForTesting(ast::Variable* var) {
    variable_stack_.set(var->name(), var);
  }

 private:
  void set_error(const Source& src, const std::string& msg);

  bool DetermineArrayAccessor(ast::ArrayAccessorExpression* expr);
  bool DetermineAs(ast::AsExpression* expr);
  bool DetermineBinary(ast::BinaryExpression* expr);
  bool DetermineCall(ast::CallExpression* expr);
  bool DetermineCast(ast::CastExpression* expr);
  bool DetermineConstructor(ast::ConstructorExpression* expr);
  bool DetermineIdentifier(ast::IdentifierExpression* expr);
  bool DetermineMemberAccessor(ast::MemberAccessorExpression* expr);
  bool DetermineUnaryDerivative(ast::UnaryDerivativeExpression* expr);
  bool DetermineUnaryMethod(ast::UnaryMethodExpression* expr);
  bool DetermineUnaryOp(ast::UnaryOpExpression* expr);

  Context& ctx_;
  std::string error_;
  ScopeStack<ast::Variable*> variable_stack_;
  std::unordered_map<std::string, ast::Function*> name_to_function_;
};

}  // namespace tint

#endif  // SRC_TYPE_DETERMINER_H_
