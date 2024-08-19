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

#include "src/tint/lang/glsl/writer/ast_raise/pad_structs.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "src/tint/lang/wgsl/ast/disable_validation_attribute.h"
#include "src/tint/lang/wgsl/ast/parameter.h"
#include "src/tint/lang/wgsl/ast/transform/add_block_attribute.h"
#include "src/tint/lang/wgsl/program/clone_context.h"
#include "src/tint/lang/wgsl/program/program_builder.h"
#include "src/tint/lang/wgsl/resolver/resolve.h"
#include "src/tint/lang/wgsl/sem/call.h"
#include "src/tint/lang/wgsl/sem/module.h"
#include "src/tint/lang/wgsl/sem/value_constructor.h"

using namespace tint::core::number_suffixes;  // NOLINT

TINT_INSTANTIATE_TYPEINFO(tint::glsl::writer::PadStructs);

namespace tint::glsl::writer {

namespace {

void CreatePadding(Vector<const ast::StructMember*, 8>* new_members,
                   Hashset<const ast::StructMember*, 8>* padding_members,
                   Hashset<std::string, 8>* member_names,
                   int& last_padding_index,
                   ast::Builder* b,
                   uint32_t bytes) {
    const size_t count = bytes / 4u;
    padding_members->Reserve(count);
    new_members->Reserve(count);
    member_names->Reserve(count);
    std::string name = "pad";
    for (uint32_t i = 0; i < count; ++i) {
        while (member_names->Contains(name)) {
            name = "pad_" + std::to_string(++last_padding_index);
        }
        member_names->Add(name);
        auto* member = b->Member(name, b->ty.u32());
        padding_members->Add(member);
        new_members->Push(member);
    }
}

}  // namespace

PadStructs::PadStructs() = default;

PadStructs::~PadStructs() = default;

ast::transform::Transform::ApplyResult PadStructs::Apply(const Program& src,
                                                         const ast::transform::DataMap&,
                                                         ast::transform::DataMap&) const {
    ProgramBuilder b;
    program::CloneContext ctx{&b, &src, /* auto_clone_symbols */ true};
    auto& sem = src.Sem();

    std::unordered_map<const ast::Struct*, const ast::Struct*> replaced_structs;
    Hashset<const ast::StructMember*, 8> padding_members;

    ctx.ReplaceAll([&](const ast::Struct* ast_str) -> const ast::Struct* {
        auto* str = sem.Get(ast_str);
        if (!str || !str->IsHostShareable()) {
            return nullptr;
        }
        uint32_t offset = 0;
        tint::Vector<const ast::StructMember*, 8> new_members;
        Hashset<std::string, 8> member_names;
        for (auto* mem : str->Members()) {
            member_names.Add(mem->Name().Name());
        }

        int last_padding_index = 0;
        for (auto* mem : str->Members()) {
            auto name = mem->Name().Name();

            if (offset < mem->Offset()) {
                CreatePadding(&new_members, &padding_members, &member_names, last_padding_index,
                              ctx.dst, mem->Offset() - offset);
                offset = mem->Offset();
            }

            auto* ty = mem->Type();
            auto type = CreateASTTypeFor(ctx, ty);

            uint32_t size = ty->Size();
            if (ty->Is<core::type::Struct>() && str->UsedAs(core::AddressSpace::kUniform)) {
                // std140 structs should be padded out to 16 bytes.
                uint32_t rounded_size = tint::RoundUp(16u, size);

                // Add a @size attribute to satisfy WGSL validator rules.
                // But do not add manual padding for the part of (rounded_size - size),
                // which is implicitly padded by std140.
                new_members.Push(
                    b.Member(name, type, tint::Vector{b.MemberSize(core::AInt(rounded_size))}));

                offset += rounded_size;
            } else {
                new_members.Push(b.Member(name, type));
                offset += size;
            }
        }

        uint32_t struct_size = str->Size();

        // Add any required padding after the last member, if it's not a block.
        bool is_block = ast::HasAttribute<ast::transform::AddBlockAttribute::BlockAttribute>(
            ast_str->attributes);
        if (offset < struct_size && !is_block) {
            CreatePadding(&new_members, &padding_members, &member_names, last_padding_index,
                          ctx.dst, struct_size - offset);
        }

        tint::Vector<const ast::Attribute*, 1> struct_attribs = ctx.Clone(ast_str->attributes);
        if (!padding_members.IsEmpty()) {
            struct_attribs.Push(b.Disable(ast::DisabledValidation::kIgnoreStructMemberLimit));
        }

        auto* new_struct = b.create<ast::Struct>(ctx.Clone(ast_str->name), std::move(new_members),
                                                 std::move(struct_attribs));
        replaced_structs[ast_str] = new_struct;
        return new_struct;
    });

    ctx.ReplaceAll([&](const ast::CallExpression* ast_call) -> const ast::CallExpression* {
        if (ast_call->args.Length() == 0) {
            return nullptr;
        }

        auto* call = sem.Get<sem::Call>(ast_call);
        if (!call) {
            return nullptr;
        }
        auto* cons = call->Target()->As<sem::ValueConstructor>();
        if (!cons) {
            return nullptr;
        }
        auto* str = cons->ReturnType()->As<sem::Struct>();
        if (!str) {
            return nullptr;
        }

        auto* new_struct = replaced_structs[str->Declaration()];
        if (!new_struct) {
            return nullptr;
        }

        tint::Vector<const ast::Expression*, 8> new_args;

        auto arg = ast_call->args.begin();
        for (auto* member : new_struct->members) {
            if (padding_members.Contains(member)) {
                new_args.Push(b.Expr(0_u));
            } else {
                new_args.Push(ctx.Clone(*arg));
                ++arg;
            }
        }
        return b.Call(CreateASTTypeFor(ctx, str), new_args);
    });

    ctx.Clone();
    return resolver::Resolve(b);
}

}  // namespace tint::glsl::writer
