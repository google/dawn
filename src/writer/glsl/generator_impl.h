// Copyright 2021 The Tint Authors.
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

#ifndef SRC_WRITER_GLSL_GENERATOR_IMPL_H_
#define SRC_WRITER_GLSL_GENERATOR_IMPL_H_

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "src/ast/assignment_statement.h"
#include "src/ast/bitcast_expression.h"
#include "src/ast/break_statement.h"
#include "src/ast/continue_statement.h"
#include "src/ast/discard_statement.h"
#include "src/ast/for_loop_statement.h"
#include "src/ast/if_statement.h"
#include "src/ast/loop_statement.h"
#include "src/ast/return_statement.h"
#include "src/ast/switch_statement.h"
#include "src/ast/unary_op_expression.h"
#include "src/program_builder.h"
#include "src/scope_stack.h"
#include "src/transform/decompose_memory_access.h"
#include "src/utils/hash.h"
#include "src/writer/text_generator.h"

namespace tint {

// Forward declarations
namespace sem {
class Call;
class Intrinsic;
class TypeConstructor;
class TypeConversion;
}  // namespace sem

namespace writer {
namespace glsl {

/// Implementation class for GLSL generator
class GeneratorImpl : public TextGenerator {
 public:
  /// Constructor
  /// @param program the program to generate
  explicit GeneratorImpl(const Program* program);
  ~GeneratorImpl();

  /// @returns true on successful generation; false otherwise
  bool Generate();

  /// Handles an index accessor expression
  /// @param out the output of the expression stream
  /// @param expr the expression to emit
  /// @returns true if the index accessor was emitted
  bool EmitIndexAccessor(std::ostream& out,
                         const ast::IndexAccessorExpression* expr);
  /// Handles an assignment statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitAssign(const ast::AssignmentStatement* stmt);
  /// Handles generating a binary expression
  /// @param out the output of the expression stream
  /// @param expr the binary expression
  /// @returns true if the expression was emitted, false otherwise
  bool EmitBinary(std::ostream& out, const ast::BinaryExpression* expr);
  /// Handles generating a bitcast expression
  /// @param out the output of the expression stream
  /// @param expr the expression
  /// @returns true if the binary expression was emitted
  bool EmitVectorRelational(std::ostream& out,
                            const ast::BinaryExpression* expr);
  /// Handles generating a vector relational expression
  /// @param out the output of the expression stream
  /// @param expr the expression
  /// @returns true if the vector relational expression was emitted
  bool EmitBitcast(std::ostream& out, const ast::BitcastExpression* expr);
  /// Emits a list of statements
  /// @param stmts the statement list
  /// @returns true if the statements were emitted successfully
  bool EmitStatements(const ast::StatementList& stmts);
  /// Emits a list of statements with an indentation
  /// @param stmts the statement list
  /// @returns true if the statements were emitted successfully
  bool EmitStatementsWithIndent(const ast::StatementList& stmts);
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
  /// Handles generating a function call expression
  /// @param out the output of the expression stream
  /// @param call the call expression
  /// @param function the function being called
  /// @returns true if the expression is emitted
  bool EmitFunctionCall(std::ostream& out,
                        const sem::Call* call,
                        const sem::Function* function);
  /// Handles generating an intrinsic call expression
  /// @param out the output of the expression stream
  /// @param call the call expression
  /// @param intrinsic the intrinsic being called
  /// @returns true if the expression is emitted
  bool EmitIntrinsicCall(std::ostream& out,
                         const sem::Call* call,
                         const sem::Intrinsic* intrinsic);
  /// Handles generating a type conversion expression
  /// @param out the output of the expression stream
  /// @param call the call expression
  /// @param conv the type conversion
  /// @returns true if the expression is emitted
  bool EmitTypeConversion(std::ostream& out,
                          const sem::Call* call,
                          const sem::TypeConversion* conv);
  /// Handles generating a type constructor expression
  /// @param out the output of the expression stream
  /// @param call the call expression
  /// @param ctor the type constructor
  /// @returns true if the expression is emitted
  bool EmitTypeConstructor(std::ostream& out,
                           const sem::Call* call,
                           const sem::TypeConstructor* ctor);
  /// Handles generating a barrier intrinsic call
  /// @param out the output of the expression stream
  /// @param intrinsic the semantic information for the barrier intrinsic
  /// @returns true if the call expression is emitted
  bool EmitBarrierCall(std::ostream& out, const sem::Intrinsic* intrinsic);
  /// Handles generating an atomic intrinsic call for a storage buffer variable
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the atomic intrinsic
  /// @returns true if the call expression is emitted
  bool EmitStorageAtomicCall(
      std::ostream& out,
      const ast::CallExpression* expr,
      const transform::DecomposeMemoryAccess::Intrinsic* intrinsic);
  /// Handles generating an atomic intrinsic call for a workgroup variable
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the atomic intrinsic
  /// @returns true if the call expression is emitted
  bool EmitWorkgroupAtomicCall(std::ostream& out,
                               const ast::CallExpression* expr,
                               const sem::Intrinsic* intrinsic);
  /// Handles generating a call to a texture function (`textureSample`,
  /// `textureSampleGrad`, etc)
  /// @param out the output of the expression stream
  /// @param call the call expression
  /// @param intrinsic the semantic information for the texture intrinsic
  /// @returns true if the call expression is emitted
  bool EmitTextureCall(std::ostream& out,
                       const sem::Call* call,
                       const sem::Intrinsic* intrinsic);
  /// Handles generating a call to the `select()` intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @returns true if the call expression is emitted
  bool EmitSelectCall(std::ostream& out, const ast::CallExpression* expr);
  /// Handles generating a call to the `dot()` intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @returns true if the call expression is emitted
  bool EmitDotCall(std::ostream& out,
                   const ast::CallExpression* expr,
                   const sem::Intrinsic* intrinsic);
  /// Handles generating a call to the `modf()` intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @returns true if the call expression is emitted
  bool EmitModfCall(std::ostream& out,
                    const ast::CallExpression* expr,
                    const sem::Intrinsic* intrinsic);
  /// Handles generating a call to the `frexp()` intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @returns true if the call expression is emitted
  bool EmitFrexpCall(std::ostream& out,
                     const ast::CallExpression* expr,
                     const sem::Intrinsic* intrinsic);
  /// Handles generating a call to the `isNormal()` intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the intrinsic
  /// @returns true if the call expression is emitted
  bool EmitIsNormalCall(std::ostream& out,
                        const ast::CallExpression* expr,
                        const sem::Intrinsic* intrinsic);
  /// Handles generating a call to data packing intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the texture intrinsic
  /// @returns true if the call expression is emitted
  bool EmitDataPackingCall(std::ostream& out,
                           const ast::CallExpression* expr,
                           const sem::Intrinsic* intrinsic);
  /// Handles generating a call to data unpacking intrinsic
  /// @param out the output of the expression stream
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the texture intrinsic
  /// @returns true if the call expression is emitted
  bool EmitDataUnpackingCall(std::ostream& out,
                             const ast::CallExpression* expr,
                             const sem::Intrinsic* intrinsic);
  /// Handles a case statement
  /// @param stmt the statement
  /// @returns true if the statement was emitted successfully
  bool EmitCase(const ast::CaseStatement* stmt);
  /// Handles generating a discard statement
  /// @param stmt the discard statement
  /// @returns true if the statement was successfully emitted
  bool EmitDiscard(const ast::DiscardStatement* stmt);
  /// Handles a continue statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitContinue(const ast::ContinueStatement* stmt);
  /// Handles generate an Expression
  /// @param out the output of the expression stream
  /// @param expr the expression
  /// @returns true if the expression was emitted
  bool EmitExpression(std::ostream& out, const ast::Expression* expr);
  /// Handles generating a function
  /// @param func the function to generate
  /// @returns true if the function was emitted
  bool EmitFunction(const ast::Function* func);

