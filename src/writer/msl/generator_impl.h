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

  /// Handles generating a constructed
  /// @param ty the constructed type to generate
  /// @returns true if the constructed type was emitted
  bool EmitConstructedType(const sem::Type* ty);
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
  bool EmitIndentedBlockAndNewline(ast::BlockStatement* stmt);
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
  /// Handles generating a call to a texture function (`textureSample`,
  /// `textureSampleGrad`, etc)
  /// @param expr the call expression
  /// @param intrinsic the semantic information for the texture intrinsic
  /// @returns true if the call expression is emitted
  bool EmitTextureCall(ast::CallExpression* expr,
                       const sem::Intrinsic* intrinsic);
  /// Handles a case statement
  /// @param stmt the statement
  /// @returns true if the statement was emitted successfully
  bool EmitCase(ast::CaseStatement* stmt);
  /// Handles generating constructor expressions
  /// @param expr the constructor expression
  /// @returns true if the expression was emitted
  bool EmitConstructor(ast::ConstructorExpression* expr);
  /// Handles a continue statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitContinue(ast::ContinueStatement* stmt);
  /// Handles generating a discard statement
  /// @param stmt the discard statement
  /// @returns true if the statement was successfully emitted
  bool EmitDiscard(ast::DiscardStatement* stmt);
  /// Handles generating an else statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitElse(ast::ElseStatement* stmt);
  /// Handles emitting information for an entry point
  /// @param func the entry point function
  /// @returns true if the entry point data was emitted
  bool EmitEntryPointData(ast::Function* func);
  /// Handles emitting the entry point function
  /// @param func the entry point function
  /// @returns true if the entry point function was emitted
  bool EmitEntryPointFunction(ast::Function* func);
  /// Handles generate an Expression
  /// @param expr the expression
  /// @returns true if the expression was emitted
  bool EmitExpression(ast::Expression* expr);
  /// Handles generating a function
  /// @param func the function to generate
  /// @returns true if the function was emitted
  bool EmitFunction(ast::Function* func);
  /// Internal helper for emitting functions
  /// @param func the function to emit
  /// @param emit_duplicate_functions set true if we need to duplicate per entry
  /// point
  /// @param ep_sym the current entry point or symbol::kInvalid if not set
  /// @returns true if the function was emitted.
  bool EmitFunctionInternal(ast::Function* func,
                            bool emit_duplicate_functions,
                            Symbol ep_sym);
  /// Handles generating an identifier expression
  /// @param expr the identifier expression
  /// @returns true if the identifier was emitted
  bool EmitIdentifier(ast::IdentifierExpression* expr);
  /// Handles an if statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitIf(ast::IfStatement* stmt);
  /// Handles a literal
  /// @param lit the literal to emit
  /// @returns true if the literal was successfully emitted
  bool EmitLiteral(ast::Literal* lit);
  /// Handles a loop statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
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
  /// Handles statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitStatement(ast::Statement* stmt);
  /// Handles generating a switch statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitSwitch(ast::SwitchStatement* stmt);
  /// Handles generating a type
  /// @param type the type to generate
  /// @param name the name of the variable, only used for array emission
  /// @returns true if the type is emitted
  bool EmitType(const sem::Type* type, const std::string& name);
  /// Handles generating an MSL-packed storage type.
  /// If the type does not have a packed form, the standard non-packed form is
  /// emitted.
  /// @param type the type to generate
  /// @param name the name of the variable, only used for array emission
  /// @returns true if the type is emitted
  bool EmitPackedType(const sem::Type* type, const std::string& name);
  /// Handles generating a struct declaration
  /// @param str the struct to generate
  /// @returns true if the struct is emitted
  bool EmitStructType(const sem::Struct* str);
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
  /// @param skip_constructor set true if the constructor should be skipped
  /// @returns true if the variable was emitted
  bool EmitVariable(const sem::Variable* var, bool skip_constructor);
  /// Handles generating a program scope constant variable
  /// @param var the variable to emit
  /// @returns true if the variable was emitted
  bool EmitProgramConstVariable(const ast::Variable* var);
  /// Emits the zero value for the given type
  /// @param type the type to emit the value for
  /// @returns true if the zero value was successfully emitted.
  bool EmitZeroValue(const sem::Type* type);

  /// Determines if the function needs the input struct passed to it.
  /// @param func the function to check
  /// @returns true if there are input struct variables used in the function
  bool has_referenced_in_var_needing_struct(ast::Function* func);
  /// Determines if the function needs the output struct passed to it.
  /// @param func the function to check
  /// @returns true if there are output struct variables used in the function
  bool has_referenced_out_var_needing_struct(ast::Function* func);
  /// Determines if any used module variable requires an input or output struct.
  /// @param func the function to check
  /// @returns true if an input or output struct is required.
  bool has_referenced_var_needing_struct(ast::Function* func);

  /// Handles generating a builtin name
  /// @param intrinsic the semantic info for the intrinsic
  /// @returns the name or "" if not valid
  std::string generate_builtin_name(const sem::Intrinsic* intrinsic);

  /// Checks if the global variable is in an input or output struct
  /// @param var the variable to check
  /// @returns true if the global is in an input or output struct
  bool global_is_in_struct(const sem::Variable* var) const;

  /// Converts a builtin to an attribute name
  /// @param builtin the builtin to convert
  /// @returns the string name of the builtin or blank on error
  std::string builtin_to_attribute(ast::Builtin builtin) const;

 private:
  enum class VarType { kIn, kOut };

  struct EntryPointData {
    std::string struct_name;
    std::string var_name;
  };

  std::string current_ep_var_name(VarType type);

  /// @returns the resolved type of the ast::Expression `expr`
  /// @param expr the expression
  sem::Type* TypeOf(ast::Expression* expr) const {
    return program_->TypeOf(expr);
  }

  /// @returns the resolved type of the ast::Type `type`
  /// @param type the type
  const sem::Type* TypeOf(const ast::Type* type) const {
    return program_->TypeOf(type);
  }

  // A pair of byte size and alignment `uint32_t`s.
  struct SizeAndAlign {
    uint32_t size;
    uint32_t align;
  };

  /// @returns the MSL packed type size and alignment in bytes for the given
  /// type.
  SizeAndAlign MslPackedTypeSizeAndAlign(const sem::Type* ty);

  ScopeStack<const sem::Variable*> global_variables_;
  Symbol current_ep_sym_;
  bool generating_entry_point_ = false;
  const Program* program_ = nullptr;
  uint32_t loop_emission_counter_ = 0;

  std::unordered_map<Symbol, EntryPointData> ep_sym_to_in_data_;
  std::unordered_map<Symbol, EntryPointData> ep_sym_to_out_data_;

  // This maps an input of "<entry_point_name>_<function_name>" to a remapped
  // function name. If there is no entry for a given key then function did
  // not need to be remapped for the entry point and can be emitted directly.
  std::unordered_map<std::string, std::string> ep_func_name_remapped_;
};

}  // namespace msl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_MSL_GENERATOR_IMPL_H_
