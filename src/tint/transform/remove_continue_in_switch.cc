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

#include "src/tint/transform/remove_continue_in_switch.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/ast/continue_statement.h"
#include "src/tint/ast/switch_statement.h"
#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/for_loop_statement.h"
#include "src/tint/sem/loop_statement.h"
#include "src/tint/sem/switch_statement.h"
#include "src/tint/sem/while_statement.h"
#include "src/tint/transform/utils/get_insertion_point.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::RemoveContinueInSwitch);

namespace tint::transform {

/// PIMPL state for the transform
struct RemoveContinueInSwitch::State {
    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        bool made_changes = false;

        for (auto* node : src->ASTNodes().Objects()) {
            auto* cont = node->As<ast::ContinueStatement>();
            if (!cont) {
                continue;
            }

            // If first parent is not a switch within a loop, skip
            auto* switch_stmt = GetParentSwitchInLoop(sem, cont);
            if (!switch_stmt) {
                continue;
            }

            made_changes = true;

            auto cont_var_name =
                tint::utils::GetOrCreate(switch_to_cont_var_name, switch_stmt, [&]() {
                    // Create and insert 'var tint_continue : bool = false;' before the
                    // switch.
                    auto var_name = b.Symbols().New("tint_continue");
                    auto* decl = b.Decl(b.Var(var_name, b.ty.bool_(), b.Expr(false)));
                    auto ip = utils::GetInsertionPoint(ctx, switch_stmt);
                    ctx.InsertBefore(ip.first->Declaration()->statements, ip.second, decl);

                    // Create and insert 'if (tint_continue) { continue; }' after
                    // switch.
                    auto* if_stmt = b.If(b.Expr(var_name), b.Block(b.Continue()));
                    ctx.InsertAfter(ip.first->Declaration()->statements, ip.second, if_stmt);

                    // Return the new var name
                    return var_name;
                });

            // Replace 'continue;' with '{ tint_continue = true; break; }'
            auto* new_stmt = b.Block(                   //
                b.Assign(b.Expr(cont_var_name), true),  //
                b.Break());

            ctx.Replace(cont, new_stmt);
        }

        if (!made_changes) {
            return SkipTransform;
        }

        ctx.Clone();
        return Program(std::move(b));
    }

  private:
    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};
    /// Alias to src->sem
    const sem::Info& sem = src->Sem();

    // Map of switch statement to 'tint_continue' variable.
    std::unordered_map<const ast::SwitchStatement*, Symbol> switch_to_cont_var_name;

    // If `cont` is within a switch statement within a loop, returns a pointer to
    // that switch statement.
    static const ast::SwitchStatement* GetParentSwitchInLoop(const sem::Info& sem,
                                                             const ast::ContinueStatement* cont) {
        // Find whether first parent is a switch or a loop
        auto* sem_stmt = sem.Get(cont);
        auto* sem_parent = sem_stmt->FindFirstParent<sem::SwitchStatement, sem::LoopBlockStatement,
                                                     sem::ForLoopStatement, sem::WhileStatement>();
        if (!sem_parent) {
            return nullptr;
        }
        return sem_parent->Declaration()->As<ast::SwitchStatement>();
    }
};

RemoveContinueInSwitch::RemoveContinueInSwitch() = default;
RemoveContinueInSwitch::~RemoveContinueInSwitch() = default;

Transform::ApplyResult RemoveContinueInSwitch::Apply(const Program* src,
                                                     const DataMap&,
                                                     DataMap&) const {
    State state(src);
    return state.Run();
}

}  // namespace tint::transform
