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

#ifndef SRC_TINT_WRITER_WGSL_GENERATOR_IMPL_H_
#define SRC_TINT_WRITER_WGSL_GENERATOR_IMPL_H_

#include <string>

#include "src/tint/ast/assignment_statement.h"
#include "src/tint/ast/binary_expression.h"
#include "src/tint/ast/bitcast_expression.h"
#include "src/tint/ast/break_statement.h"
#include "src/tint/ast/compound_assignment_statement.h"
#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/discard_statement.h"
#include "src/tint/ast/fallthrough_statement.h"
#include "src/tint/ast/for_loop_statement.h"
#include "src/tint/ast/if_statement.h"
#include "src/tint/ast/index_accessor_expression.h"
#include "src/tint/ast/loop_statement.h"
#include "src/tint/ast/member_accessor_expression.h"
#include "src/tint/ast/return_statement.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/ast/unary_op_expression.h"
#include "src/tint/program.h"
#include "src/tint/sem/storage_texture.h"
#include "src/tint/sem/struct.h"
#include "src/tint/writer/text_generator.h"

namespace tint::writer::wgsl {

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

    /// Handles generating a enable directive
    /// @param ext the extension kind in the enable directive to generate
    /// @returns true if the enable directive was emitted
    bool EmitEnableDirective(const ast::Enable::ExtensionKind ext);
    /// Handles generating a declared type
    /// @param ty the declared type to generate
    /// @returns true if the declared type was emitted
    bool EmitTypeDecl(const ast::TypeDecl* ty);
    /// Handles an index accessor expression
    /// @param out the output of the expression stream
    /// @param expr the expression to emit
    /// @returns true if the index accessor was emitted
    bool EmitIndexAccessor(std::ostream& out, const ast::IndexAccessorExpression* expr);
    /// Handles an assignment statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitAssign(const ast::AssignmentStatement* stmt);
    /// Handles generating a binary expression
    /// @param out the output of the expression stream
    /// @param expr the binary expression
    /// @returns true if the expression was emitted, false otherwise
    bool EmitBinary(std::ostream& out, const ast::BinaryExpression* expr);
    /// Handles generating a binary operator
    /// @param out the output of the expression stream
    /// @param op the binary operator
    /// @returns true if the operator was emitted, false otherwise
    bool EmitBinaryOp(std::ostream& out, const ast::BinaryOp op);
    /// Handles generating a bitcast expression
    /// @param out the output of the expression stream
    /// @param expr the bitcast expression
    /// @returns true if the bitcast was emitted
    bool EmitBitcast(std::ostream& out, const ast::BitcastExpression* expr);
    /// Handles a block statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitBlock(const ast::BlockStatement* stmt);
    /// Handles a break statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitBreak(const ast::BreakStatement* stmt);
    /// Handles generating a call expression
    /// @param out the output of the expression stream
    /// @param expr the call expression
    /// @returns true if the call expression is emitted
    bool EmitCall(std::ostream& out, const ast::CallExpression* expr);
    /// Handles a case statement
    /// @param stmt the statement
    /// @returns true if the statment was emitted successfully
    bool EmitCase(const ast::CaseStatement* stmt);
    /// Handles a compound assignment statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt);
    /// Handles generating a literal expression
    /// @param out the output of the expression stream
    /// @param expr the literal expression expression
    /// @returns true if the literal expression is emitted
    bool EmitLiteral(std::ostream& out, const ast::LiteralExpression* expr);
    /// Handles a continue statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted successfully
    bool EmitContinue(const ast::ContinueStatement* stmt);
    /// Handles generate an Expression
    /// @param out the output of the expression stream
    /// @param expr the expression
    /// @returns true if the expression was emitted
    bool EmitExpression(std::ostream& out, const ast::Expression* expr);
    /// Handles generating a fallthrough statement
    /// @param stmt the fallthrough statement
    /// @returns true if the statement was successfully emitted
    bool EmitFallthrough(const ast::FallthroughStatement* stmt);
    /// Handles generating a function
    /// @param func the function to generate
    /// @returns true if the function was emitted
    bool EmitFunction(const ast::Function* func);
    /// Handles generating an identifier expression
    /// @param out the output of the expression stream
    /// @param expr the identifier expression
    /// @returns true if the identifier was emitted
    bool EmitIdentifier(std::ostream& out, const ast::IdentifierExpression* expr);
    /// Handles an if statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was successfully emitted
    bool EmitIf(const ast::IfStatement* stmt);
    /// Handles an increment/decrement statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was successfully emitted
    bool EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt);
    /// Handles generating a discard statement
    /// @param stmt the discard statement
    /// @returns true if the statement was successfully emitted
    bool EmitDiscard(const ast::DiscardStatement* stmt);
    /// Handles a loop statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emtited
    bool EmitLoop(const ast::LoopStatement* stmt);
    /// Handles a for-loop statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emtited
    bool EmitForLoop(const ast::ForLoopStatement* stmt);
    /// Handles a member accessor expression
    /// @param out the output of the expression stream
    /// @param expr the member accessor expression
    /// @returns true if the member accessor was emitted
    bool EmitMemberAccessor(std::ostream& out, const ast::MemberAccessorExpression* expr);
    /// Handles return statements
    /// @param stmt the statement to emit
    /// @returns true if the statement was successfully emitted
    bool EmitReturn(const ast::ReturnStatement* stmt);
    /// Handles statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitStatement(const ast::Statement* stmt);
    /// Handles a statement list
    /// @param stmts the statements to emit
    /// @returns true if the statements were emitted
    bool EmitStatements(const ast::StatementList& stmts);
    /// Handles a statement list with an increased indentation
    /// @param stmts the statements to emit
    /// @returns true if the statements were emitted
    bool EmitStatementsWithIndent(const ast::StatementList& stmts);
    /// Handles generating a switch statement
    /// @param stmt the statement to emit
    /// @returns true if the statement was emitted
    bool EmitSwitch(const ast::SwitchStatement* stmt);
    /// Handles generating type
    /// @param out the output of the expression stream
    /// @param type the type to generate
    /// @returns true if the type is emitted
    bool EmitType(std::ostream& out, const ast::Type* type);
    /// Handles generating a struct declaration
    /// @param str the struct
    /// @returns true if the struct is emitted
    bool EmitStructType(const ast::Struct* str);
    /// Handles emitting an image format
    /// @param out the output of the expression stream
    /// @param fmt the format to generate
    /// @returns true if the format is emitted
    bool EmitImageFormat(std::ostream& out, const ast::TexelFormat fmt);
    /// Handles emitting an access control
    /// @param out the output of the expression stream
    /// @param access the access to generate
    /// @returns true if the access is emitted
    bool EmitAccess(std::ostream& out, const ast::Access access);
    /// Handles a unary op expression
    /// @param out the output of the expression stream
    /// @param expr the expression to emit
    /// @returns true if the expression was emitted
    bool EmitUnaryOp(std::ostream& out, const ast::UnaryOpExpression* expr);
    /// Handles generating a variable
    /// @param out the output of the expression stream
    /// @param var the variable to generate
    /// @returns true if the variable was emitted
    bool EmitVariable(std::ostream& out, const ast::Variable* var);
    /// Handles generating a attribute list
    /// @param out the output of the expression stream
    /// @param attrs the attribute list
    /// @returns true if the attributes were emitted
    bool EmitAttributes(std::ostream& out, const ast::AttributeList& attrs);
};

}  // namespace tint::writer::wgsl

#endif  // SRC_TINT_WRITER_WGSL_GENERATOR_IMPL_H_