  /// Handles emitting a global variable
  /// @param global the global variable
  /// @returns true on success
  bool EmitGlobalVariable(const ast::Variable* global);

  /// Handles emitting a global variable with the uniform storage class
  /// @param var the global variable
  /// @returns true on success
  bool EmitUniformVariable(const sem::Variable* var);

  /// Handles emitting a global variable with the storage storage class
  /// @param var the global variable
  /// @returns true on success
  bool EmitStorageVariable(const sem::Variable* var);

  /// Handles emitting a global variable with the handle storage class
  /// @param var the global variable
  /// @returns true on success
  bool EmitHandleVariable(const sem::Variable* var);

  /// Handles emitting a global variable with the private storage class
  /// @param var the global variable
  /// @returns true on success
  bool EmitPrivateVariable(const sem::Variable* var);

  /// Handles emitting a global variable with the workgroup storage class
  /// @param var the global variable
  /// @returns true on success
  bool EmitWorkgroupVariable(const sem::Variable* var);

  /// Handles emitting the entry point function
  /// @param func the entry point
  /// @returns true if the entry point function was emitted
  bool EmitEntryPointFunction(const ast::Function* func);
  /// Handles an if statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitIf(const ast::IfStatement* stmt);
  /// Handles a literal
  /// @param out the output stream
  /// @param lit the literal to emit
  /// @returns true if the literal was successfully emitted
  bool EmitLiteral(std::ostream& out, const ast::LiteralExpression* lit);
  /// Handles a loop statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitLoop(const ast::LoopStatement* stmt);
  /// Handles a for loop statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitForLoop(const ast::ForLoopStatement* stmt);
  /// Handles generating an identifier expression
  /// @param out the output of the expression stream
  /// @param expr the identifier expression
  /// @returns true if the identifeir was emitted
  bool EmitIdentifier(std::ostream& out, const ast::IdentifierExpression* expr);
  /// Handles a member accessor expression
  /// @param out the output of the expression stream
  /// @param expr the member accessor expression
  /// @returns true if the member accessor was emitted
  bool EmitMemberAccessor(std::ostream& out,
                          const ast::MemberAccessorExpression* expr);
  /// Handles return statements
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitReturn(const ast::ReturnStatement* stmt);
  /// Handles statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitStatement(const ast::Statement* stmt);
  /// Handles generating a switch statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitSwitch(const ast::SwitchStatement* stmt);
  /// Handles generating type
  /// @param out the output stream
  /// @param type the type to generate
  /// @param storage_class the storage class of the variable
  /// @param access the access control type of the variable
  /// @param name the name of the variable, used for array emission.
  /// @param name_printed (optional) if not nullptr and an array was printed
  /// then the boolean is set to true.
  /// @returns true if the type is emitted
  bool EmitType(std::ostream& out,
                const sem::Type* type,
                ast::StorageClass storage_class,
                ast::Access access,
                const std::string& name,
                bool* name_printed = nullptr);
  /// Handles generating type and name
  /// @param out the output stream
  /// @param type the type to generate
  /// @param storage_class the storage class of the variable
  /// @param access the access control type of the variable
  /// @param name the name to emit
  /// @returns true if the type is emitted
  bool EmitTypeAndName(std::ostream& out,
                       const sem::Type* type,
                       ast::StorageClass storage_class,
                       ast::Access access,
                       const std::string& name);
  /// Handles generating a structure declaration
  /// @param buffer the text buffer that the type declaration will be written to
  /// @param ty the struct to generate
  /// @returns true if the struct is emitted
  bool EmitStructType(TextBuffer* buffer, const sem::Struct* ty);
  /// Handles generating the members of a structure
  /// @param buffer the text buffer that the struct members will be written to
  /// @param ty the struct to generate
  /// @returns true if the struct members are emitted
  bool EmitStructMembers(TextBuffer* buffer, const sem::Struct* ty);
  /// Handles a unary op expression
  /// @param out the output of the expression stream
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryOp(std::ostream& out, const ast::UnaryOpExpression* expr);
  /// Emits the zero value for the given type
  /// @param out the output stream
  /// @param type the type to emit the value for
  /// @returns true if the zero value was successfully emitted.
  bool EmitZeroValue(std::ostream& out, const sem::Type* type);
  /// Handles generating a variable
  /// @param var the variable to generate
  /// @returns true if the variable was emitted
  bool EmitVariable(const ast::Variable* var);
  /// Handles generating a program scope constant variable
  /// @param var the variable to emit
  /// @returns true if the variable was emitted
  bool EmitProgramConstVariable(const ast::Variable* var);
  /// Handles generating a builtin method name
  /// @param intrinsic the semantic info for the intrinsic
  /// @returns the name or "" if not valid
  std::string generate_builtin_name(const sem::Intrinsic* intrinsic);
  /// Converts a builtin to a gl_ string
  /// @param builtin the builtin to convert
  /// @param stage pipeline stage in which this builtin is used
  /// @returns the string name of the builtin or blank on error
  const char* builtin_to_string(ast::Builtin builtin, ast::PipelineStage stage);
  /// Converts a builtin to a sem::Type appropriate for GLSL.
  /// @param builtin the builtin to convert
  /// @returns the appropriate semantic type or null on error.
  sem::Type* builtin_type(ast::Builtin builtin);

