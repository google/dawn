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

#ifndef SRC_VALIDATOR_IMPL_H_
#define SRC_VALIDATOR_IMPL_H_

#include <string>

#include "src/ast/assignment_statement.h"
#include "src/ast/expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/statement.h"
#include "src/ast/variable.h"
#include "src/scope_stack.h"

namespace tint {

/// Determines if the module is complete and valid
class ValidatorImpl {
 public:
  /// Constructor
  ValidatorImpl();
  ~ValidatorImpl();

  /// Runs the validator
  /// @param module the module to validate
  /// @returns true if the validation was successful
  bool Validate(const ast::Module* module);

  /// @returns error messages from the validator
  const std::string& error() { return error_; }

  /// @returns true if an error was encountered
  bool has_error() const { return error_.size() > 0; }

  /// Sets the error string
  /// @param src the source causing the error
  /// @param msg the error message
  void set_error(const Source& src, const std::string& msg);
  /// Validates Functions
  /// @param funcs the functions to check
  /// @returns true if the validation was successful
  bool ValidateFunctions(const ast::FunctionList& funcs);
  /// Validates a function
  /// @param func the function to check
  /// @returns true if the validation was successful
  bool ValidateFunction(const ast::Function* func);
  /// Validates a block of statements
  /// @param block the statements to check
  /// @returns true if the validation was successful
  bool ValidateStatements(const ast::BlockStatement* block);
  /// Validates a statement
  /// @param stmt the statement to check
  /// @returns true if the validation was successful
  bool ValidateStatement(const ast::Statement* stmt);
  /// Validates an assignment
  /// @param a the assignment to check
  /// @returns true if the validation was successful
  bool ValidateAssign(const ast::AssignmentStatement* a);
  /// Validates v-0001: Only allowed import is "GLSL.std.450"
  /// @param module the modele to check imports
  /// @returns ture if input complies with v-0001 rule
  bool CheckImports(const ast::Module* module);
  /// Validates an expression
  /// @param expr the expression to check
  /// @return true if the expresssion is valid
  bool ValidateExpression(const ast::Expression* expr);
  /// Validates v-0006:Variables must be defined before use
  /// @param ident the identifer to check if its in the scope
  /// @return true if idnet was defined
  bool ValidateIdentifier(const ast::IdentifierExpression* ident);
  /// Validates if the input follows type checking rules
  /// @param 'a' the assignment to check
  /// @returns ture if successful
  bool ValidateResultTypes(const ast::AssignmentStatement* a);

 private:
  std::string error_;
  ScopeStack<ast::Variable*> variable_stack_;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_IMPL_H_
