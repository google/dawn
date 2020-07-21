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

#include "src/ast/module.h"
#include "src/scope_stack.h"
#include "src/writer/hlsl/namer.h"
#include "src/writer/text_generator.h"

namespace tint {
namespace writer {
namespace hlsl {

/// Implementation class for HLSL generator
class GeneratorImpl : public TextGenerator {
 public:
  /// Constructor
  /// @param module the module to generate
  explicit GeneratorImpl(ast::Module* module);
  ~GeneratorImpl();

  /// @returns true on successful generation; false otherwise
  bool Generate();

  /// Handles generating a binary expression
  /// @param expr the binary expression
  /// @returns true if the expression was emitted, false otherwise
  bool EmitBinary(ast::BinaryExpression* expr);
  /// Handles a break statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitBreak(ast::BreakStatement* stmt);
  /// Handles a continue statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitContinue(ast::ContinueStatement* stmt);
  /// Handles generate an Expression
  /// @param expr the expression
  /// @returns true if the expression was emitted
  bool EmitExpression(ast::Expression* expr);
  /// Handles generating an identifier expression
  /// @param expr the identifier expression
  /// @returns true if the identifeir was emitted
  bool EmitIdentifier(ast::IdentifierExpression* expr);
  /// Handles return statements
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitReturn(ast::ReturnStatement* stmt);
  /// Handles statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted
  bool EmitStatement(ast::Statement* stmt);
  /// Handles a unary op expression
  /// @param expr the expression to emit
  /// @returns true if the expression was emitted
  bool EmitUnaryOp(ast::UnaryOpExpression* expr);

  /// Checks if the global variable is in an input or output struct
  /// @param var the variable to check
  /// @returns true if the global is in an input or output struct
  bool global_is_in_struct(ast::Variable* var) const;

 private:
  enum class VarType { kIn, kOut };

  struct EntryPointData {
    std::string struct_name;
    std::string var_name;
  };

  std::string current_ep_var_name(VarType type);

  Namer namer_;
  ast::Module* module_ = nullptr;
  std::string current_ep_name_;
  bool generating_entry_point_ = false;
  ScopeStack<ast::Variable*> global_variables_;
  std::unordered_map<std::string, EntryPointData> ep_name_to_in_data_;
  std::unordered_map<std::string, EntryPointData> ep_name_to_out_data_;
};

}  // namespace hlsl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_HLSL_GENERATOR_IMPL_H_