  /// Converts interpolation attributes to a GLSL modifiers
  /// @param type the interpolation type
  /// @param sampling the interpolation sampling
  /// @returns the string name of the attribute or blank on error
  std::string interpolation_to_modifiers(
      ast::InterpolationType type,
      ast::InterpolationSampling sampling) const;

 private:
  enum class VarType { kIn, kOut };

  struct EntryPointData {
    std::string struct_name;
    std::string var_name;
  };

  struct DMAIntrinsic {
    transform::DecomposeMemoryAccess::Intrinsic::Op op;
    transform::DecomposeMemoryAccess::Intrinsic::DataType type;
    bool operator==(const DMAIntrinsic& rhs) const {
      return op == rhs.op && type == rhs.type;
    }
    /// Hasher is a std::hash function for DMAIntrinsic
    struct Hasher {
      /// @param i the DMAIntrinsic to hash
      /// @returns the hash of `i`
      inline std::size_t operator()(const DMAIntrinsic& i) const {
        return utils::Hash(i.op, i.type);
      }
    };
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
                           const ast::CallExpression* call,
                           const sem::Intrinsic* intrinsic,
                           F&& build);

  TextBuffer helpers_;  // Helper functions emitted at the top of the output
  std::function<bool()> emit_continuing_;
  std::unordered_map<DMAIntrinsic, std::string, DMAIntrinsic::Hasher>
      dma_intrinsics_;
  std::unordered_map<const sem::Intrinsic*, std::string> intrinsics_;
  std::unordered_map<const sem::Struct*, std::string> structure_builders_;
  std::unordered_map<const sem::Vector*, std::string> dynamic_vector_write_;
  std::unordered_map<const sem::Vector*, std::string> int_dot_funcs_;
};

}  // namespace glsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_GLSL_GENERATOR_IMPL_H_
