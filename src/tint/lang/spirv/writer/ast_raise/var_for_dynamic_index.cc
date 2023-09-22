// Copyright 2022 The Tint Authors.
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
