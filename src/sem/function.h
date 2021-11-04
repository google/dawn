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
#include "src/sem/call.h"
#include "src/utils/unique_vector.h"

namespace tint {

// Forward declarations
namespace ast {
class BuiltinDecoration;
class Function;
class LocationDecoration;
class ReturnStatement;
}  // namespace ast

namespace sem {

class Intrinsic;
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

/// WorkgroupSize is a three-dimensional array of WorkgroupDimensions.
using WorkgroupSize = std::array<WorkgroupDimension, 3>;

/// Function holds the semantic information for function nodes.
class Function : public Castable<Function, CallTarget> {
 public:
  /// A vector of [Variable*, ast::VariableBindingPoint] pairs
  using VariableBindings =
      std::vector<std::pair<const Variable*, ast::VariableBindingPoint>>;

  /// Constructor
  /// @param declaration the ast::Function
  /// @param return_type the return type of the function
  /// @param parameters the parameters to the function
  /// @param transitively_referenced_globals the referenced module variables
  /// @param directly_referenced_globals the locally referenced module
  /// @param callsites the callsites of the function
  /// @param ancestor_entry_points the ancestor entry points
  /// @param workgroup_size the workgroup size
  Function(const ast::Function* declaration,
           Type* return_type,
           std::vector<Parameter*> parameters,
           std::vector<const GlobalVariable*> transitively_referenced_globals,
           std::vector<const GlobalVariable*> directly_referenced_globals,
           std::vector<const ast::CallExpression*> callsites,
           std::vector<Symbol> ancestor_entry_points,
           sem::WorkgroupSize workgroup_size);

  /// Destructor
  ~Function() override;

  /// @returns the ast::Function declaration
  const ast::Function* Declaration() const { return declaration_; }

  /// @returns the workgroup size {x, y, z} for the function.
  const sem::WorkgroupSize& WorkgroupSize() const { return workgroup_size_; }

  /// @returns all transitively referenced global variables
  const utils::UniqueVector<const GlobalVariable*>&
  TransitivelyReferencedGlobals() const {
    return transitively_referenced_globals_;
  }

  /// @returns the list of callsites of this function
  std::vector<const ast::CallExpression*> CallSites() const {
    return callsites_;
  }

  /// @returns the names of the ancestor entry points
  const std::vector<Symbol>& AncestorEntryPoints() const {
    return ancestor_entry_points_;
  }

  /// Retrieves any referenced location variables
  /// @returns the <variable, decoration> pair.
  std::vector<std::pair<const Variable*, const ast::LocationDecoration*>>
  TransitivelyReferencedLocationVariables() const;

  /// Retrieves any referenced builtin variables
  /// @returns the <variable, decoration> pair.
  std::vector<std::pair<const Variable*, const ast::BuiltinDecoration*>>
  TransitivelyReferencedBuiltinVariables() const;

  /// Retrieves any referenced uniform variables. Note, the variables must be
  /// decorated with both binding and group decorations.
  /// @returns the referenced uniforms
  VariableBindings TransitivelyReferencedUniformVariables() const;

  /// Retrieves any referenced storagebuffer variables. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  VariableBindings TransitivelyReferencedStorageBufferVariables() const;

  /// Retrieves any referenced regular Sampler variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  VariableBindings TransitivelyReferencedSamplerVariables() const;

  /// Retrieves any referenced comparison Sampler variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced storagebuffers
  VariableBindings TransitivelyReferencedComparisonSamplerVariables() const;

  /// Retrieves any referenced sampled textures variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced sampled textures
  VariableBindings TransitivelyReferencedSampledTextureVariables() const;

  /// Retrieves any referenced multisampled textures variables. Note, the
  /// variables must be decorated with both binding and group decorations.
  /// @returns the referenced sampled textures
  VariableBindings TransitivelyReferencedMultisampledTextureVariables() const;

  /// Retrieves any referenced variables of the given type. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @param type_info the type of the variables to find
  /// @returns the referenced variables
  VariableBindings TransitivelyReferencedVariablesOfType(
      const tint::TypeInfo& type_info) const;

  /// Retrieves any referenced variables of the given type. Note, the variables
  /// must be decorated with both binding and group decorations.
  /// @returns the referenced variables
  template <typename T>
  VariableBindings TransitivelyReferencedVariablesOfType() const {
    return TransitivelyReferencedVariablesOfType(TypeInfo::Of<T>());
  }

  /// Checks if the given entry point is an ancestor
  /// @param sym the entry point symbol
  /// @returns true if `sym` is an ancestor entry point of this function
  bool HasAncestorEntryPoint(Symbol sym) const;

 private:
  VariableBindings TransitivelyReferencedSamplerVariablesImpl(
      ast::SamplerKind kind) const;
  VariableBindings TransitivelyReferencedSampledTextureVariablesImpl(
      bool multisampled) const;

  const ast::Function* const declaration_;
  const sem::WorkgroupSize workgroup_size_;

  utils::UniqueVector<const GlobalVariable*> directly_referenced_globals_;
  utils::UniqueVector<const GlobalVariable*> transitively_referenced_globals_;
  utils::UniqueVector<const Function*> transitively_called_functions_;
  utils::UniqueVector<const Intrinsic*> directly_called_intrinsics_;
  std::vector<const ast::CallExpression*> callsites_;
  std::vector<Symbol> ancestor_entry_points_;
};

}  // namespace sem
}  // namespace tint

#endif  // SRC_SEM_FUNCTION_H_
