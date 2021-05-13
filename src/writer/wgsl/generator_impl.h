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

#include <string>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/fallthrough_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/program.h"
#include "src/sem/storage_texture_type.h"
#include "src/sem/struct.h"
#include "src/writer/text_generator.h"

namespace tint {
namespace writer {
namespace wgsl {

/// Implementation class for WGSL generator
class GeneratorImpl : public TextGenerator {
 public:
  /// Constructor
  /// @param program the program
  explicit GeneratorImpl(const Program* program);
  ~GeneratorImpl();

  /// Generates the result data
  /// @returns true on successful generation; false otherwise
  bool Generate();

  /// Handles generating a constructed type
  /// @param ty the constructed to generate
  /// @returns true if the constructed was emitted
  bool EmitConstructedType(const ast::Type* ty);
  /// Handles an array accessor expression
  /// @param expr the expression to emit
  /// @returns true if the array accessor was emitted
  bool EmitArrayAccessor(ast::ArrayAccessorExpression* expr);
  /// Handles an assignment statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitAssign(ast::AssignmentStatement* stmt);
  /// Handles generating a binary expression
  /// @param expr the binary expression
  /// @returns true if the expression was emitted, false otherwise
  bool EmitBinary(ast::BinaryExpression* expr);
  /// Handles generating a bitcast expression
  /// @param expr the bitcast expression
  /// @returns true if the bitcast was emitted
  bool EmitBitcast(ast::BitcastExpression* expr);
  /// Handles a block statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBlock(const ast::BlockStatement* stmt);
  /// Handles a block statement with a newline at the end
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitIndentedBlockAndNewline(const ast::BlockStatement* stmt);
  /// Handles a block statement with a newline at the end
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBlockAndNewline(const ast::BlockStatement* stmt);
  /// Handles a break statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBreak(ast::BreakStatement* stmt);
  /// Handles generating a call expression
  /// @param expr the call expression
  /// @returns true if the call expression is emitted
  bool EmitCall(ast::CallExpression* expr);
  /// Handles a case statement
  /// @param stmt the statement
  /// @returns true if the statment was emitted successfully
  bool EmitCase(ast::CaseStatement* stmt);
  /// Handles generating a scalar constructor
  /// @param expr the scalar constructor expression
  /// @returns true if the scalar constructor is emitted
  bool EmitScalarConstructor(ast::ScalarConstructorExpression* expr);
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
  /// Handles generating a fallthrough statement
  /// @param stmt the fallthrough statement
  /// @returns true if the statement was successfully emitted
  bool EmitFallthrough(ast::FallthroughStatement* stmt);
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
  /// Handles generating constructor expressions
  /// @param expr the constructor expression
  /// @returns true if the expression was emitted
  bool EmitConstructor(ast::ConstructorExpression* expr);
  /// Handles generating a discard statement
  /// @param stmt the discard statement
  /// @returns true if the statement was successfully emitted
  bool EmitDiscard(ast::DiscardStatement* stmt);
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
  /// @returns true if the type is emitted
  bool EmitType(const ast::Type* type);
  /// Handles generating a struct declaration
  /// @param str the struct
  /// @returns true if the struct is emitted
  bool EmitStructType(const ast::Struct* str);
  /// Handles emitting an image format
  /// @param fmt the format to generate
  /// @returns true if the format is emitted
  bool EmitImageFormat(const ast::ImageFormat fmt);
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
  /// Handles generating a decoration list
  /// @param decos the decoration list
  /// @returns true if the decorations were emitted
  bool EmitDecorations(const ast::DecorationList& decos);

 private:
  /// @return a new, unique, valid WGSL identifier with the given suffix.
  std::string UniqueIdentifier(const std::string& suffix = "");

  Program const* const program_;
  uint32_t next_unique_identifier_suffix = 0;
};

}  // namespace wgsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_WGSL_GENERATOR_IMPL_H_
