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

#ifndef SRC_TINT_LANG_WGSL_WRITER_AST_PRINTER_AST_PRINTER_H_
#define SRC_TINT_LANG_WGSL_WRITER_AST_PRINTER_AST_PRINTER_H_

#include <string>

#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/utils/generator/text_generator.h"
#include "src/tint/utils/text/string_stream.h"

// Forward declarations
namespace tint {
class Program;
}
namespace tint::ast {
class AssignmentStatement;
class Attribute;
class BinaryExpression;
enum class BinaryOp;
class BitcastExpression;
class BlockStatement;
class BlockStatement;
class BreakIfStatement;
class BreakStatement;
class CallExpression;
class CaseStatement;
class CompoundAssignmentStatement;
class ConstAssert;
class ContinueStatement;
struct DiagnosticControl;
class DiscardStatement;
class Enable;
class Expression;
class ForLoopStatement;
class Function;
class Identifier;
class IdentifierExpression;
class IfStatement;
class IncrementDecrementStatement;
class IndexAccessorExpression;
class LiteralExpression;
class LoopStatement;
class MemberAccessorExpression;
class ReturnStatement;
class Statement;
class Statement;
class Statement;
class Struct;
class SwitchStatement;
class TypeDecl;
class UnaryOpExpression;
class Variable;
class WhileStatement;
}  // namespace tint::ast

namespace tint::builtin {
enum class TexelFormat;
}  // namespace tint::builtin

namespace tint::wgsl::writer {

/// Implementation class for WGSL generator
class ASTPrinter : public tint::TextGenerator {
  public:
    /// Constructor
    /// @param program the program
    explicit ASTPrinter(const Program* program);
    ~ASTPrinter() override;

    /// Generates the result data
    void Generate();

