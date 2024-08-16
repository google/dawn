// Copyright 2024 The Dawn & Tint Authors
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

#include "src/tint/lang/msl/writer/ast_raise/quad_swap.h"

#include <utility>

#include "src/tint/lang/core/builtin_fn.h"
#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/wgsl/ast/transform/transform.h"
#include "src/tint/lang/wgsl/builtin_fn.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/function.h"
#include "src/tint/lang/wgsl/sem/statement.h"

TINT_INSTANTIATE_TYPEINFO(tint::msl::writer::QuadSwap);
TINT_INSTANTIATE_TYPEINFO(tint::msl::writer::QuadSwap::QuadShuffle);
TINT_INSTANTIATE_TYPEINFO(tint::msl::writer::QuadSwap::ThreadIndexInQuadgroup);

using namespace tint::core::number_suffixes;  // NOLINT
using namespace tint::core::fluent_types;     // NOLINT

namespace tint::msl::writer {

/// PIMPL state for the transform
struct QuadSwap::State {
    /// The source program
    const Program& src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    program::CloneContext ctx = {&b, &src, /* auto_clone_symbols */ true};

    /// The set of names for the tint_msl_quadSwap* helper functions.
    Hashmap<std::pair<wgsl::BuiltinFn, const core::type::Type*>, Symbol, 8> quad_swap_helpers;

    /// The set of names for the tint_msl_quad_shuffle* intrinsic functions.
    Hashmap<const core::type::Type*, Symbol, 8> quad_shuffle_intrinsics;

    /// The name of the `tint_msl_thread_index_in_quadgroup` global variable.
    Symbol thread_index_in_quadgroup{};

