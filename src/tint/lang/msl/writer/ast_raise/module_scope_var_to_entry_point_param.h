// Copyright 2021 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef SRC_TINT_LANG_MSL_WRITER_AST_RAISE_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_
#define SRC_TINT_LANG_MSL_WRITER_AST_RAISE_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_

#include "src/tint/lang/wgsl/ast/transform/transform.h"

namespace tint::msl::writer {

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
    : public Castable<ModuleScopeVarToEntryPointParam, ast::transform::Transform> {
  public:
    /// Constructor
    ModuleScopeVarToEntryPointParam();
    /// Destructor
    ~ModuleScopeVarToEntryPointParam() override;

    /// @copydoc ast::transform::Transform::Apply
    ApplyResult Apply(const Program& program,
                      const ast::transform::DataMap& inputs,
                      ast::transform::DataMap& outputs) const override;

  private:
    struct State;
};

}  // namespace tint::msl::writer

#endif  // SRC_TINT_LANG_MSL_WRITER_AST_RAISE_MODULE_SCOPE_VAR_TO_ENTRY_POINT_PARAM_H_
