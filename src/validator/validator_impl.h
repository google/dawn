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

#ifndef SRC_VALIDATOR_VALIDATOR_IMPL_H_
#define SRC_VALIDATOR_VALIDATOR_IMPL_H_

#include <string>
#include <unordered_set>

#include "src/ast/assignment_statement.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/ast/switch_statement.h"
#include "src/ast/variable_decl_statement.h"
#include "src/program.h"
#include "src/scope_stack.h"

namespace tint {

/// Determines if the program is complete and valid
class ValidatorImpl {
 public:
  /// Constructor
  /// @param program the program to validate
  explicit ValidatorImpl(const Program* program);
  ~ValidatorImpl();

  /// Runs the validator
  /// @returns true if the validation was successful
  bool Validate();

  /// @returns the program being validated
  const Program* program() { return program_; }

  /// @returns the diagnostic messages
  const diag::List& diagnostics() const { return diags_; }
  /// @returns the diagnostic messages
  diag::List& diagnostics() { return diags_; }

  /// @returns error messages from the validator
  std::string error() {
    diag::Formatter formatter{{false, false, false, false}};
    return formatter.format(diags_);
  }
  /// @returns true if an error was encountered
  bool has_error() const { return diags_.contains_errors(); }

  /// Appends an error at `src` with the code `code` and message `msg`
  /// @param src the source causing the error
  /// @param code the validation error code
  /// @param msg the error message
  void add_error(const Source& src, const char* code, const std::string& msg);

  /// Appends an error at `src` with the message `msg`
  /// @param src the source causing the error
  /// @param msg the error message
  void add_error(const Source& src, const std::string& msg);

  /// Validates a global variable
  /// @param var the global variable to check
  /// @returns true if the validation was successful
  bool ValidateGlobalVariable(const ast::Variable* var);
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
  /// @param assign the assignment to check
  /// @returns true if the validation was successful
  bool ValidateAssign(const ast::AssignmentStatement* assign);
  /// Validates a bad assignment to an identifier. Issues an error
  /// and returns false if the left hand side is an identifier.
  /// @param assign the assignment to check
  /// @returns true if the LHS of theassignment is not an identifier expression
  bool ValidateBadAssignmentToIdentifier(
      const ast::AssignmentStatement* assign);
  /// Validates declaration name uniqueness
  /// @param decl is the new declaration to be added
  /// @returns true if no previous declaration with the `decl` 's name
  /// exist in the variable stack
  bool ValidateDeclStatement(const ast::VariableDeclStatement* decl);
  /// Validates return statement
  /// @param ret the return statement to check
  /// @returns true if function return type matches the return statement type
  bool ValidateReturnStatement(const ast::ReturnStatement* ret);
  /// Validates switch statements
  /// @param s the switch statement to check
  /// @returns true if the valdiation was successful
  bool ValidateSwitch(const ast::SwitchStatement* s);
  /// Validates case statements
  /// @param c the case statement to check
  /// @returns true if the valdiation was successful
  bool ValidateCase(const ast::CaseStatement* c);
  /// Validates entry points
  /// @param funcs the functions to check
  /// @returns true if the valdiation was successful
  bool ValidateEntryPoint(const ast::FunctionList& funcs);
  /// Returns true if the given type is storable. This uses and
  /// updates `storable_` and `not_storable_`.
  /// @param type the given type
  /// @returns true if the given type is storable.
  bool IsStorable(type::Type* type);

  /// Testing method to inserting a given variable into the current scope.
  /// @param var the variable to register
  void RegisterVariableForTesting(ast::Variable* var) {
    variable_stack_.set(var->symbol(), var);
  }

 private:
  const Program* program_;
  diag::List diags_;
  ScopeStack<const ast::Variable*> variable_stack_;
  ast::Function* current_function_ = nullptr;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_VALIDATOR_IMPL_H_
