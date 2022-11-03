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

#include "src/tint/transform/remove_unreachable_statements.h"

#include <memory>
#include <unordered_map>
#include <utility>
#include <vector>

#include "src/tint/ast/traverse_expressions.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/sem/variable.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/scoped_assignment.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::RemoveUnreachableStatements);

namespace tint::transform {

RemoveUnreachableStatements::RemoveUnreachableStatements() = default;

RemoveUnreachableStatements::~RemoveUnreachableStatements() = default;

Transform::ApplyResult RemoveUnreachableStatements::Apply(const Program* src,
                                                          const DataMap&,
                                                          DataMap&) const {
    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    bool made_changes = false;
    for (auto* node : src->ASTNodes().Objects()) {
        if (auto* stmt = src->Sem().Get<sem::Statement>(node)) {
            if (!stmt->IsReachable()) {
                RemoveStatement(ctx, stmt->Declaration());
                made_changes = true;
            }
        }
    }

    if (!made_changes) {
        return SkipTransform;
    }

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
