// Copyright 2021 The Tint Authors.
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

#ifndef SRC_TRANSFORM_SPIRV_H_
#define SRC_TRANSFORM_SPIRV_H_

#include <vector>

#include "src/transform/transform.h"

namespace tint {

// Forward declarations
class CloneContext;

namespace transform {

/// Spirv is a transform used to sanitize a Program for use with the Spirv
/// writer. Passing a non-sanitized Program to the Spirv writer will result in
/// undefined behavior.
class Spirv : public Transform {
 public:
  /// Configuration options for the transform.
  struct Config : public Castable<Config, Data> {
    /// Constructor
    /// @param emit_vertex_point_size `true` to generate a PointSize builtin
    explicit Config(bool emit_vertex_point_size = false);

    /// Copy constructor.
    Config(const Config&);

    /// Destructor.
    ~Config() override;

    /// Assignment operator.
    /// @returns this Config
    Config& operator=(const Config&);

    /// Set to `true` to generate a PointSize builtin and have it set to 1.0
    /// from all vertex shaders in the module.
    bool emit_vertex_point_size;
  };

  /// Constructor
  Spirv();
  ~Spirv() override;

  /// Runs the transform on `program`, returning the transformation result.
  /// @param program the source program to transform
  /// @param data optional extra transform-specific input data
  /// @returns the transformation result
  Output Run(const Program* program, const DataMap& data = {}) override;

 private:
  /// Hoist entry point parameters, return values, and struct members out to
  /// global variables.
  void HandleEntryPointIOTypes(CloneContext& ctx) const;
  /// Change type of sample mask builtin variables to single element arrays.
  void HandleSampleMaskBuiltins(CloneContext& ctx) const;
  /// Add a PointSize builtin output to the module and set it to 1.0 from all
  /// vertex stage entry points.
  void EmitVertexPointSize(CloneContext& ctx) const;
  /// Add an empty shader entry point if none exist in the module.
  void AddEmptyEntryPoint(CloneContext& ctx) const;

  /// Recursively create module-scope input variables for `ty` and add
  /// function-scope variables for structs to `func`.
  ///
  /// For non-structures, create a module-scope input variable.
  /// For structures, recurse into members and then create a function-scope
  /// variable initialized using the variables created for its members.
  /// Return the symbol for the variable that was created.
  Symbol HoistToInputVariables(CloneContext& ctx,
                               const ast::Function* func,
                               sem::Type* ty,
                               ast::Type* declared_ty,
                               const ast::DecorationList& decorations) const;

  /// Recursively create module-scope output variables for `ty` and build a list
  /// of assignment instructions to write to them from `store_value`.
  ///
  /// For non-structures, create a module-scope output variable and generate the
  /// assignment instruction.
  /// For structures, recurse into members, tracking the chain of member
  /// accessors.
  /// Returns the list of variable assignments in `stores`.
  void HoistToOutputVariables(CloneContext& ctx,
                              const ast::Function* func,
                              sem::Type* ty,
                              ast::Type* declared_ty,
                              const ast::DecorationList& decorations,
                              std::vector<Symbol> member_accesses,
                              Symbol store_value,
                              ast::StatementList& stores) const;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_SPIRV_H_
