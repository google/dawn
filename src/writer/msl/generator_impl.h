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
#include <unordered_map>

#include "src/ast/literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type/struct_type.h"
#include "src/ast/type_constructor_expression.h"
#include "src/scope_stack.h"
#include "src/writer/msl/namer.h"
#include "src/writer/text_generator.h"

namespace tint {
namespace writer {
namespace msl {

/// Implementation class for MSL generator
class GeneratorImpl : public TextGenerator {
 public:
  /// Constructor
  /// @param module the module to generate
  explicit GeneratorImpl(ast::Module* module);
  ~GeneratorImpl();

  /// @returns true on successful generation; false otherwise
  bool Generate();

  /// Calculates the alignment size of the given |type|. This returns 0
  /// for pointers as the size is unknown.
  /// @param type the type to calculate the alignment size for
  /// @returns the number of bytes used to align |type| or 0 on error
  uint32_t calculate_alignment_size(ast::type::Type* type);
  /// Calculates the largest alignment seen within a struct
  /// @param type the struct to calculate
  /// @returns the largest alignment value
  uint32_t calculate_largest_alignment(ast::type::StructType* type);

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
  /// Handles a case statement
  /// @param stmt the statement
  /// @returns true if the statement was emitted successfully
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
  /// Handles generating a discard statement
  /// @param stmt the discard statement
  /// @returns true if the statement was successfully emitted
  bool EmitDiscard(ast::DiscardStatement* stmt);
  /// Handles generating an else statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitElse(ast::ElseStatement* stmt);
  /// Handles emitting information for an entry point
  /// @param ep the entry point
  /// @returns true if the entry point data was emitted
  bool EmitEntryPointData(ast::EntryPoint* ep);
  /// Handles emitting the entry point function
  /// @param ep the entry point
  /// @returns true if the entry point function was emitted
  bool EmitEntryPointFunction(ast::EntryPoint* ep);
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
  /// @param ep_name the current entry point or blank if none set
  /// @returns true if the function was emitted.
  bool EmitFunctionInternal(ast::Function* func,
                            bool emit_duplicate_functions,
                            const std::string& ep_name);
  /// Handles generating an identifier expression
  /// @param expr the identifier expression
  /// @returns true if the identifier was emitted
  bool EmitIdentifier(ast::IdentifierExpression* expr);
  /// Handles an if statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitIf(ast::IfStatement* stmt);
  /// Handles genreating an import expression
  /// @param expr the expression
  /// @returns true if the expression was successfully emitted.
  bool EmitImportFunction(ast::CallExpression* expr);
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
  /// @param skip_constructor set true if the constructor should be skipped
  /// @returns true if the variable was emitted
  bool EmitVariable(ast::Variable* var, bool skip_constructor);
  /// Handles generating a program scope constant variable
  /// @param var the variable to emit
  /// @returns true if the variable was emitted
  bool EmitProgramConstVariable(const ast::Variable* var);
  /// Emits the zero value for the given type
  /// @param type the type to emit the value for
  /// @returns true if the zero value was successfully emitted.
  bool EmitZeroValue(ast::type::Type* type);

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

  /// Generates a name for the prefix
  /// @param prefix the prefix of the name to generate
  /// @returns the name
  std::string generate_name(const std::string& prefix);
  /// Generates an intrinsic name from the given name
  /// @param name the name to convert to an intrinsic
  /// @returns the intrinsic name or blank on error
  std::string generate_intrinsic_name(const std::string& name);

  /// Checks if the global variable is in an input or output struct
  /// @param var the variable to check
  /// @returns true if the global is in an input or output struct
  bool global_is_in_struct(ast::Variable* var) const;

  /// Converts a builtin to an attribute name
  /// @param builtin the builtin to convert
  /// @returns the string name of the builtin or blank on error
  std::string builtin_to_attribute(ast::Builtin builtin) const;

  /// @returns the namer for testing
  Namer* namer_for_testing() { return &namer_; }

 private:
  enum class VarType { kIn, kOut };

  struct EntryPointData {
    std::string struct_name;
    std::string var_name;
  };

  std::string current_ep_var_name(VarType type);

  Namer namer_;
  ScopeStack<ast::Variable*> global_variables_;
  std::string current_ep_name_;
  bool generating_entry_point_ = false;
  const ast::Module* module_ = nullptr;
  uint32_t loop_emission_counter_ = 0;

  std::unordered_map<std::string, EntryPointData> ep_name_to_in_data_;
  std::unordered_map<std::string, EntryPointData> ep_name_to_out_data_;

  // This maps an input of "<entry_point_name>_<function_name>" to a remapped
  // function name. If there is no entry for a given key then function did
  // not need to be remapped for the entry point and can be emitted directly.
  std::unordered_map<std::string, std::string> ep_func_name_remapped_;
};

}  // namespace msl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_MSL_GENERATOR_IMPL_H_
