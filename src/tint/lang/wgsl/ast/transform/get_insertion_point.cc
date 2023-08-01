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

#include "src/tint/lang/wgsl/ast/transform/get_insertion_point.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/sem/for_loop_statement.h"
#include "src/tint/utils/diagnostic/diagnostic.h"
#include "src/tint/utils/ice/ice.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::ast::transform::utils {

InsertionPoint GetInsertionPoint(program::CloneContext& ctx, const Statement* stmt) {
    auto& sem = ctx.src->Sem();
    using RetType = std::pair<const sem::BlockStatement*, const Statement*>;

    if (auto* sem_stmt = sem.Get(stmt)) {
        auto* parent = sem_stmt->Parent();
        return Switch(
            parent,
            [&](const sem::BlockStatement* block) -> RetType {
                // Common case, can insert in the current block above/below the input
                // statement.
                return {block, stmt};
            },
            [&](const sem::ForLoopStatement* fl) -> RetType {
                // `stmt` is either the for loop initializer or the continuing
                // statement of a for-loop.
                if (fl->Declaration()->initializer == stmt) {
                    // For loop init, can insert above the for loop itself.
                    return {fl->Block(), fl->Declaration()};
                }

                // Cannot insert before or after continuing statement of a for-loop
                return {};
            },
            [&](Default) -> RetType {
                TINT_ICE() << "expected parent of statement to be "
                              "either a block or for loop";
                return {};
            });
    }

    return {};
}

}  // namespace tint::ast::transform::utils
