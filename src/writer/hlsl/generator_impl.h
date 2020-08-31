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

#ifndef SRC_WRITER_HLSL_GENERATOR_IMPL_H_
#define SRC_WRITER_HLSL_GENERATOR_IMPL_H_

#include "src/ast/literal.h"
#include "src/ast/module.h"
#include "src/ast/scalar_constructor_expression.h"
#include "src/ast/type_constructor_expression.h"
#include "src/scope_stack.h"
#include "src/writer/hlsl/namer.h"

namespace tint {
namespace writer {
namespace hlsl {

/// Implementation class for HLSL generator
class GeneratorImpl {
 public:
  /// Constructor
  /// @param module the module to generate
  explicit GeneratorImpl(ast::Module* module);
  ~GeneratorImpl();

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
  /// @param out the output stream
  void make_indent(std::ostream& out);

  /// @returns the error
  std::string error() const { return error_; }

  /// @param out the output stream
  /// @returns true on successful generation; false otherwise
  bool Generate(std::ostream& out);

  /// Handles generating an alias
  /// @param out the output stream
  /// @param alias the alias to generate
  /// @returns true if the alias was emitted
  bool EmitAliasType(std::ostream& out, const ast::type::AliasType* alias);
  /// Handles an array accessor expression
  /// @param out the output stream
  /// @param expr the expression to emit
  /// @returns true if the array accessor was emitted
  bool EmitArrayAccessor(std::ostream& out, ast::ArrayAccessorExpression* expr);
  /// Handles generating an as expression
  /// @param out the output stream
  /// @param expr the as expression
  /// @returns true if the as was emitted
  bool EmitAs(std::ostream& out, ast::AsExpression* expr);
  /// Handles an assignment statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitAssign(std::ostream& out, ast::AssignmentStatement* stmt);
  /// Handles generating a binary expression
  /// @param out the output stream
  /// @param expr the binary expression
  /// @returns true if the expression was emitted, false otherwise
  bool EmitBinary(std::ostream& out, ast::BinaryExpression* expr);
  /// Handles a block statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBlock(std::ostream& out, const ast::BlockStatement* stmt);
  /// Handles a block statement with a newline at the end
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitIndentedBlockAndNewline(std::ostream& out,
                                   ast::BlockStatement* stmt);
  /// Handles a block statement with a newline at the end
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBlockAndNewline(std::ostream& out, const ast::BlockStatement* stmt);
  /// Handles a break statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBreak(std::ostream& out, ast::BreakStatement* stmt);
  /// Handles generating a call expression
  /// @param out the output stream
  /// @param expr the call expression
  /// @returns true if the call expression is emitted
  bool EmitCall(std::ostream& out, ast::CallExpression* expr);
  /// Handles a case statement
  /// @param out the output stream
  /// @param stmt the statement
  /// @returns true if the statment was emitted successfully
  bool EmitCase(std::ostream& out, ast::CaseStatement* stmt);
  /// Handles generating a cast expression
  /// @param out the output stream
  /// @param expr the cast expression
  /// @returns true if the cast was emitted
  bool EmitCast(std::ostream& out, ast::CastExpression* expr);
  /// Handles generating constructor expressions
  /// @param out the output stream
  /// @param expr the constructor expression
  /// @returns true if the expression was emitted
  bool EmitConstructor(std::ostream& out, ast::ConstructorExpression* expr);
  /// Handles generating a discard statement
  /// @param out the output stream
  /// @param stmt the discard statement
  /// @returns true if the statement was successfully emitted
  bool EmitDiscard(std::ostream& out, ast::DiscardStatement* stmt);
  /// Handles generating a scalar constructor
  /// @param out the output stream
  /// @param expr the scalar constructor expression
  /// @returns true if the scalar constructor is emitted
  bool EmitScalarConstructor(std::ostream& out,
                             ast::ScalarConstructorExpression* expr);
  /// Handles emitting a type constructor
  /// @param out the output stream
  /// @param expr the type constructor expression
  /// @returns true if the constructor is emitted
  bool EmitTypeConstructor(std::ostream& out,
                           ast::TypeConstructorExpression* expr);
  /// Handles a continue statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitContinue(std::ostream& out, ast::ContinueStatement* stmt);
  /// Handles generating an else statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitElse(std::ostream& out, ast::ElseStatement* stmt);
  /// Handles generate an Expression
  /// @param out the output stream
  /// @param expr the expression
  /// @returns true if the expression was emitted
  bool EmitExpression(std::ostream& out, ast::Expression* expr);
  /// Handles generating a function
  /// @param out the output stream
  /// @param func the function to generate
  /// @returns true if the function was emitted
  bool EmitFunction(std::ostream& out, ast::Function* func);
  /// Internal helper for emitting functions
  /// @param out the output stream
  /// @param func the function to emit
  /// @param emit_duplicate_functions set true if we need to duplicate per entry
  /// point
  /// @param ep_name the current entry point or blank if none set
  /// @returns true if the function was emitted.
  bool EmitFunctionInternal(std::ostream& out,
                            ast::Function* func,
                            bool emit_duplicate_functions,
                            const std::string& ep_name);
  /// Handles emitting information for an entry point
  /// @param out the output stream
  /// @param ep the entry point
  /// @returns true if the entry point data was emitted
  bool EmitEntryPointData(std::ostream& out, ast::EntryPoint* ep);
  /// Handles emitting the entry point function
  /// @param out the output stream
  /// @param ep the entry point
  /// @returns true if the entry point function was emitted
  bool EmitEntryPointFunction(std::ostream& out, ast::EntryPoint* ep);
  /// Handles an if statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitIf(std::ostream& out, ast::IfStatement* stmt);
  /// Handles genreating an import expression
  /// @param out the output stream
  /// @param expr the expression
  /// @returns true if the expression was successfully emitted.
  bool EmitImportFunction(std::ostream& out, ast::CallExpression* expr);
  /// Handles a literal
  /// @param out the output stream
  /// @param lit the literal to emit
  /// @returns true if the literal was successfully emitted
  bool EmitLiteral(std::ostream& out, ast::Literal* lit);
  /// Handles a loop statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitLoop(std::ostream& out, ast::LoopStatement* stmt);
  /// Handles generating an identifier expression
  /// @param out the output stream
  /// @param expr the identifier expression
  /// @returns true if the identifeir was emitted
  bool EmitIdentifier(std::ostream& out, ast::IdentifierExpression* expr);
  /// Handles a member accessor expression
  /// @param out the output stream
  /// @param expr the member accessor expression
  /// @returns true if the member accessor was emitted
  bool EmitMemberAccessor(std::ostream& out,
                          ast::MemberAccessorExpression* expr);
  /// Handles a storage buffer accessor expression
  /// @param out the output stream
  /// @param expr the storage buffer accessor expression
  /// @param rhs the right side of a store expression. Set to nullptr for a load
  /// @returns true if the storage buffer accessor was emitted
  bool EmitStorageBufferAccessor(std::ostream& out,
                                 ast::Expression* expr,
                                 ast::Expression* rhs);
  /// Handles return statements
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitReturn(std::ostream& out, ast::ReturnStatement* stmt);
  /// Handles statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitStatement(std::ostream& out, ast::Statement* stmt);
  /// Handles generating a switch statement
  /// @param out the output stream
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitSwitch(std::ostream& out, ast::SwitchStatement* stmt);
  /// Handles generating type
  /// @param out the output stream
  /// @param type the type to generate
  /// @param name the name of the variable, only used for array emission
  /// @returns true if the type is emitted
  bool EmitType(std::ostream& out,
                ast::type::Type* type,
                const std::string& name);
  /// Handles a unary op expression
  /// @param out the output stream
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryOp(std::ostream& out, ast::UnaryOpExpression* expr);
  /// Emits the zero value for the given type
  /// @param out the output stream
  /// @param type the type to emit the value for
  /// @returns true if the zero value was successfully emitted.
  bool EmitZeroValue(std::ostream& out, ast::type::Type* type);
  /// Handles generating a variable
  /// @param out the output stream
  /// @param var the variable to generate
  /// @param skip_constructor set true if the constructor should be skipped
  /// @returns true if the variable was emitted
  bool EmitVariable(std::ostream& out,
                    ast::Variable* var,
                    bool skip_constructor);
  /// Handles generating a program scope constant variable
  /// @param out the output stream
  /// @param var the variable to emit
  /// @returns true if the variable was emitted
  bool EmitProgramConstVariable(std::ostream& out, const ast::Variable* var);

