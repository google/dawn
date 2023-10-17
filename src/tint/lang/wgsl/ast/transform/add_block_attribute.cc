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

#include "src/tint/lang/wgsl/ast/transform/add_block_attribute.h"

#include <unordered_set>
#include <utility>

#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/variable.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/containers/hashset.h"

TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::AddBlockAttribute);
TINT_INSTANTIATE_TYPEINFO(tint::ast::transform::AddBlockAttribute::BlockAttribute);

namespace tint::ast::transform {

AddBlockAttribute::AddBlockAttribute() = default;

AddBlockAttribute::~AddBlockAttribute() = default;

Transform::ApplyResult AddBlockAttribute::Apply(const Program& src,
                                                const DataMap&,
                                                DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};

    auto& sem = src.Sem();

    // A map from a type in the source program to a block-decorated wrapper that contains it in the
    // destination program.
    Hashmap<const core::type::Type*, const Struct*, 8> wrapper_structs;

    // Process global 'var' declarations that are buffers.
    bool made_changes = false;
    for (auto* global : src.AST().GlobalVariables()) {
        auto* var = sem.Get(global);
        if (!core::IsHostShareable(var->AddressSpace())) {
            // Not declared in a host-sharable address space
            continue;
        }

        made_changes = true;

        auto* ty = var->Type()->UnwrapRef();
        auto* str = ty->As<sem::Struct>();

        // Always try to wrap the buffer type into a struct. We can not do so only if it is a struct
        // but without a fixed footprint, i.e. contains a runtime-sized array as its member. Note
        // that such struct type can be only used as storage buffer variables' type. Also note that
        // any buffer struct type that may be nested by another type must have a fixed footprint,
        // therefore will be wrapped.
        bool needs_wrapping = !str ||                    // Type is not a structure
                              str->HasFixedFootprint();  // Struct has a fixed footprint

        if (needs_wrapping) {
            const char* kMemberName = "inner";

            auto* wrapper = wrapper_structs.GetOrCreate(ty, [&] {
                auto* block = b.ASTNodes().Create<BlockAttribute>(b.ID(), b.AllocateNodeID());
                auto wrapper_name = global->name->symbol.Name() + "_block";
                auto* ret =
                    b.create<Struct>(b.Ident(b.Symbols().New(wrapper_name)),
                                     tint::Vector{b.Member(kMemberName, CreateASTTypeFor(ctx, ty))},
                                     tint::Vector{block});
                ctx.InsertBefore(src.AST().GlobalDeclarations(), global, ret);
                return ret;
            });
            ctx.Replace(global->type.expr, b.Expr(wrapper->name->symbol));

            // Insert a member accessor to get the original type from the wrapper at
            // any usage of the original variable.
            for (auto* user : var->Users()) {
                ctx.Replace(user->Declaration(),
                            b.MemberAccessor(ctx.Clone(global->name->symbol), kMemberName));
            }
        } else {
            // Add a block attribute to this struct directly.
            auto* block = b.ASTNodes().Create<BlockAttribute>(b.ID(), b.AllocateNodeID());
            ctx.InsertFront(str->Declaration()->attributes, block);
        }
    }

    if (!made_changes) {
        return SkipTransform;
    }

    ctx.Clone();
    return resolver::Resolve(b);
}

AddBlockAttribute::BlockAttribute::BlockAttribute(GenerationID pid, NodeID nid)
    : Base(pid, nid, tint::Empty) {}
AddBlockAttribute::BlockAttribute::~BlockAttribute() = default;
std::string AddBlockAttribute::BlockAttribute::InternalName() const {
    return "block";
}

const AddBlockAttribute::BlockAttribute* AddBlockAttribute::BlockAttribute::Clone(
    ast::CloneContext& ctx) const {
    return ctx.dst->ASTNodes().Create<AddBlockAttribute::BlockAttribute>(ctx.dst->ID(),
                                                                         ctx.dst->AllocateNodeID());
}

}  // namespace tint::ast::transform
