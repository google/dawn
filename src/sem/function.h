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

#ifndef SRC_SEM_FUNCTION_H_
#define SRC_SEM_FUNCTION_H_

#include <array>
#include <utility>
#include <vector>

#include "src/ast/variable.h"
#include "src/sem/call_target.h"

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

namespace sem {

class Variable;

/// WorkgroupDimension describes the size of a single dimension of an entry
/// point's workgroup size.
struct WorkgroupDimension {
  /// The size of this dimension.
  uint32_t value;
  /// A pipeline-overridable constant that overrides the size, or nullptr if
  /// this dimension is not overridable.
  const ast::Variable* overridable_const = nullptr;
};

/// Function holds the semantic information for function nodes.
class Function : public Castable<Function, CallTarget> {
 public:
  /// A vector of [Variable*, ast::Variable::BindingPoint] pairs
  using VariableBindings =
      std::vector<std::pair<const Variable*, ast::Variable::BindingPoint>>;

  /// Constructor
  /// @param declaration the ast::Function
  /// @param return_type the return type of the function
  /// @param parameters the parameters to the function
  /// @param referenced_module_vars the referenced module variables
  /// @param local_referenced_module_vars the locally referenced module
  /// @param return_statements the function return statements
  /// variables
  /// @param ancestor_entry_points the ancestor entry points
  /// @param workgroup_size the workgroup size
  Function(ast::Function* declaration,
           Type* return_type,
           std::vector<const Variable*> parameters,
           std::vector<const Variable*> referenced_module_vars,
           std::vector<const Variable*> local_referenced_module_vars,
           std::vector<const ast::ReturnStatement*> return_statements,
           std::vector<Symbol> ancestor_entry_points,
           std::array<WorkgroupDimension, 3> workgroup_size);

  /// Destructor
  ~Function() override;

  /// @returns the ast::Function declaration
  ast::Function* Declaration() const { return declaration_; }

  /// @return the parameters to the function
  const std::vector<const Variable*> Parameters() const { return parameters_; }

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
  /// @returns the referenced depth textures
  VariableBindings ReferencedDepthTextureVariables() const;

  /// Retrieves any referenced external texture variables. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @returns the referenced external textures
  VariableBindings ReferencedExternalTextureVariables() const;

  /// Checks if the given entry point is an ancestor
  /// @param sym the entry point symbol
  /// @returns true if `sym` is an ancestor entry point of this function
  bool HasAncestorEntryPoint(Symbol sym) const;

  /// @returns the workgroup size {x, y, z} for the function.
  const std::array<WorkgroupDimension, 3>& workgroup_size() const {
    return workgroup_size_;
  }

 private:
  VariableBindings ReferencedSamplerVariablesImpl(ast::SamplerKind kind) const;
  VariableBindings ReferencedSampledTextureVariablesImpl(
      bool multisampled) const;

  ast::Function* const declaration_;
  std::vector<const Variable*> const parameters_;
  std::vector<const Variable*> const referenced_module_vars_;
  std::vector<const Variable*> const local_referenced_module_vars_;
  std::vector<const ast::ReturnStatement*> const return_statements_;
  std::vector<Symbol> const ancestor_entry_points_;
  std::array<WorkgroupDimension, 3> workgroup_size_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_FUNCTION_H_
