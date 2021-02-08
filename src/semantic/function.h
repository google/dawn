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
#include "src/type/sampler_type.h"

namespace tint {

// Forward declarations
namespace ast {
class BindingDecoration;
class BuiltinDecoration;
class Function;
class GroupDecoration;
class LocationDecoration;
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

  /// Constructor
  /// @param ast the ast::Function
  /// @param referenced_module_vars the referenced module variables
  /// @param local_referenced_module_vars the locally referenced module
  /// variables
  /// @param ancestor_entry_points the ancestor entry points
  Function(ast::Function* ast,
           std::vector<const Variable*> referenced_module_vars,
           std::vector<const Variable*> local_referenced_module_vars,
           std::vector<Symbol> ancestor_entry_points);

  /// Destructor
  ~Function() override;

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
  /// @returns the ancestor entry points
  const std::vector<Symbol>& AncestorEntryPoints() const {
    return ancestor_entry_points_;
  }
  /// Retrieves any referenced location variables
  /// @returns the <variable, decoration> pair.
  const std::vector<std::pair<const Variable*, ast::LocationDecoration*>>
  ReferencedLocationVariables() const;

  /// Retrieves any referenced builtin variables
  /// @returns the <variable, decoration> pair.
  const std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
  ReferencedBuiltinVariables() const;

  /// Retrieves any referenced uniform variables. Note, the variables must be
  /// decorated with both binding and group decorations.
  /// @returns the referenced uniforms
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedUniformVariables() const;

  /// Retrieves any referenced storagebuffer variables. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedStoragebufferVariables() const;

  /// Retrieves any referenced regular Sampler variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedSamplerVariables() const;

  /// Retrieves any referenced comparison Sampler variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedComparisonSamplerVariables() const;

  /// Retrieves any referenced sampled textures variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced sampled textures
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedSampledTextureVariables() const;

  /// Retrieves any referenced multisampled textures variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced sampled textures
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedMultisampledTextureVariables() const;

  /// Retrieves any locally referenced builtin variables
  /// @returns the <variable, decoration> pairs.
  const std::vector<std::pair<const Variable*, ast::BuiltinDecoration*>>
  LocalReferencedBuiltinVariables() const;

  /// Checks if the given entry point is an ancestor
  /// @param sym the entry point symbol
  /// @returns true if `sym` is an ancestor entry point of this function
  bool HasAncestorEntryPoint(Symbol sym) const;

 private:
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedSamplerVariablesImpl(type::SamplerKind kind) const;
  const std::vector<std::pair<const Variable*, BindingInfo>>
  ReferencedSampledTextureVariablesImpl(bool multisampled) const;

  std::vector<const Variable*> const referenced_module_vars_;
  std::vector<const Variable*> const local_referenced_module_vars_;
  std::vector<Symbol> const ancestor_entry_points_;
};

}  // namespace semantic
}  // namespace tint

#endif  // SRC_SEMANTIC_FUNCTION_H_
