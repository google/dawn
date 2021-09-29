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

#ifndef SRC_TRANSFORM_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_
#define SRC_TRANSFORM_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_

#include "src/transform/transform.h"

namespace tint {
namespace transform {

/// Move module-scope variables into the entry point as parameters.
///
/// MSL does not allow private and workgroup variables at module-scope, so we
/// push these declarations into the entry point function and then pass them as
/// pointer parameters to any function that references them.
/// Similarly, texture and sampler types are converted to entry point
/// parameters and passed by value to functions that need them.
///
/// Since WGSL does not allow function-scope variables to have these storage
/// classes, we annotate the new variable declarations with an attribute that
/// bypasses that validation rule.
///
/// Before:
/// ```
/// var<private> v : f32 = 2.0;
///
/// fn foo() {
///   v = v + 1.0;
/// }
///
/// [[stage(compute), workgroup_size(1)]]
/// fn main() {
///   foo();
/// }
/// ```
///
/// After:
/// ```
/// fn foo(v : ptr<private, f32>) {
///   *v = *v + 1.0;
/// }
///
/// [[stage(compute), workgroup_size(1)]]
/// fn main() {
///   var<private> v : f32 = 2.0;
///   foo(&v);
/// }
/// ```
class ModuleScopeVarToEntryPointParam
    : public Castable<ModuleScopeVarToEntryPointParam, Transform> {
 public:
  /// Constructor
  ModuleScopeVarToEntryPointParam();
  /// Destructor
  ~ModuleScopeVarToEntryPointParam() override;

 protected:
  /// Runs the transform using the CloneContext built for transforming a
  /// program. Run() is responsible for calling Clone() on the CloneContext.
  /// @param ctx the CloneContext primed with the input program and
  /// ProgramBuilder
  /// @param inputs optional extra transform-specific input data
  /// @param outputs optional extra transform-specific output data
  void Run(CloneContext& ctx, const DataMap& inputs, DataMap& outputs) override;

  struct State;
};

}  // namespace transform
}  // namespace tint

#endif  // SRC_TRANSFORM_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_
