// Copyright 2023 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/ast_raise/subgroup_ballot.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::msl::writer::SubgroupBallot);
TINT_INSTANTIATE_TYPEINFO(tint::msl::writer::SubgroupBallot::SimdBallot);

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {

/// PIMPL state for the transform
struct SubgroupBallot::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};

    /// The name of the `tint_subgroup_ballot` helper function.
    Symbol ballot_helper{};

    /// The name of the `tint_subgroup_size_mask` global variable.
    Symbol subgroup_size_mask{};

    /// The set of a functions that directly call `subgroupBallot()`.
    Hashset<const sem::Function*, 4> ballot_callers;

    /// Constructor
    /// @param program the source program
    explicit State(const Program& program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        auto& sem = src.Sem();

        bool made_changes = false;
        for (auto* node : ctx.src->ASTNodes().Objects()) {
            auto* call = sem.Get<sem::Call>(node);
            if (call) {
                // If this is a call to a `subgroupBallot()` builtin, replace it with a call to the
                // helper function and make a note of the function that we are in.
                auto* builtin = call->Target()->As<sem::BuiltinFn>();
                if (builtin && builtin->Fn() == wgsl::BuiltinFn::kSubgroupBallot) {
                    auto* pred = ctx.Clone(call->Arguments()[0]->Declaration());
                    ctx.Replace(call->Declaration(), b.Call(GetHelper(), pred));
                    ballot_callers.Add(call->Stmt()->Function());
                    made_changes = true;
                }
            }
        }
        if (!made_changes) {
            return SkipTransform;
        }

        // Set the subgroup size mask at the start of each entry point that transitively calls
        // `subgroupBallot()`.
        for (auto* global : src.AST().GlobalDeclarations()) {
            auto* func = global->As<ast::Function>();
            if (func && func->IsEntryPoint() && TransitvelyCallsSubgroupBallot(sem.Get(func))) {
                SetSubgroupSizeMask(func);
            }
        }

        ctx.Clone();
        return resolver::Resolve(b);
    }

    /// Get (or create) the `tint_msl_subgroup_ballot` helper function.
    /// @returns the name of the helper function
    Symbol GetHelper() {
        if (!ballot_helper) {
            auto intrinsic = b.Symbols().New("tint_msl_simd_ballot");
            subgroup_size_mask = b.Symbols().New("tint_subgroup_size_mask");
            ballot_helper = b.Symbols().New("tint_msl_subgroup_ballot");

            // Declare the `tint_msl_subgroup_ballot` intrinsic function, which will use the
            // `simd_ballot` function to return 64-bit vote.
            {
                auto* pred = b.Param("pred", b.ty.bool_());
                b.Func(intrinsic, Vector{pred}, b.ty.vec2<u32>(), nullptr,
                       Vector{b.ASTNodes().Create<SimdBallot>(b.ID(), b.AllocateNodeID()),
                              b.Disable(ast::DisabledValidation::kFunctionHasNoBody)});
            }

            // Declare the `tint_subgroup_size_mask` variable.
            b.GlobalVar(subgroup_size_mask, core::AddressSpace::kPrivate, b.ty.vec4<u32>());

            // Declare the `tint_msl_subgroup_ballot` helper function as follows:
            //   fn tint_msl_subgroup_ballot(pred : bool) -> vec4u {
            //     let vote : vec2u = vec4f(tint_simd_ballot(pred), 0, 0);
            //     return (vote & tint_subgroup_size_mask);
            //   }
            auto* pred = b.Param("pred", b.ty.bool_());
            auto* vote =
                b.Let(b.Sym(), b.Call(b.ty.vec4<u32>(), b.Call(intrinsic, pred), 0_u, 0_u));
            b.Func(ballot_helper, Vector{pred}, b.ty.vec4<u32>(),
                   Vector{
                       b.Decl(vote),
                       b.Return(b.And(vote, subgroup_size_mask)),
                   });
        }
        return ballot_helper;
    }

    /// Check if a function directly or transitively calls the `subgroupBallot()` builtin.
    /// @param func the function to check
    /// @returns true if the function transitively calls `subgroupBallot()`
    bool TransitvelyCallsSubgroupBallot(const sem::Function* func) {
        if (ballot_callers.Contains(func)) {
            return true;
        }
        for (auto* called : func->TransitivelyCalledFunctions()) {
            if (ballot_callers.Contains(called)) {
                return true;
            }
        }
        return false;
    }

    /// Add code to set the `subgroup_size_mask` variable at the start of an entry point.
    /// @param ep the entry point
    void SetSubgroupSizeMask(const ast::Function* ep) {
        // Check the entry point parameters for an existing `subgroup_size` builtin.
        Symbol subgroup_size;
        for (auto* param : ep->params) {
            auto* builtin = ast::GetAttribute<ast::BuiltinAttribute>(param->attributes);
            if (builtin && builtin->builtin == core::BuiltinValue::kSubgroupSize) {
                subgroup_size = ctx.Clone(param->name->symbol);
            }
        }
        if (!subgroup_size.IsValid()) {
            // No `subgroup_size` builtin parameter was found, so add one.
            subgroup_size = b.Symbols().New("tint_subgroup_size");
            ctx.InsertBack(ep->params, b.Param(subgroup_size, b.ty.u32(),
                                               Vector{
                                                   b.Builtin(core::BuiltinValue::kSubgroupSize),
                                               }));
        }

        // Add the following to the top of the entry point:
        // {
        //   let gt = subgroup_size > 32;
        //   subgroup_size_mask[0] = select(0xffffffff >> (32 - subgroup_size), 0xffffffff, gt);
        //   subgroup_size_mask[1] = select(0, 0xffffffff >> (64 - subgroup_size), gt);
        // }
        auto* gt = b.Let(b.Sym("gt"), b.GreaterThan(subgroup_size, 32_u));
        auto* lo =
            b.Call("select", b.Shr(0xffffffff_u, b.Sub(32_u, subgroup_size)), 0xffffffff_u, gt);
        auto* hi = b.Call("select", 0_u, b.Shr(0xffffffff_u, b.Sub(64_u, subgroup_size)), gt);
        auto* block = b.Block(Vector{
            b.Decl(gt),
            b.Assign(b.IndexAccessor(subgroup_size_mask, 0_u), lo),
            b.Assign(b.IndexAccessor(subgroup_size_mask, 1_u), hi),
        });
        ctx.InsertFront(ep->body->statements, block);
    }
};

SubgroupBallot::SubgroupBallot() = default;

SubgroupBallot::~SubgroupBallot() = default;

ast::transform::Transform::ApplyResult SubgroupBallot::Apply(const Program& src,
                                                             const ast::transform::DataMap&,
                                                             ast::transform::DataMap&) const {
    return State(src).Run();
}

SubgroupBallot::SimdBallot::~SimdBallot() = default;

const SubgroupBallot::SimdBallot* SubgroupBallot::SimdBallot::Clone(ast::CloneContext& ctx) const {
    return ctx.dst->ASTNodes().Create<SubgroupBallot::SimdBallot>(ctx.dst->ID(),
                                                                  ctx.dst->AllocateNodeID());
}

}  // namespace tint::msl::writer
