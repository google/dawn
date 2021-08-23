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

#include <string>
#include <unordered_map>

#include "src/ast/array_accessor_expression.h"
#include "src/ast/assignment_statement.h"
#include "src/ast/binary_expression.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/interpolate_decoration.h"
#include "src/ast/loop_statement.h"
#include "src/ast/member_accessor_expression.h"
#include "src/ast/return_statement.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/switch_statement.h"
#include "src/ast/type_constructor_expression.h"
#include "src/ast/unary_op_expression.h"
#include "src/program.h"
#include "src/scope_stack.h"
#include "src/sem/struct.h"
#include "src/writer/text_generator.h"

namespace tint {

// Forward declarations
namespace sem {
class Call;
class Intrinsic;
}  // namespace sem

namespace writer {
namespace msl {

/// Implementation class for MSL generator
class GeneratorImpl : public TextGenerator {
 public:
  /// Constructor
  /// @param program the program to generate
  explicit GeneratorImpl(const Program* program);
  ~GeneratorImpl();

  /// @returns true on successful generation; false otherwise
  bool Generate();

  /// @returns true if an invariant attribute was generated
  bool HasInvariant() { return has_invariant_; }

  /// Handles generating a declared type
  /// @param ty the declared type to generate
  /// @returns true if the declared type was emitted
  bool EmitTypeDecl(const sem::Type* ty);
  /// Handles an array accessor expression
  /// @param out the output of the expression stream
  /// @param expr the expression to emit
  /// @returns true if the array accessor was emitted
  bool EmitArrayAccessor(std::ostream& out, ast::ArrayAccessorExpression* expr);
  /// Handles an assignment statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitAssign(ast::AssignmentStatement* stmt);
  /// Handles generating a binary expression
  /// @param out the output of the expression stream
  /// @param expr the binary expression
  /// @returns true if the expression was emitted, false otherwise
  bool EmitBinary(std::ostream& out, ast::BinaryExpression* expr);
  /// Handles generating a bitcast expression
  /// @param out the output of the expression stream
  /// @param expr the bitcast expression
  /// @returns true if the bitcast was emitted
  bool EmitBitcast(std::ostream& out, ast::BitcastExpression* expr);
  /// Handles a block statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBlock(const ast::BlockStatement* stmt);
  /// Handles a break statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBreak(ast::BreakStatement* stmt);
  /// Handles generating a call expression
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @returns true if the call expression is emitted
  bool EmitCall(std::ostream& out, ast::CallExpression* expr);
  /// Handles generating an intrinsic call expression
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the intrinsic being called
  /// @returns true if the call expression is emitted
  bool EmitIntrinsicCall(std::ostream& out,
                         ast::CallExpression* expr,
                         const sem::Intrinsic* intrinsic);
  /// Handles generating a call to an atomic function (`atomicAdd`,
  /// `atomicMax`, etc)
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the atomic intrinsic
  /// @returns true if the call expression is emitted
  bool EmitAtomicCall(std::ostream& out,
                      ast::CallExpression* expr,
                      const sem::Intrinsic* intrinsic);
  /// Handles generating a call to a texture function (`textureSample`,
  /// `textureSampleGrad`, etc)
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the texture intrinsic
  /// @returns true if the call expression is emitted
  bool EmitTextureCall(std::ostream& out,
                       ast::CallExpression* expr,
                       const sem::Intrinsic* intrinsic);
  /// Handles generating a call to the `modf()` intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @returns true if the call expression is emitted
  bool EmitModfCall(std::ostream& out,
                    ast::CallExpression* expr,
                    const sem::Intrinsic* intrinsic);
  /// Handles generating a call to the `frexp()` intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @returns true if the call expression is emitted
  bool EmitFrexpCall(std::ostream& out,
                     ast::CallExpression* expr,
                     const sem::Intrinsic* intrinsic);
  /// Handles a case statement
  /// @param stmt the statement
  /// @returns true if the statement was emitted successfully
  bool EmitCase(ast::CaseStatement* stmt);
  /// Handles generating constructor expressions
  /// @param out the output of the expression stream
  /// @param expr the constructor expression
  /// @returns true if the expression was emitted
  bool EmitConstructor(std::ostream& out, ast::ConstructorExpression* expr);
  /// Handles a continue statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitContinue(ast::ContinueStatement* stmt);
  /// Handles generating a discard statement
  /// @param stmt the discard statement
  /// @returns true if the statement was successfully emitted
  bool EmitDiscard(ast::DiscardStatement* stmt);
  /// Handles emitting the entry point function
  /// @param func the entry point function
  /// @returns true if the entry point function was emitted
  bool EmitEntryPointFunction(ast::Function* func);
  /// Handles generate an Expression
  /// @param out the output of the expression stream
  /// @param expr the expression
  /// @returns true if the expression was emitted
  bool EmitExpression(std::ostream& out, ast::Expression* expr);
  /// Handles generating a function
  /// @param func the function to generate
  /// @returns true if the function was emitted
  bool EmitFunction(ast::Function* func);
  /// Handles generating an identifier expression
  /// @param out the output of the expression stream
  /// @param expr the identifier expression
  /// @returns true if the identifier was emitted
  bool EmitIdentifier(std::ostream& out, ast::IdentifierExpression* expr);
  /// Handles an if statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitIf(ast::IfStatement* stmt);
  /// Handles a literal
  /// @param out the output of the expression stream
  /// @param lit the literal to emit
  /// @returns true if the literal was successfully emitted
  bool EmitLiteral(std::ostream& out, ast::Literal* lit);
  /// Handles a loop statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitLoop(ast::LoopStatement* stmt);
  /// Handles a for loop statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitForLoop(ast::ForLoopStatement* stmt);
  /// Handles a member accessor expression
  /// @param out the output of the expression stream
  /// @param expr the member accessor expression
  /// @returns true if the member accessor was emitted
  bool EmitMemberAccessor(std::ostream& out,
                          ast::MemberAccessorExpression* expr);
  /// Handles return statements
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitReturn(ast::ReturnStatement* stmt);
  /// Handles generating a scalar constructor
  /// @param out the output of the expression stream
  /// @param expr the scalar constructor expression
  /// @returns true if the scalar constructor is emitted
  bool EmitScalarConstructor(std::ostream& out,
                             ast::ScalarConstructorExpression* expr);
  /// Handles emitting a pipeline stage name
  /// @param out the output of the expression stream
  /// @param stage the stage to emit
  void EmitStage(std::ostream& out, ast::PipelineStage stage);
  /// Handles statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitStatement(ast::Statement* stmt);
  /// Emits a list of statements
  /// @param stmts the statement list
  /// @returns true if the statements were emitted successfully
  bool EmitStatements(const ast::StatementList& stmts);
  /// Emits a list of statements with an indentation
  /// @param stmts the statement list
  /// @returns true if the statements were emitted successfully
  bool EmitStatementsWithIndent(const ast::StatementList& stmts);
  /// Handles generating a switch statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitSwitch(ast::SwitchStatement* stmt);
  /// Handles generating a type
  /// @param out the output of the type stream
  /// @param type the type to generate
  /// @param name the name of the variable, only used for array emission
  /// @param name_printed (optional) if not nullptr and an array was printed
  /// @returns true if the type is emitted
  bool EmitType(std::ostream& out,
                const sem::Type* type,
                const std::string& name,
                bool* name_printed = nullptr);
  /// Handles generating type and name
  /// @param out the output stream
  /// @param type the type to generate
  /// @param name the name to emit
  /// @returns true if the type is emitted
  bool EmitTypeAndName(std::ostream& out,
                       const sem::Type* type,
                       const std::string& name);
  /// Handles generating a storage class
  /// @param out the output of the type stream
  /// @param sc the storage class to generate
  /// @returns true if the storage class is emitted
  bool EmitStorageClass(std::ostream& out, ast::StorageClass sc);
  /// Handles generating an MSL-packed storage type.
  /// If the type does not have a packed form, the standard non-packed form is
  /// emitted.
  /// @param out the output of the type stream
  /// @param type the type to generate
  /// @param name the name of the variable, only used for array emission
  /// @returns true if the type is emitted
  bool EmitPackedType(std::ostream& out,
                      const sem::Type* type,
                      const std::string& name);
  /// Handles generating a struct declaration
  /// @param buffer the text buffer that the type declaration will be written to
  /// @param str the struct to generate
  /// @returns true if the struct is emitted
  bool EmitStructType(TextBuffer* buffer, const sem::Struct* str);
  /// Handles emitting a type constructor
  /// @param out the output of the expression stream
  /// @param expr the type constructor expression
  /// @returns true if the constructor is emitted
  bool EmitTypeConstructor(std::ostream& out,
                           ast::TypeConstructorExpression* expr);
  /// Handles a unary op expression
  /// @param out the output of the expression stream
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryOp(std::ostream& out, ast::UnaryOpExpression* expr);
  /// Handles generating a variable
  /// @param var the variable to generate
  /// @returns true if the variable was emitted
  bool EmitVariable(const sem::Variable* var);
  /// Handles generating a program scope constant variable
  /// @param var the variable to emit
  /// @returns true if the variable was emitted
  bool EmitProgramConstVariable(const ast::Variable* var);
  /// Emits the zero value for the given type
  /// @param out the output of the expression stream
  /// @param type the type to emit the value for
  /// @returns true if the zero value was successfully emitted.
  bool EmitZeroValue(std::ostream& out, const sem::Type* type);

