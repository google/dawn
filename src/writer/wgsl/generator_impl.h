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

#ifndef SRC_WRITER_WGSL_GENERATOR_IMPL_H_
#define SRC_WRITER_WGSL_GENERATOR_IMPL_H_

#include <sstream>
#include <string>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/const_initializer_expression.h"
#include "src/ast/entry_point.h"
#include "src/ast/identifier_expression.h"
#include "src/ast/import.h"
#include "src/ast/initializer_expression.h"
#include "src/ast/module.h"
#include "src/ast/type/alias_type.h"
#include "src/ast/type/type.h"
#include "src/ast/type_initializer_expression.h"
#include "src/ast/variable.h"

namespace tint {
namespace writer {
namespace wgsl {

/// Implementation class for WGSL generator
class GeneratorImpl {
 public:
  /// Constructor
  GeneratorImpl();
  ~GeneratorImpl();

  /// Generates the result data
  /// @param module the module to generate
  /// @returns true on successful generation; false otherwise
  bool Generate(const ast::Module& module);

  /// @returns the result data
  std::string result() const { return out_.str(); }

  /// @returns the error from the generator
  std::string error() const { return error_; }

  /// Increment the emitter indent level
  void increment_indent() { indent_ += 2; }
  /// Decrement the emiter indent level
  void decrement_indent() {
    if (indent_ < 2) {
      indent_ = 0;
      return;
    }
    indent_ -= 2;
  }

  /// Writes the current indent to the output stream
  void make_indent();

  /// Handles generating an alias
  /// @param alias the alias to generate
  /// @returns true if the alias was emitted
  bool EmitAliasType(const ast::type::AliasType* alias);
  /// Handles an array accessor expression
  /// @param expr the expression to emit
  /// @returns true if the array accessor was emitted
  bool EmitArrayAccessor(ast::ArrayAccessorExpression* expr);
  /// Handles generating an as expression
  /// @param expr the as expression
  /// @returns true if the as was emitted
  bool EmitAs(ast::AsExpression* expr);
  /// Handles an assignment statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitAssign(ast::AssignmentStatement* stmt);
  /// Handles generating a call expression
  /// @param expr the call expression
  /// @returns true if the call expression is emitted
  bool EmitCall(ast::CallExpression* expr);
  /// Handles generating a cast expression
  /// @param expr the cast expression
  /// @returns true if the cast was emitted
  bool EmitCast(ast::CastExpression* expr);
  /// Handles generating a const initializer
  /// @param expr the const initializer expression
  /// @returns true if the initializer is emitted
  bool EmitConstInitializer(ast::ConstInitializerExpression* expr);
  /// Handles generating an entry_point command
  /// @param ep the entry point
  /// @returns true if the entry point was emitted
  bool EmitEntryPoint(const ast::EntryPoint* ep);
  /// Handles generate an Expression
  /// @param expr the expression
  /// @returns true if the expression was emitted
  bool EmitExpression(ast::Expression* expr);
  /// Handles generating an identifier expression
  /// @param expr the identifier expression
  /// @returns true if the identifeir was emitted
  bool EmitIdentifier(ast::IdentifierExpression* expr);
  /// Handles generating an import command
  /// @param import the import to generate
  /// @returns true if the import was emitted
  bool EmitImport(const ast::Import* import);
  /// Handles generating initializer expressions
  /// @param expr the initializer expression
  /// @returns true if the expression was emitted
  bool EmitInitializer(ast::InitializerExpression* expr);
  /// Handles a member accessor expression
  /// @param expr the member accessor expression
  /// @returns true if the member accessor was emitted
  bool EmitMemberAccessor(ast::MemberAccessorExpression* expr);
  /// Handles generating a relational expression
  /// @param expr the relational expression
  /// @returns true if the expression was emitted, false otherwise
  bool EmitRelational(ast::RelationalExpression* expr);
  /// Handles statements
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitStatement(ast::Statement* stmt);
  /// Handles generating type
  /// @param type the type to generate
  /// @returns true if the type is emitted
  bool EmitType(ast::type::Type* type);
  /// Handles emitting a type initializer
  /// @param expr the type initializer expression
  /// @returns true if the initializer is emitted
  bool EmitTypeInitializer(ast::TypeInitializerExpression* expr);
  /// Handles a unary derivative expression
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryDerivative(ast::UnaryDerivativeExpression* expr);
  /// Handles a unary method expression
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryMethod(ast::UnaryMethodExpression* expr);
  /// Handles a unary op expression
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryOp(ast::UnaryOpExpression* expr);
  /// Handles generating a variable
  /// @param var the variable to generate
  /// @returns true if the variable was emitted
  bool EmitVariable(ast::Variable* var);
  /// Handles generating variable decorations
  /// @param var the decorated variable
  /// @returns true if the variable decoration was emitted
  bool EmitVariableDecorations(ast::DecoratedVariable* var);

 private:
  size_t indent_ = 0;
  std::ostringstream out_;
  std::string error_;
};

}  // namespace wgsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_WGSL_GENERATOR_IMPL_H_
