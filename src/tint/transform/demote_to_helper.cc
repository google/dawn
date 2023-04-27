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

#include "src/tint/transform/demote_to_helper.h"

#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/call.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/statement.h"
#include "src/tint/switch.h"
#include "src/tint/transform/utils/hoist_to_decl_before.h"
#include "src/tint/type/reference.h"
#include "src/tint/utils/map.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::DemoteToHelper);

using namespace tint::number_suffixes;  // NOLINT

namespace tint::transform {

DemoteToHelper::DemoteToHelper() = default;

DemoteToHelper::~DemoteToHelper() = default;

Transform::ApplyResult DemoteToHelper::Apply(const Program* src, const DataMap&, DataMap&) const {
    auto& sem = src->Sem();

    // Collect the set of functions that need to be processed.
    // A function needs to be processed if it is reachable by a shader that contains a discard at
    // any point in its call hierarchy.
    std::unordered_set<const sem::Function*> functions_to_process;
    for (auto* func : src->AST().Functions()) {
        if (!func->IsEntryPoint()) {
            continue;
        }

        // Determine whether this entry point and its callees need to be transformed.
        bool needs_transform = false;
        if (sem.Get(func)->DiscardStatement()) {
            needs_transform = true;
        } else {
            for (auto* callee : sem.Get(func)->TransitivelyCalledFunctions()) {
                if (callee->DiscardStatement()) {
                    needs_transform = true;
                    break;
                }
            }
        }
        if (!needs_transform) {
            continue;
        }

        // Process the entry point and its callees.
        functions_to_process.insert(sem.Get(func));
        for (auto* callee : sem.Get(func)->TransitivelyCalledFunctions()) {
            functions_to_process.insert(callee);
        }
    }

    if (functions_to_process.empty()) {
        return SkipTransform;
    }

    ProgramBuilder b;
    CloneContext ctx{&b, src, /* auto_clone_symbols */ true};

    // Create a module-scope flag that indicates whether the current invocation has been discarded.
    auto flag = b.Symbols().New("tint_discarded");
    b.GlobalVar(flag, builtin::AddressSpace::kPrivate, b.Expr(false));

    // Replace all discard statements with a statement that marks the invocation as discarded.
    ctx.ReplaceAll([&](const ast::DiscardStatement*) -> const ast::Statement* {
        return b.Assign(flag, b.Expr(true));
    });

    // Insert a conditional discard at the end of each entry point that does not end with a return.
    for (auto* func : functions_to_process) {
        if (func->Declaration()->IsEntryPoint()) {
            auto* sem_body = sem.Get(func->Declaration()->body);
            if (sem_body->Behaviors().Contains(sem::Behavior::kNext)) {
                ctx.InsertBack(func->Declaration()->body->statements,
                               b.If(flag, b.Block(b.Discard())));
            }
        }
    }

    HoistToDeclBefore hoist_to_decl_before(ctx);

    // Mask all writes to host-visible memory using the discarded flag.
    // We also insert a discard statement before all return statements in entry points for shaders
    // that discard.
    std::unordered_map<const type::Type*, Symbol> atomic_cmpxchg_result_types;
    for (auto* node : src->ASTNodes().Objects()) {
        Switch(
            node,

            // Mask assignments to storage buffer variables.
            [&](const ast::AssignmentStatement* assign) {
                // Skip writes in functions that are not called from shaders that discard.
                auto* func = sem.Get(assign)->Function();
                if (functions_to_process.count(func) == 0) {
                    return;
                }

                // Skip phony assignments.
                if (assign->lhs->Is<ast::PhonyExpression>()) {
                    return;
                }

                // Skip writes to invocation-private address spaces.
                auto* ref = sem.GetVal(assign->lhs)->Type()->As<type::Reference>();
                switch (ref->AddressSpace()) {
                    case builtin::AddressSpace::kStorage:
                        // Need to mask these.
                        break;
                    case builtin::AddressSpace::kFunction:
                    case builtin::AddressSpace::kPrivate:
                    case builtin::AddressSpace::kOut:
                        // Skip these.
                        return;
                    default:
                        TINT_UNREACHABLE(Transform, b.Diagnostics())
                            << "write to unhandled address space: " << ref->AddressSpace();
                }

                // Mask the assignment using the invocation-discarded flag.
                ctx.Replace(assign, b.If(b.Not(flag), b.Block(ctx.Clone(assign))));
            },

            // Mask builtins that write to host-visible memory.
            [&](const ast::CallExpression* call) {
                auto* sem_call = sem.Get<sem::Call>(call);
                auto* stmt = sem_call ? sem_call->Stmt() : nullptr;
                auto* func = stmt ? stmt->Function() : nullptr;
                auto* builtin = sem_call ? sem_call->Target()->As<sem::Builtin>() : nullptr;
                if (functions_to_process.count(func) == 0 || !builtin) {
                    return;
                }

                if (builtin->Type() == builtin::Function::kTextureStore) {
                    // A call to textureStore() will always be a statement.
                    // Wrap it inside a conditional block.
                    auto* masked_call = b.If(b.Not(flag), b.Block(ctx.Clone(stmt->Declaration())));
                    ctx.Replace(stmt->Declaration(), masked_call);
                } else if (builtin->IsAtomic() &&
                           builtin->Type() != builtin::Function::kAtomicLoad) {
                    // A call to an atomic builtin can be a statement or an expression.
                    if (auto* call_stmt = stmt->Declaration()->As<ast::CallStatement>();
                        call_stmt && call_stmt->expr == call) {
                        // This call is a statement.
                        // Wrap it inside a conditional block.
                        auto* masked_call = b.If(b.Not(flag), b.Block(ctx.Clone(call_stmt)));
                        ctx.Replace(stmt->Declaration(), masked_call);
                    } else {
                        // This call is an expression.
                        // We transform:
                        //   let y = x + atomicAdd(&p, 1);
                        // Into:
                        //   var tmp : i32;
                        //   if (!tint_discarded) {
                        //     tmp = atomicAdd(&p, 1);
                        //   }
                        //   let y = x + tmp;
                        auto result = b.Sym();
                        ast::Type result_ty;
                        const ast::Statement* masked_call = nullptr;
                        if (builtin->Type() == builtin::Function::kAtomicCompareExchangeWeak) {
                            // Special case for atomicCompareExchangeWeak as we cannot name its
                            // result type. We have to declare an equivalent struct and copy the
                            // original member values over to it.

                            // Declare a struct to hold the result values.
                            auto* result_struct = sem_call->Type()->As<type::Struct>();
                            auto* atomic_ty = result_struct->Members()[0]->Type();
                            result_ty = b.ty(
                                utils::GetOrCreate(atomic_cmpxchg_result_types, atomic_ty, [&]() {
                                    auto name = b.Sym();
                                    b.Structure(
                                        name,
                                        utils::Vector{
                                            b.Member("old_value", CreateASTTypeFor(ctx, atomic_ty)),
                                            b.Member("exchanged", b.ty.bool_()),
                                        });
                                    return name;
                                }));

                            // Generate the masked call and member-wise copy:
                            //   if (!tint_discarded) {
                            //     let tmp_result = atomicCompareExchangeWeak(&p, 1, 2);
                            //     result.exchanged = tmp_result.exchanged;
                            //     result.old_value = tmp_result.old_value;
                            //   }
                            auto tmp_result = b.Sym();
                            masked_call =
                                b.If(b.Not(flag),
                                     b.Block(utils::Vector{
                                         b.Decl(b.Let(tmp_result, ctx.CloneWithoutTransform(call))),
                                         b.Assign(b.MemberAccessor(result, "old_value"),
                                                  b.MemberAccessor(tmp_result, "old_value")),
                                         b.Assign(b.MemberAccessor(result, "exchanged"),
                                                  b.MemberAccessor(tmp_result, "exchanged")),
                                     }));
                        } else {
                            result_ty = CreateASTTypeFor(ctx, sem_call->Type());
                            masked_call =
                                b.If(b.Not(flag),
                                     b.Block(b.Assign(result, ctx.CloneWithoutTransform(call))));
                        }
                        auto* result_decl = b.Decl(b.Var(result, result_ty));
                        hoist_to_decl_before.Prepare(sem_call);
                        hoist_to_decl_before.InsertBefore(stmt, result_decl);
                        hoist_to_decl_before.InsertBefore(stmt, masked_call);
                        ctx.Replace(call, b.Expr(result));
                    }
                }
            },

            // Insert a conditional discard before all return statements in entry points.
            [&](const ast::ReturnStatement* ret) {
                auto* func = sem.Get(ret)->Function();
                if (func->Declaration()->IsEntryPoint() && functions_to_process.count(func)) {
                    auto* discard = b.If(flag, b.Block(b.Discard()));
                    ctx.InsertBefore(sem.Get(ret)->Block()->Declaration()->statements, ret,
                                     discard);
                }
            });
    }

    ctx.Clone();
    return Program(std::move(b));
}

}  // namespace tint::transform
