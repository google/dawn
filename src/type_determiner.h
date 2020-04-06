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

class Function;
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
  bool DetermineResultType(const ast::StatementList& stmts);
  /// Determines type information for a statement
  /// @param stmt the statement to check
  /// @returns true if the determination was successful
  bool DetermineResultType(ast::Statement* stmt);
  /// Determines type information for an expression
  /// @param expr the expression to check
  /// @returns true if the determination was successful
  bool DetermineResultType(ast::Expression* expr);

 private:
  Context& ctx_;
  std::string error_;
  ScopeStack<ast::Variable*> variable_stack_;
  std::unordered_map<std::string, ast::Function*> name_to_function_;
};

}  // namespace tint

#endif  // SRC_TYPE_DETERMINER_H_
