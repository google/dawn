// Copyright 2022 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/writer/ast_raise/var_for_dynamic_index.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/transform/hoist_to_decl_before.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"

TINT_INSTANTIATE_TYPEINFO(tint::spirv::writer::VarForDynamicIndex);

namespace tint::spirv::writer {

VarForDynamicIndex::VarForDynamicIndex() = default;

VarForDynamicIndex::~VarForDynamicIndex() = default;

ast::transform::Transform::ApplyResult VarForDynamicIndex::Apply(const Program& src,
                                                                 const ast::transform::DataMap&,
                                                                 ast::transform::DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    ast::transform::HoistToDeclBefore hoist_to_decl_before(ctx);

    // Extracts array and matrix values that are dynamically indexed to a
    // temporary `var` local that is then indexed.
    auto dynamic_index_to_var = [&](const ast::IndexAccessorExpression* access_expr) {
        auto* index_expr = access_expr->index;
        auto* object_expr = access_expr->object;
        auto& sem = src.Sem();

        auto stage = sem.GetVal(index_expr)->Stage();
        if (stage == core::EvaluationStage::kConstant ||
            stage == core::EvaluationStage::kNotEvaluated) {
            // Index expression resolves to a compile time value or will not be evaluated.
            // As this isn't a dynamic index, we can ignore this.
            return true;
        }

        auto* indexed = sem.GetVal(object_expr);
        if (!indexed->Type()->IsAnyOf<core::type::Array, core::type::Matrix>()) {
            // We only care about array and matrices.
            return true;
        }

        // TODO(bclayton): group multiple accesses in the same object.
        // e.g. arr[i] + arr[i+1] // Don't create two vars for this
        return hoist_to_decl_before.Add(indexed, object_expr,
                                        ast::transform::HoistToDeclBefore::VariableKind::kVar,
                                        "var_for_index");
    };

    bool index_accessor_found = false;
    for (auto* node : src.ASTNodes().Objects()) {
        if (auto* access_expr = node->As<ast::IndexAccessorExpression>()) {
            if (!dynamic_index_to_var(access_expr)) {
                return resolver::Resolve(b);
            }
            index_accessor_found = true;
        }
    }
    if (!index_accessor_found) {
        return SkipTransform;
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::spirv::writer
