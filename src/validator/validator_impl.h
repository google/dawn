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
#include <unordered_map>
#include <vector>

#include "src/ast/assignment_statement.h"
#include "src/ast/call_expression.h"
#include "src/ast/expression.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/module.h"
#include "src/ast/return_statement.h"
#include "src/ast/statement.h"
#include "src/ast/switch_statement.h"
#include "src/ast/variable.h"
#include "src/ast/variable_decl_statement.h"
#include "src/diagnostic/diagnostic.h"
#include "src/diagnostic/formatter.h"
#include "src/scope_stack.h"

namespace tint {

/// Determines if the module is complete and valid
class ValidatorImpl {
 public:
  /// Constructor
  /// @param module the module to validate
  explicit ValidatorImpl(const ast::Module* module);
  ~ValidatorImpl();

  /// Runs the validator
  /// @returns true if the validation was successful
  bool Validate();

  /// @returns the diagnostic messages
  const diag::List& diagnostics() const { return diags_; }
  /// @returns the diagnostic messages
  diag::List& diagnostics() { return diags_; }

  /// @returns error messages from the validator
  std::string error() {
    diag::Formatter formatter{{false, false, false}};
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

  /// Validate global variables
  /// @param global_vars list of global variables to check
  /// @returns true if the validation was successful
  bool ValidateGlobalVariables(const ast::VariableList& global_vars);
  /// Validates Functions
  /// @param funcs the functions to check
  /// @returns true if the validation was successful
  bool ValidateFunctions(const ast::FunctionList& funcs);
  /// Validates a function
  /// @param func the function to check
  /// @returns true if the validation was successful
  bool ValidateFunction(const ast::Function* func);
  /// Validates a function parameter
  /// @param param the function parameter to check
  /// @returns true if the validation was successful
  bool ValidateParameter(const ast::Variable* param);
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
  /// Validates an expression
  /// @param expr the expression to check
  /// @return true if the expression is valid
  bool ValidateExpression(const ast::Expression* expr);
  /// Validates v-0006:Variables must be defined before use
  /// @param ident the identifer to check if its in the scope
  /// @return true if idnet was defined
  bool ValidateIdentifier(const ast::IdentifierExpression* ident);
  /// Validates if the input follows type checking rules
  /// @param assign the assignment to check
  /// @returns ture if successful
  bool ValidateResultTypes(const ast::AssignmentStatement* assign);
  /// Validate v-0021: Cannot re-assign a constant
  /// @param assign is the assigment to check if its lhs is a const
  /// @returns false if lhs of assign is a constant identifier
  bool ValidateConstant(const ast::AssignmentStatement* assign);
  /// Validates declaration name uniqueness
  /// @param decl is the new declaration to be added
  /// @returns true if no previous declaration with the `decl` 's name
  /// exist in the variable stack
  bool ValidateDeclStatement(const ast::VariableDeclStatement* decl);
  /// Validates return statement
  /// @param ret the return statement to check
  /// @returns true if function return type matches the return statement type
  bool ValidateReturnStatement(const ast::ReturnStatement* ret);
  /// Validates function calls
  /// @param expr the call to validate
  /// @returns true if successful
  bool ValidateCallExpr(const ast::CallExpression* expr);
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

  /// Validates constructed types
  /// @param constructed_types the types to check
  /// @returns true if the valdiation was successful
  bool ValidateConstructedTypes(
      const std::vector<ast::type::Type*>& constructed_types);

 private:
  const ast::Module& module_;
  diag::List diags_;
  ScopeStack<ast::Variable*> variable_stack_;
  ScopeStack<ast::Function*> function_stack_;
  ast::Function* current_function_ = nullptr;
};

}  // namespace tint

#endif  // SRC_VALIDATOR_VALIDATOR_IMPL_H_