  /// Returns true if the accessor is accessing a storage buffer.
  /// @param expr the expression to check
  /// @returns true if the accessor is accessing a storage buffer for which
  /// we need to execute a Load instruction.
  bool is_storage_buffer_access(ast::MemberAccessorExpression* expr);
  /// Returns true if the accessor is accessing a storage buffer.
  /// @param expr the expression to check
  /// @returns true if the accessor is accessing a storage buffer
  bool is_storage_buffer_access(ast::ArrayAccessorExpression* expr);
  /// Registers the given global with the generator
  /// @param global the global to register
  void register_global(ast::Variable* global);
  /// Checks if the global variable is in an input or output struct
  /// @param var the variable to check
  /// @returns true if the global is in an input or output struct
  bool global_is_in_struct(ast::Variable* var) const;
  /// Creates a text string representing the index into a storage buffer
  /// @param expr the expression to use as the index
  /// @returns the index string, or blank if unable to generate
  std::string generate_storage_buffer_index_expression(ast::Expression* expr);
  /// Generates a name for the prefix
  /// @param prefix the prefix of the name to generate
  /// @returns the name
  std::string generate_name(const std::string& prefix);
  /// Generates an intrinsic name from the given name
  /// @param name the name to convert to an intrinsic
  /// @returns the intrinsic name or blank on error
  std::string generate_intrinsic_name(const std::string& name);
  /// Converts a builtin to an attribute name
  /// @param builtin the builtin to convert
  /// @returns the string name of the builtin or blank on error
  std::string builtin_to_attribute(ast::Builtin builtin) const;
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

  /// @returns the namer for testing
  Namer* namer_for_testing() { return &namer_; }

 private:
  enum class VarType { kIn, kOut };

  struct EntryPointData {
    std::string struct_name;
    std::string var_name;
  };

  std::string current_ep_var_name(VarType type);

  std::string error_;
  size_t indent_ = 0;

  Namer namer_;
  ast::Module* module_ = nullptr;
  std::string current_ep_name_;
  bool generating_entry_point_ = false;
  uint32_t loop_emission_counter_ = 0;
  ScopeStack<ast::Variable*> global_variables_;
  std::unordered_map<std::string, EntryPointData> ep_name_to_in_data_;
  std::unordered_map<std::string, EntryPointData> ep_name_to_out_data_;

  // This maps an input of "<entry_point_name>_<function_name>" to a remapped
  // function name. If there is no entry for a given key then function did
  // not need to be remapped for the entry point and can be emitted directly.
  std::unordered_map<std::string, std::string> ep_func_name_remapped_;
};

}  // namespace hlsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_HLSL_GENERATOR_IMPL_H_
