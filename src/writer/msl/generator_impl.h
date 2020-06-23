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

#include "src/ast/module.h"
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

  /// Handles an assignment statement
  /// @param stmt the statement to emit
  /// @returns true if the statement was emitted successfully
  bool EmitAssign(ast::AssignmentStatement* stmt);
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
  /// Handles return statements
  /// @param stmt the statement to emit
  /// @returns true if the statement was successfully emitted
  bool EmitReturn(ast::ReturnStatement* stmt);
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
  /// Handles generating type
  /// @param type the type to generate
  /// @param name the name of the variable, only used for array emission
  /// @returns true if the type is emitted
  bool EmitType(ast::type::Type* type, const std::string& name);
};

}  // namespace msl
}  // namespace writer
}  // namespace tint

#endif  // SRC_WRITER_MSL_GENERATOR_IMPL_H_
