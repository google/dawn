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
#include "src/ast/type/storage_texture_type.h"
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
class UnaryOpExpression;
class Variable;

}  // namespace ast

/// Determines types for all items in the given tint module
class TypeDeterminer {
 public:
  /// Constructor
  /// @param ctx the tint context
  /// @param mod the module to update with typing information
  TypeDeterminer(Context* ctx, ast::Module* mod);
  ~TypeDeterminer();

  /// @returns error messages from the type determiner
  const std::string& error() { return error_; }

  /// @returns true if the type determiner was successful
  bool Determine();
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
  bool DetermineStatements(const ast::BlockStatement* stmts);
  /// Determines type information for a statement
  /// @param stmt the statement to check
  /// @returns true if the determination was successful
  bool DetermineResultType(ast::Statement* stmt);
  /// Determines type information for an expression list
  /// @param list the expression list to check
  /// @returns true if the determination was successful
  bool DetermineResultType(const ast::ExpressionList& list);
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

  /// Retrieves information for the requested import.
  /// @param src the source of the import
  /// @param path the import path
  /// @param name the method name to get information on
  /// @param params the parameters to the method call
  /// @param id out parameter for the external call ID. Must not be a nullptr.
  /// @returns the return type of |name| in |path| or nullptr on error.
  ast::type::Type* GetImportData(const Source& src,
                                 const std::string& path,
                                 const std::string& name,
                                 const ast::ExpressionList& params,
                                 uint32_t* id);

 private:
  void set_error(const Source& src, const std::string& msg);
  void set_referenced_from_function_if_needed(ast::Variable* var);
  void set_entry_points(const std::string& fn_name, const std::string& ep_name);

  bool DetermineArrayAccessor(ast::ArrayAccessorExpression* expr);
  bool DetermineAs(ast::AsExpression* expr);
  bool DetermineBinary(ast::BinaryExpression* expr);
  bool DetermineCall(ast::CallExpression* expr);
  bool DetermineCast(ast::CastExpression* expr);
  bool DetermineConstructor(ast::ConstructorExpression* expr);
  bool DetermineIdentifier(ast::IdentifierExpression* expr);
  bool DetermineIntrinsic(const std::string& name, ast::CallExpression* expr);
  bool DetermineMemberAccessor(ast::MemberAccessorExpression* expr);
  bool DetermineUnaryOp(ast::UnaryOpExpression* expr);

  bool DetermineStorageTextureSubtype(ast::type::StorageTextureType* tex);

  Context& ctx_;
  ast::Module* mod_;
  std::string error_;
  ScopeStack<ast::Variable*> variable_stack_;
  std::unordered_map<std::string, ast::Function*> name_to_function_;
  ast::Function* current_function_ = nullptr;

  // Map from caller functions to callee functions.
  std::unordered_map<std::string, std::vector<std::string>> caller_to_callee_;
};

}  // namespace tint

#endif  // SRC_TYPE_DETERMINER_H_
