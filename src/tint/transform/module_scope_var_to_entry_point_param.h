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

#ifndef SRC_TINT_TRANSFORM_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_
#define SRC_TINT_TRANSFORM_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_

#include "src/tint/transform/transform.h"

namespace tint::transform {

/// Move module-scope variables into the entry point as parameters.
///
/// MSL does not allow module-scope variables to have any address space other
/// than `constant`. This transform moves all module-scope declarations into the
/// entry point function (either as parameters or function-scope variables) and
/// then passes them as pointer parameters to any function that references them.
///
/// Since WGSL does not allow entry point parameters or function-scope variables
/// to have these address spaces, we annotate the new variable declarations
/// with an attribute that bypasses that validation rule.
///
/// Before:
/// ```
/// struct S {
///   f : f32;
/// };
/// @binding(0) @group(0)
/// var<storage, read> s : S;
/// var<private> p : f32 = 2.0;
///
/// fn foo() {
///   p = p + f;
/// }
///
/// @compute @workgroup_size(1)
/// fn main() {
///   foo();
/// }
/// ```
///
/// After:
/// ```
/// fn foo(p : ptr<private, f32>, sptr : ptr<storage, S, read>) {
///   *p = *p + (*sptr).f;
/// }
///
/// @compute @workgroup_size(1)
/// fn main(sptr : ptr<storage, S, read>) {
///   var<private> p : f32 = 2.0;
///   foo(&p, sptr);
/// }
/// ```
class ModuleScopeVarToEntryPointParam final
    : public utils::Castable<ModuleScopeVarToEntryPointParam, Transform> {
  public:
    /// Constructor
    ModuleScopeVarToEntryPointParam();
    /// Destructor
    ~ModuleScopeVarToEntryPointParam() override;

    /// @copydoc Transform::Apply
    ApplyResult Apply(const Program* program,
                      const DataMap& inputs,
                      DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::transform

#endif  // SRC_TINT_TRANSFORM_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_