    /// The set of a functions that directly call `quadSwap*()` builtin functions.
    Hashset<const sem::Function*, 4> quad_swap_callers;

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
                // If this is a call to a `quadSwap*()` builtin, replace it with a call to the
                // helper function and make a note of the function that we are in.
                auto* builtin = call->Target()->As<sem::BuiltinFn>();
                if (builtin && builtin->IsQuadSwap()) {
                    auto* expr = ctx.Clone(call->Arguments()[0]->Declaration());
                    ctx.Replace(call->Declaration(),
                                b.Call(GetHelper(builtin->Fn(), call->Type()), expr));
                    quad_swap_callers.Add(call->Stmt()->Function());
                    made_changes = true;
                }
            }
        }
        if (!made_changes) {
            return SkipTransform;
        }

        // Set the thread_index_in_quadgroup global variable at the start of each entry point that
        // transitively calls the `quadSwap*()` builtin function.
        for (auto* global : src.AST().GlobalDeclarations()) {
            auto* func = global->As<ast::Function>();
            if (func && func->IsEntryPoint() && TransitvelyCallsQuadSwap(sem.Get(func))) {
                SetThreadIndexInQuadgroup(func);
            }
        }

        ctx.Clone();
        return resolver::Resolve(b);
    }

    /// Get or create the appropriate `tint_msl_quadSwap*` helper function for the given builtin
    /// function and return type.
    /// @returns the name of the helper function
    Symbol GetHelper(wgsl::BuiltinFn func, const core::type::Type* type) {
        return quad_swap_helpers.GetOrAdd(std::make_pair(func, type), [&] {
            Symbol quad_swap_helper = b.Symbols().New(std::string("tint_msl_") + str(func));
            Symbol intrinsic = GetIntrinsic(type);

            // Declare the `tint_msl_quadSwap*` helper function as follows:
            //   fn tint_msl_quadSwapFnName(e : T) -> T {
            //     tint_msl_quad_shuffle(e, tint_msl_thread_index_in_quadgroup ^ rhs)
            //   }
            // where rhs is determined based on the builtin function call:
            // +------------------+------+
            // |       func       | rhs  |
            // +------------------+------+
            // | quadSwapX        | 0b1  |
            // | quadSwapY        | 0b10 |
            // | quadSwapDiagonal | 0b11 |
            // +------------------+------+
            auto* expr = b.Param("e", CreateASTTypeFor(ctx, type));
            core::u32 rhs;
            switch (func) {
                case wgsl::BuiltinFn::kQuadSwapX:
                    rhs = 0b1;
                    break;
                case wgsl::BuiltinFn::kQuadSwapY:
                    rhs = 0b10;
                    break;
                case wgsl::BuiltinFn::kQuadSwapDiagonal:
                    rhs = 0b11;
                    break;
                default:
                    TINT_UNREACHABLE() << "unsupported builtin function";
            }
            b.Func(quad_swap_helper, Vector{expr}, CreateASTTypeFor(ctx, type),
                   Vector{
                       b.Return(b.Call(intrinsic, expr, b.Xor(GetGlobalVar(), rhs))),
                   });
            return quad_swap_helper;
        });
    }

    /// Get or create the `tint_msl_quad_shuffle` intrinsic placeholder function for the given
    /// return type.
    /// @returns the name of the function
    Symbol GetIntrinsic(const core::type::Type* type) {
        return quad_shuffle_intrinsics.GetOrAdd(type, [&] {
            auto intrinsic = b.Symbols().New(std::string("tint_msl_quad_shuffle"));

            // Declare the `tint_msl_quad_shuffle` function, which will be replaced by the MSL
            // `quad_shuffle` intrinsic function to perform the swap.
            {
                auto* data = b.Param("data", CreateASTTypeFor(ctx, type));
                auto* quad_lane_id = b.Param("quad_lane_id", b.ty.u32());
                b.Func(intrinsic, Vector{data, quad_lane_id}, CreateASTTypeFor(ctx, type), nullptr,
                       Vector{b.ASTNodes().Create<QuadShuffle>(b.ID(), b.AllocateNodeID()),
                              b.Disable(ast::DisabledValidation::kFunctionHasNoBody)});
            }

            return intrinsic;
        });
    }

    /// Get or create the `tint_msl_thread_index_in_quadgroup` global variable.
    /// @returns the name of the variable
    Symbol GetGlobalVar() {
        if (!thread_index_in_quadgroup) {
            thread_index_in_quadgroup = b.Symbols().New("tint_msl_thread_index_in_quadgroup");

            // Declare the `tint_msl_thread_index_in_quadgroup` variable.
            b.GlobalVar(thread_index_in_quadgroup, core::AddressSpace::kPrivate, b.ty.u32());
        }

        return thread_index_in_quadgroup;
    }

    /// Check if a function directly or transitively calls a `quadSwap*()` builtin.
    /// @param func the function to check
    /// @returns true if the function transitively calls `quadSwap*()`
    bool TransitvelyCallsQuadSwap(const sem::Function* func) {
        if (quad_swap_callers.Contains(func)) {
            return true;
        }
        for (auto* called : func->TransitivelyCalledFunctions()) {
            if (quad_swap_callers.Contains(called)) {
                return true;
            }
        }
        return false;
    }

    /// Add code to set the `thread_index_in_quadgroup` variable at the start of an entry point.
    /// @param ep the entry point
    void SetThreadIndexInQuadgroup(const ast::Function* ep) {
        // Add the entry point parameter with an attribute to indicate to the the MSL backend that
        // it should be annotated with [[thread_index_in_quadgroup]].
        Symbol thread_index_in_quadgroup_param = b.Symbols().New("tint_thread_index_in_quadgroup");
        ctx.InsertBack(ep->params, b.Param(thread_index_in_quadgroup_param, b.ty.u32(),
                                           Vector{b.ASTNodes().Create<ThreadIndexInQuadgroup>(
                                               b.ID(), b.AllocateNodeID())}));

        // Add the following to the top of the entry point:
        // {
        //   tint_msl_thread_index_in_quadgroup = tint_thread_index_in_quadgroup;
        // }
        auto* block = b.Block(Vector{
            b.Assign(thread_index_in_quadgroup, thread_index_in_quadgroup_param),
        });
        ctx.InsertFront(ep->body->statements, block);
    }
};

QuadSwap::QuadSwap() = default;

QuadSwap::~QuadSwap() = default;

ast::transform::Transform::ApplyResult QuadSwap::Apply(const Program& src,
                                                       const ast::transform::DataMap&,
                                                       ast::transform::DataMap&) const {
    return State(src).Run();
}

QuadSwap::QuadShuffle::~QuadShuffle() = default;

const QuadSwap::QuadShuffle* QuadSwap::QuadShuffle::Clone(ast::CloneContext& ctx) const {
    return ctx.dst->ASTNodes().Create<QuadSwap::QuadShuffle>(ctx.dst->ID(),
                                                             ctx.dst->AllocateNodeID());
}

QuadSwap::ThreadIndexInQuadgroup::~ThreadIndexInQuadgroup() = default;

const QuadSwap::ThreadIndexInQuadgroup* QuadSwap::ThreadIndexInQuadgroup::Clone(
    ast::CloneContext& ctx) const {
    return ctx.dst->ASTNodes().Create<QuadSwap::ThreadIndexInQuadgroup>(ctx.dst->ID(),
                                                                        ctx.dst->AllocateNodeID());
}

}  // namespace tint::msl::writer
