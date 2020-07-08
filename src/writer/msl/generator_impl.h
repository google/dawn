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

#ifndef SRC_WRITER_MSL_GENERATOR_IMPL_H_
#define SRC_WRITER_MSL_GENERATOR_IMPL_H_

#include <sstream>
#include <string>

#include "src/ast/literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type_constructor_expression.h"
#include "src/writer/msl/namer.h"
#include "src/writer/text_generator.h"

namespace tint {
namespace writer {
namespace msl {

/// Implementation class for WGSL generator
class GeneratorImpl : public TextGenerator {
 public:
  /// Constructor
  GeneratorImpl();
  ~GeneratorImpl();

  /// Generates the result data
  /// @param module the module to generate
  /// @returns true on successful generation; false otherwise
  bool Generate(const ast::Module& module);

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
  /// Handles generating a binary expression
  /// @param expr the binary expression
  /// @returns true if the expression was emitted, false otherwise
  bool EmitBinary(ast::BinaryExpression* expr);
  /// Handles a break statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBreak(ast::BreakStatement* stmt);
  /// Handles a case statement
  /// @param stmt the statement
  /// @returns true if the statment was emitted successfully
  bool EmitCase(ast::CaseStatement* stmt);
  /// Handles generating a cast expression
  /// @param expr the cast expression
  /// @returns true if the cast was emitted
  bool EmitCast(ast::CastExpression* expr);
  /// Handles generating constructor expressions
  /// @param expr the constructor expression
  /// @returns true if the expression was emitted
  bool EmitConstructor(ast::ConstructorExpression* expr);
  /// Handles a continue statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitContinue(ast::ContinueStatement* stmt);
  /// Handles generating an else statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitElse(ast::ElseStatement* stmt);
  /// Handles generate an Expression
  /// @param expr the expression
  /// @returns true if the expression was emitted
  bool EmitExpression(ast::Expression* expr);
  /// Handles generating a function
  /// @param func the function to generate
  /// @returns true if the function was emitted
  bool EmitFunction(ast::Function* func);
  /// Handles generating an identifier expression
  /// @param expr the identifier expression
  /// @returns true if the identifeir was emitted
  bool EmitIdentifier(ast::IdentifierExpression* expr);
  /// Handles an if statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitIf(ast::IfStatement* stmt);
  /// Handles generating a kill statement
  /// @param stmt the kill statement
  /// @returns true if the statement was successfully emitted
  bool EmitKill(ast::KillStatement* stmt);
  /// Handles a literal
  /// @param lit the literal to emit
  /// @returns true if the literal was successfully emitted
  bool EmitLiteral(ast::Literal* lit);
  /// Handles a loop statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emtited
  bool EmitLoop(ast::LoopStatement* stmt);
  /// Handles a member accessor expression
  /// @param expr the member accessor expression
  /// @returns true if the member accessor was emitted
  bool EmitMemberAccessor(ast::MemberAccessorExpression* expr);
  /// Handles return statements
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitReturn(ast::ReturnStatement* stmt);
  /// Handles generating a scalar constructor
  /// @param expr the scalar constructor expression
  /// @returns true if the scalar constructor is emitted
  bool EmitScalarConstructor(ast::ScalarConstructorExpression* expr);
  /// Handles emitting a pipeline stage name
  /// @param stage the stage to emit
  void EmitStage(ast::PipelineStage stage);
  /// Handles a brace-enclosed list of statements.
  /// @param statements the statements to output
  /// @returns true if the statements were emitted
  bool EmitStatementBlock(const ast::StatementList& statements);
  /// Handles a brace-enclosed list of statements and trailing newline.
  /// @param statements the statements to output
  /// @returns true if the statements were emitted
  bool EmitStatementBlockAndNewline(const ast::StatementList& statements);
  /// Handles statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitStatement(ast::Statement* stmt);
  /// Handles generating a switch statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitSwitch(ast::SwitchStatement* stmt);
  /// Handles generating type
  /// @param type the type to generate
  /// @param name the name of the variable, only used for array emission
  /// @returns true if the type is emitted
  bool EmitType(ast::type::Type* type, const std::string& name);
  /// Handles emitting a type constructor
  /// @param expr the type constructor expression
  /// @returns true if the constructor is emitted
  bool EmitTypeConstructor(ast::TypeConstructorExpression* expr);
  /// Handles a unary op expression
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryOp(ast::UnaryOpExpression* expr);
  /// Handles generating a variable
  /// @param var the variable to generate
  /// @returns true if the variable was emitted
  bool EmitVariable(ast::Variable* var);
  /// Emits the zero value for the given type
  /// @param type the type to emit the value for
  /// @returns true if the zero value was successfully emitted.
  bool EmitZeroValue(ast::type::Type* type);

 private:
  Namer namer_;
  const ast::Module* module_ = nullptr;
  uint32_t loop_emission_counter_ = 0;
};

}  // namespace msl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_MSL_GENERATOR_IMPL_H_
