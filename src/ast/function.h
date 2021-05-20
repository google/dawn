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

#ifndef SRC_AST_FUNCTION_H_
#define SRC_AST_FUNCTION_H_

#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "src/ast/binding_decoration.h"
#include "src/ast/block_statement.h"
#include "src/ast/builtin_decoration.h"
#include "src/ast/decoration.h"
#include "src/ast/group_decoration.h"
#include "src/ast/location_decoration.h"
#include "src/ast/pipeline_stage.h"
#include "src/ast/variable.h"

namespace tint {
namespace ast {

/// A Function statement.
class Function : public Castable<Function, Node> {
 public:
  /// Create a function
  /// @param program_id the identifier of the program that owns this node
  /// @param source the variable source
  /// @param symbol the function symbol
  /// @param params the function parameters
  /// @param return_type the return type
  /// @param body the function body
  /// @param decorations the function decorations
  /// @param return_type_decorations the return type decorations
  Function(ProgramID program_id,
           const Source& source,
           Symbol symbol,
           VariableList params,
           ast::Type* return_type,
           BlockStatement* body,
           DecorationList decorations,
           DecorationList return_type_decorations);
  /// Move constructor
  Function(Function&&);

  ~Function() override;

  /// @returns the function symbol
  Symbol symbol() const { return symbol_; }
  /// @returns the function params
  const VariableList& params() const { return params_; }

  /// @returns the decorations attached to this function
  const DecorationList& decorations() const { return decorations_; }

  /// @returns the functions pipeline stage or None if not set
  PipelineStage pipeline_stage() const;

  /// @returns true if this function is an entry point
  bool IsEntryPoint() const { return pipeline_stage() != PipelineStage::kNone; }

  /// @returns the function return type.
  ast::Type* return_type() const { return return_type_; }

  /// @returns the decorations attached to the function return type.
  const DecorationList& return_type_decorations() const {
    return return_type_decorations_;
  }

  /// @returns a pointer to the last statement of the function or nullptr if
  // function is empty
  const Statement* get_last_statement() const;

  /// @returns the function body
  const BlockStatement* body() const { return body_; }
  /// @returns the function body
  BlockStatement* body() { return body_; }

  /// Clones this node and all transitive child nodes using the `CloneContext`
  /// `ctx`.
  /// @param ctx the clone context
  /// @return the newly cloned node
  Function* Clone(CloneContext* ctx) const override;

  /// Writes a representation of the node to the output stream
  /// @param sem the semantic info for the program
  /// @param out the stream to write to
  /// @param indent number of spaces to indent the node when writing
  void to_str(const sem::Info& sem,
              std::ostream& out,
              size_t indent) const override;

  /// @returns the type name for this function
  std::string type_name() const;

 private:
  Function(const Function&) = delete;

  Symbol const symbol_;
  VariableList const params_;
  ast::Type* const return_type_;
  BlockStatement* const body_;
  DecorationList const decorations_;
  DecorationList const return_type_decorations_;
};

/// A list of functions
class FunctionList : public std::vector<Function*> {
 public:
  /// Appends f to the end of the list
  /// @param f the function to append to this list
  void Add(Function* f) { this->emplace_back(f); }

  /// Returns the function with the given name
  /// @param sym the function symbol to search for
  /// @returns the associated function or nullptr if none exists
  Function* Find(Symbol sym) const;

  /// Returns the function with the given name
  /// @param sym the function symbol to search for
  /// @param stage the pipeline stage
  /// @returns the associated function or nullptr if none exists
  Function* Find(Symbol sym, PipelineStage stage) const;

  /// @param stage the pipeline stage
  /// @returns true if the Builder contains an entrypoint function with
  /// the given stage
  bool HasStage(PipelineStage stage) const;
};

}  // namespace ast
}  // namespace tint

#endif  // SRC_AST_FUNCTION_H_
