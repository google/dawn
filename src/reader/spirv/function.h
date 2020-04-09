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

#ifndef SRC_READER_SPIRV_FUNCTION_H_
#define SRC_READER_SPIRV_FUNCTION_H_

#include <memory>
#include <vector>

#include "source/opt/basic_block.h"
#include "source/opt/constants.h"
#include "source/opt/function.h"
#include "source/opt/instruction.h"
#include "source/opt/ir_context.h"
#include "source/opt/type_manager.h"
#include "src/ast/expression.h"
#include "src/ast/module.h"
#include "src/reader/spirv/fail_stream.h"
#include "src/reader/spirv/namer.h"
#include "src/reader/spirv/parser_impl.h"

namespace tint {
namespace reader {
namespace spirv {

/// A FunctionEmitter emits a SPIR-V function onto a Tint AST module.
class FunctionEmitter {
 public:
  /// Creates a FunctionEmitter, and prepares to write to the AST module
  /// in |pi|.
  /// @param pi a ParserImpl which has already executed BuildInternalModule
  /// @param function the function to emit
  FunctionEmitter(ParserImpl* pi, const spvtools::opt::Function& function);
  /// Destructor
  ~FunctionEmitter();

  /// Emits the function to AST module.
  /// @return whether emission succeeded
  bool Emit();

  /// @returns true if emission has not yet failed.
  bool success() const { return fail_stream_.status(); }
  /// @returns true if emission has failed.
  bool failed() const { return !success(); }

  /// @returns the body of the function.
  const ast::StatementList& ast_body() { return ast_body_; }

  /// Records failure.
  /// @returns a FailStream on which to emit diagnostics.
  FailStream& Fail() { return fail_stream_.Fail(); }

  /// Emits the declaration, which comprises the name, parameters, and
  /// return type. The function AST node is appended to the module
  /// AST node.
  /// @returns true if emission has not yet failed.
  bool EmitFunctionDeclaration();

  /// Emits the function body, populating |ast_body_|
  /// @returns false if emission failed.
  bool EmitBody();

  /// Emits declarations of function variables.
  /// @returns false if emission failed.
  bool EmitFunctionVariables();

  /// Emits statements in the body.
  /// @returns false if emission failed.
  bool EmitFunctionBodyStatements();

  /// Emits a basic block
  /// @param bb internal representation of the basic block
  /// @returns false if emission failed.
  bool EmitStatementsInBasicBlock(const spvtools::opt::BasicBlock& bb);

  /// Emits a normal instruction: not a terminator, label, or variable
  /// declaration.
  /// @param inst the instruction
  /// @returns false if emission failed.
  bool EmitStatement(const spvtools::opt::Instruction& inst);

  /// Makes an expression
  /// @param id the SPIR-V ID of the value
  /// @returns true if emission has not yet failed.
  std::unique_ptr<ast::Expression> MakeExpression(uint32_t id);

 private:
  /// @returns the store type for the OpVariable instruction, or
  /// null on failure.
  ast::type::Type* GetVariableStoreType(
      const spvtools::opt::Instruction& var_decl_inst);

  ParserImpl& parser_impl_;
  ast::Module& ast_module_;
  spvtools::opt::IRContext& ir_context_;
  spvtools::opt::analysis::DefUseManager* def_use_mgr_;
  spvtools::opt::analysis::ConstantManager* constant_mgr_;
  spvtools::opt::analysis::TypeManager* type_mgr_;
  FailStream& fail_stream_;
  Namer& namer_;
  const spvtools::opt::Function& function_;
  ast::StatementList ast_body_;
  // The set of IDs that have already had an identifier name generated for it.
  std::unordered_set<uint32_t> identifier_values_;
};

}  // namespace spirv
}  // namespace reader
}  // namespace tint

#endif  // SRC_READER_SPIRV_FUNCTION_H_
