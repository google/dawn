// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0(the "License");
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

#ifndef SRC_SEMANTIC_FUNCTION_H_
#define SRC_SEMANTIC_FUNCTION_H_

#include <utility>
#include <vector>

#include "src/semantic/call_target.h"

namespace tint {

// Forward declarations
namespace ast {
class BindingDecoration;
class BuiltinDecoration;
class Function;
class GroupDecoration;
class LocationDecoration;
class ReturnStatement;
}  // namespace ast

namespace semantic {

class Variable;

/// Function holds the semantic information for function nodes.
class Function : public Castable<Function, CallTarget> {
 public:
  /// Information about a binding
  struct BindingInfo {
    /// The binding decoration
    ast::BindingDecoration* binding = nullptr;
    /// The group decoration
    ast::GroupDecoration* group = nullptr;
  };

  /// A vector of [Variable*, BindingInfo] pairs
  using VariableBindings = std::vector<std::pair<const Variable*, BindingInfo>>;

  /// Constructor
  /// @param declaration the ast::Function
  /// @param referenced_module_vars the referenced module variables
  /// @param local_referenced_module_vars the locally referenced module
  /// @param return_statements the function return statements
  /// variables
  /// @param ancestor_entry_points the ancestor entry points
  Function(ast::Function* declaration,
           std::vector<const Variable*> referenced_module_vars,
           std::vector<const Variable*> local_referenced_module_vars,
           std::vector<const ast::ReturnStatement*> return_statements,
           std::vector<Symbol> ancestor_entry_points);

  /// Destructor
  ~Function() override;

  /// @returns the ast::Function declaration
  ast::Function* Declaration() const { return declaration_; }

  /// Note: If this function calls other functions, the return will also include
  /// all of the referenced variables from the callees.
  /// @returns the referenced module variables
  const std::vector<const Variable*>& ReferencedModuleVariables() const {
    return referenced_module_vars_;
  }
  /// @returns the locally referenced module variables
  const std::vector<const Variable*>& LocalReferencedModuleVariables() const {
    return local_referenced_module_vars_;
  }
  /// @returns the return statements
  const std::vector<const ast::ReturnStatement*> ReturnStatements() const {
    return return_statements_;
  }
  /// @returns the ancestor entry points
  const std::vector<Symbol>& AncestorEntryPoints() const {
    return ancestor_entry_points_;
  }
  /// Retrieves any referenced location variables
  /// @returns the <variable, decoration> pair.
  std::vector<std::pair<const Variable*, ast::LocationDecoration*>>
  ReferencedLocationVariables() const;

  /// Retrieves any referenced builtin variables
  /// @returns the <variable, decoration> pair.
  std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
  ReferencedBuiltinVariables() const;

  /// Retrieves any referenced uniform variables. Note, the variables must be
  /// decorated with both binding and group decorations.
  /// @returns the referenced uniforms
  VariableBindings ReferencedUniformVariables() const;

  /// Retrieves any referenced storagebuffer variables. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  VariableBindings ReferencedStorageBufferVariables() const;

  /// Retrieves any referenced regular Sampler variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  VariableBindings ReferencedSamplerVariables() const;

  /// Retrieves any referenced comparison Sampler variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  VariableBindings ReferencedComparisonSamplerVariables() const;

  /// Retrieves any referenced sampled textures variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced sampled textures
  VariableBindings ReferencedSampledTextureVariables() const;

  /// Retrieves any referenced multisampled textures variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced sampled textures
  VariableBindings ReferencedMultisampledTextureVariables() const;

  /// Retrieves any referenced storage texture variables. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @returns the referenced storage textures
  VariableBindings ReferencedStorageTextureVariables() const;

  /// Retrieves any referenced depth texture variables. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @returns the referenced storage textures
  VariableBindings ReferencedDepthTextureVariables() const;

  /// Retrieves any locally referenced builtin variables
  /// @returns the <variable, decoration> pairs.
  std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
  LocalReferencedBuiltinVariables() const;

  /// Checks if the given entry point is an ancestor
  /// @param sym the entry point symbol
  /// @returns true if `sym` is an ancestor entry point of this function
  bool HasAncestorEntryPoint(Symbol sym) const;

 private:
  VariableBindings ReferencedSamplerVariablesImpl(type::SamplerKind kind) const;
  VariableBindings ReferencedSampledTextureVariablesImpl(
      bool multisampled) const;

  ast::Function* const declaration_;
  std::vector<const Variable*> const referenced_module_vars_;
  std::vector<const Variable*> const local_referenced_module_vars_;
  std::vector<const ast::ReturnStatement*> const return_statements_;
  std::vector<Symbol> const ancestor_entry_points_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_FUNCTION_H_