  /// Handles generating a builtin name
  /// @param intrinsic the semantic info for the intrinsic
  /// @returns the name or "" if not valid
  std::string generate_builtin_name(const sem::Intrinsic* intrinsic);

  /// Converts a builtin to an attribute name
  /// @param builtin the builtin to convert
  /// @returns the string name of the builtin or blank on error
  std::string builtin_to_attribute(ast::Builtin builtin) const;

  /// Converts interpolation attributes to an MSL attribute
  /// @param type the interpolation type
  /// @param sampling the interpolation sampling
  /// @returns the string name of the attribute or blank on error
  std::string interpolation_to_attribute(
      ast::InterpolationType type,
      ast::InterpolationSampling sampling) const;

 private:
  // A pair of byte size and alignment `uint32_t`s.
  struct SizeAndAlign {
    uint32_t size;
    uint32_t align;
  };

  /// CallIntrinsicHelper will call the intrinsic helper function, creating it
  /// if it hasn't been built already. If the intrinsic needs to be built then
  /// CallIntrinsicHelper will generate the function signature and will call
  /// `build` to emit the body of the function.
  /// @param out the output of the expression stream
  /// @param call the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @param build a function with the signature:
  ///        `bool(TextBuffer* buffer, const std::vector<std::string>& params)`
  ///        Where:
  ///          `buffer` is the body of the generated function
  ///          `params` is the name of all the generated function parameters
  /// @returns true if the call expression is emitted
  template <typename F>
  bool CallIntrinsicHelper(std::ostream& out,
                           ast::CallExpression* call,
                           const sem::Intrinsic* intrinsic,
                           F&& build);

  TextBuffer helpers_;  // Helper functions emitted at the top of the output

  /// @returns the MSL packed type size and alignment in bytes for the given
  /// type.
  SizeAndAlign MslPackedTypeSizeAndAlign(const sem::Type* ty);

  using StorageClassToString =
      std::unordered_map<ast::StorageClass, std::string>;

  std::function<bool()> emit_continuing_;

  /// Name of atomicCompareExchangeWeak() helper for the given pointer storage
  /// class.
  StorageClassToString atomicCompareExchangeWeak_;

  /// True if an invariant attribute has been generated.
  bool has_invariant_ = false;

  /// True if matrix-packed_vector operator overloads have been generated.
  bool matrix_packed_vector_overloads_ = false;

  std::unordered_map<const sem::Intrinsic*, std::string> intrinsics_;
  std::unordered_map<const sem::Type*, std::string> unary_minus_funcs_;
};

}  // namespace msl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_MSL_GENERATOR_IMPL_H_