    /// Handles generating a diagnostic control
    /// @param out the output stream
    /// @param diagnostic the diagnostic control node
    void EmitDiagnosticControl(StringStream& out, const ast::DiagnosticControl& diagnostic);
    /// Handles generating an enable directive
    /// @param enable the enable node
    void EmitEnable(const ast::Enable* enable);
    /// Handles generating a declared type
    /// @param ty the declared type to generate
    void EmitTypeDecl(const ast::TypeDecl* ty);
    /// Handles an index accessor expression
    /// @param out the output stream
    /// @param expr the expression to emit
    void EmitIndexAccessor(StringStream& out, const ast::IndexAccessorExpression* expr);
    /// Handles an assignment statement
    /// @param stmt the statement to emit
    void EmitAssign(const ast::AssignmentStatement* stmt);
    /// Handles generating a binary expression
    /// @param out the output stream
    /// @param expr the binary expression
    void EmitBinary(StringStream& out, const ast::BinaryExpression* expr);
    /// Handles generating a binary operator
    /// @param out the output stream
    /// @param op the binary operator
    void EmitBinaryOp(StringStream& out, const ast::BinaryOp op);
    /// Handles generating a bitcast expression
    /// @param out the output stream
    /// @param expr the bitcast expression
    void EmitBitcast(StringStream& out, const ast::BitcastExpression* expr);
    /// Handles a block statement
    /// @param stmt the statement to emit
    void EmitBlock(const ast::BlockStatement* stmt);
    /// Handles emitting the start of a block statement (including attributes)
    /// @param out the output stream to write the header to
    /// @param stmt the block statement to emit the header for
    void EmitBlockHeader(StringStream& out, const ast::BlockStatement* stmt);
    /// Handles a break statement
    /// @param stmt the statement to emit
    void EmitBreak(const ast::BreakStatement* stmt);
    /// Handles a break-if statement
    /// @param stmt the statement to emit
    void EmitBreakIf(const ast::BreakIfStatement* stmt);
    /// Handles generating a call expression
    /// @param out the output stream
    /// @param expr the call expression
    void EmitCall(StringStream& out, const ast::CallExpression* expr);
    /// Handles a case statement
    /// @param stmt the statement
    void EmitCase(const ast::CaseStatement* stmt);
    /// Handles a compound assignment statement
    /// @param stmt the statement to emit
    void EmitCompoundAssign(const ast::CompoundAssignmentStatement* stmt);
    /// Handles generating a literal expression
    /// @param out the output stream
    /// @param expr the literal expression expression
    void EmitLiteral(StringStream& out, const ast::LiteralExpression* expr);
    /// Handles a continue statement
    /// @param stmt the statement to emit
    void EmitContinue(const ast::ContinueStatement* stmt);
    /// Handles generate an Expression
    /// @param out the output stream
    /// @param expr the expression
    void EmitExpression(StringStream& out, const ast::Expression* expr);
    /// Handles generating a function
    /// @param func the function to generate
    void EmitFunction(const ast::Function* func);
    /// Handles generating an identifier expression
    /// @param out the output stream
    /// @param expr the identifier expression
    void EmitIdentifier(StringStream& out, const ast::IdentifierExpression* expr);
    /// Handles generating an identifier
    /// @param out the output of the expression stream
    /// @param ident the identifier
    void EmitIdentifier(StringStream& out, const ast::Identifier* ident);
    /// Handles an if statement
    /// @param stmt the statement to emit
    void EmitIf(const ast::IfStatement* stmt);
    /// Handles an increment/decrement statement
    /// @param stmt the statement to emit
    void EmitIncrementDecrement(const ast::IncrementDecrementStatement* stmt);
    /// Handles generating a discard statement
    /// @param stmt the discard statement
    void EmitDiscard(const ast::DiscardStatement* stmt);
    /// Handles a loop statement
    /// @param stmt the statement to emit
    void EmitLoop(const ast::LoopStatement* stmt);
    /// Handles a for-loop statement
    /// @param stmt the statement to emit
    void EmitForLoop(const ast::ForLoopStatement* stmt);
    /// Handles a while statement
    /// @param stmt the statement to emit
    void EmitWhile(const ast::WhileStatement* stmt);
    /// Handles a member accessor expression
    /// @param out the output stream
    /// @param expr the member accessor expression
    void EmitMemberAccessor(StringStream& out, const ast::MemberAccessorExpression* expr);
    /// Handles return statements
    /// @param stmt the statement to emit
    void EmitReturn(const ast::ReturnStatement* stmt);
    /// Handles const assertion statements
    /// @param stmt the statement to emit
    void EmitConstAssert(const ast::ConstAssert* stmt);
    /// Handles statement
    /// @param stmt the statement to emit
    void EmitStatement(const ast::Statement* stmt);
    /// Handles a statement list
    /// @param stmts the statements to emit
    void EmitStatements(VectorRef<const ast::Statement*> stmts);
    /// Handles a statement list with an increased indentation
    /// @param stmts the statements to emit
    void EmitStatementsWithIndent(VectorRef<const ast::Statement*> stmts);
    /// Handles generating a switch statement
    /// @param stmt the statement to emit
    void EmitSwitch(const ast::SwitchStatement* stmt);
    /// Handles generating a struct declaration
    /// @param str the struct
    void EmitStructType(const ast::Struct* str);
    /// Handles emitting an image format
    /// @param out the output stream
    /// @param fmt the format to generate
    void EmitImageFormat(StringStream& out, const builtin::TexelFormat fmt);
    /// Handles a unary op expression
    /// @param out the output stream
    /// @param expr the expression to emit
    void EmitUnaryOp(StringStream& out, const ast::UnaryOpExpression* expr);
    /// Handles generating a variable
    /// @param out the output stream
    /// @param var the variable to generate
    void EmitVariable(StringStream& out, const ast::Variable* var);
    /// Handles generating a attribute list
    /// @param out the output stream
    /// @param attrs the attribute list
    void EmitAttributes(StringStream& out, VectorRef<const ast::Attribute*> attrs);

  private:
    Program const* const program_;
};

}  // namespace tint::wgsl::writer

#endif  // SRC_TINT_LANG_WGSL_WRITER_AST_PRINTER_AST_PRINTER_H_
