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

#include "src/tint/lang/wgsl/ast/transform/push_constant_helper.h"

#include <utility>

#include "src/tint/lang/wgsl/ast/module.h"
#include "src/tint/lang/wgsl/ast/var.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/sem/struct.h"
#include "src/tint/lang/wgsl/sem/variable.h"

namespace tint::ast::transform {

PushConstantHelper::PushConstantHelper(program::CloneContext& c) : ctx(c) {
    // Find first existing push_constant, if any.
    for (auto* global : ctx.src->AST().GlobalVariables()) {
        if (auto* var = global->As<ast::Var>()) {
            auto* v = ctx.src->Sem().Get(var);
            if (v->AddressSpace() == core::AddressSpace::kPushConstant) {
                push_constants_var = var;
                auto* str = v->Type()->UnwrapRef()->As<sem::Struct>();
                if (DAWN_UNLIKELY(!str)) {
                    TINT_ICE() << "expected var<push_constant> type to be struct. Was "
                                  "AddBlockAttribute run?";
                }
                // Clone all members from the existing block and insert them into the map.
                for (auto* member : str->Members()) {
                    member_map[member->Offset()] = ctx.CloneWithoutTransform(member->Declaration());
                }
                break;
            }
        }
    }
}

void PushConstantHelper::InsertMember(const char* name, ast::Type type, uint32_t offset) {
    auto& member = member_map[offset];
    if (DAWN_UNLIKELY(member != nullptr)) {
        ctx.dst->Diagnostics().AddError(Source{}) << "struct member offset collision";
    }
    member = ctx.dst->Member(name, type, Vector{ctx.dst->MemberOffset(core::AInt(offset))});
}

Symbol PushConstantHelper::Run() {
    Vector<const tint::ast::StructMember*, 8> members;
    for (auto i : member_map) {
        members.Push(i.second);
    }

    new_struct = ctx.dst->Structure(ctx.dst->Symbols().New("PushConstants"), std::move(members));

    Symbol buffer_name;

    // If this is the first use of push constants, create a global to hold them.
    if (!push_constants_var) {
        ctx.dst->Enable(wgsl::Extension::kChromiumExperimentalPushConstant);

        buffer_name = ctx.dst->Symbols().New("push_constants");
        ctx.dst->GlobalVar(buffer_name, ctx.dst->ty.Of(new_struct),
                           core::AddressSpace::kPushConstant);
    } else {
        buffer_name = ctx.Clone(push_constants_var->name->symbol);

        // Replace all variable users of the old struct with the new struct.
        ctx.ReplaceAll([this](const ast::Variable* var) -> const ast::Variable* {
            if (ctx.src->Sem().Get(var)->AddressSpace() == core::AddressSpace::kPushConstant) {
                if (var->As<ast::Parameter>()) {
                    return ctx.dst->Param(ctx.Clone(var->name->symbol), ctx.dst->ty.Of(new_struct),
                                          ctx.Clone(var->attributes));
                } else {
                    return ctx.dst->Var(ctx.Clone(var->name->symbol), ctx.dst->ty.Of(new_struct),
                                        ctx.Clone(var->attributes),
                                        core::AddressSpace::kPushConstant);
                }
            }
            return nullptr;
        });
    }

    return buffer_name;
}

PushConstantHelper::~PushConstantHelper() = default;

}  // namespace tint::ast::transform
