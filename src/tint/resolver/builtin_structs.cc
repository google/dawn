// Copyright 2023 The Tint Authors.
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

#include "src/tint/resolver/builtin_structs.h"

#include <algorithm>
#include <string>
#include <utility>

#include "src/tint/program_builder.h"
#include "src/tint/switch.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/vector.h"

namespace tint::resolver {

namespace {

struct NameAndType {
    std::string name;
    const type::Type* type;
};

sem::Struct* BuildStruct(ProgramBuilder& b,
                         std::string name,
                         std::initializer_list<NameAndType> member_names_and_types) {
    uint32_t offset = 0;
    uint32_t max_align = 0;
    utils::Vector<const sem::StructMember*, 4> members;
    for (auto& m : member_names_and_types) {
        uint32_t align = std::max<uint32_t>(m.type->Align(), 1);
        uint32_t size = m.type->Size();
        offset = utils::RoundUp(align, offset);
        max_align = std::max(max_align, align);
        members.Push(b.create<sem::StructMember>(
            /* declaration */ nullptr,
            /* source */ Source{},
            /* name */ b.Sym(m.name),
            /* type */ m.type,
            /* index */ static_cast<uint32_t>(members.Length()),
            /* offset */ offset,
            /* align */ align,
            /* size */ size,
            /* location */ std::nullopt));
        offset += size;
    }
    uint32_t size_without_padding = offset;
    uint32_t size_with_padding = utils::RoundUp(max_align, offset);
    return b.create<sem::Struct>(
        /* declaration */ nullptr,
        /* source */ Source{},
        /* name */ b.Sym(name),
        /* members */ std::move(members),
        /* align */ max_align,
        /* size */ size_with_padding,
        /* size_no_padding */ size_without_padding);
}
}  // namespace

sem::Struct* CreateModfResult(ProgramBuilder& b, const type::Type* ty) {
    return Switch(
        ty,
        [&](const type::F32*) {
            return BuildStruct(b, "__modf_result_f32", {{"fract", ty}, {"whole", ty}});
        },  //
        [&](const type::F16*) {
            return BuildStruct(b, "__modf_result_f16", {{"fract", ty}, {"whole", ty}});
        },
        [&](const type::AbstractFloat*) {
            auto* abstract =
                BuildStruct(b, "__modf_result_abstract", {{"fract", ty}, {"whole", ty}});
            auto* f32 = b.create<type::F32>();
            auto* f16 = b.create<type::F16>();
            abstract->SetConcreteTypes(utils::Vector{
                BuildStruct(b, "__modf_result_f32", {{"fract", f32}, {"whole", f32}}),
                BuildStruct(b, "__modf_result_f16", {{"fract", f16}, {"whole", f16}}),
            });
            return abstract;
        },
        [&](const type::Vector* vec) {
            auto width = vec->Width();
            auto prefix = "__modf_result_vec" + std::to_string(width);
            return Switch(
                vec->type(),  //
                [&](const type::F32*) {
                    return BuildStruct(b, prefix + "_f32", {{"fract", vec}, {"whole", vec}});
                },
                [&](const type::F16*) {
                    return BuildStruct(b, prefix + "_f16", {{"fract", vec}, {"whole", vec}});
                },
                [&](const type::AbstractFloat*) {
                    auto* vec_f32 = b.create<type::Vector>(b.create<type::F32>(), width);
                    auto* vec_f16 = b.create<type::Vector>(b.create<type::F16>(), width);
                    auto* abstract =
                        BuildStruct(b, prefix + "_abstract", {{"fract", vec}, {"whole", vec}});
                    abstract->SetConcreteTypes(utils::Vector{
                        BuildStruct(b, prefix + "_f32", {{"fract", vec_f32}, {"whole", vec_f32}}),
                        BuildStruct(b, prefix + "_f16", {{"fract", vec_f16}, {"whole", vec_f16}}),
                    });
                    return abstract;
                },
                [&](Default) {
                    TINT_ICE(Resolver, b.Diagnostics())
                        << "unhandled modf type: " << b.FriendlyName(ty);
                    return nullptr;
                });
        },
        [&](Default) {
            TINT_ICE(Resolver, b.Diagnostics()) << "unhandled modf type: " << b.FriendlyName(ty);
            return nullptr;
        });
}

sem::Struct* CreateFrexpResult(ProgramBuilder& b, const type::Type* ty) {
    return Switch(
        ty,  //
        [&](const type::F32*) {
            auto* i32 = b.create<type::I32>();
            return BuildStruct(b, "__frexp_result_f32", {{"fract", ty}, {"exp", i32}});
        },
        [&](const type::F16*) {
            auto* i32 = b.create<type::I32>();
            return BuildStruct(b, "__frexp_result_f16", {{"fract", ty}, {"exp", i32}});
        },
        [&](const type::AbstractFloat*) {
            auto* f32 = b.create<type::F32>();
            auto* f16 = b.create<type::F16>();
            auto* i32 = b.create<type::I32>();
            auto* ai = b.create<type::AbstractInt>();
            auto* abstract =
                BuildStruct(b, "__frexp_result_abstract", {{"fract", ty}, {"exp", ai}});
            abstract->SetConcreteTypes(utils::Vector{
                BuildStruct(b, "__frexp_result_f32", {{"fract", f32}, {"exp", i32}}),
                BuildStruct(b, "__frexp_result_f16", {{"fract", f16}, {"exp", i32}}),
            });
            return abstract;
        },
        [&](const type::Vector* vec) {
            auto width = vec->Width();
            auto prefix = "__frexp_result_vec" + std::to_string(width);
            return Switch(
                vec->type(),  //
                [&](const type::F32*) {
                    auto* vec_i32 = b.create<type::Vector>(b.create<type::I32>(), width);
                    return BuildStruct(b, prefix + "_f32", {{"fract", ty}, {"exp", vec_i32}});
                },
                [&](const type::F16*) {
                    auto* vec_i32 = b.create<type::Vector>(b.create<type::I32>(), width);
                    return BuildStruct(b, prefix + "_f16", {{"fract", ty}, {"exp", vec_i32}});
                },
                [&](const type::AbstractFloat*) {
                    auto* vec_f32 = b.create<type::Vector>(b.create<type::F32>(), width);
                    auto* vec_f16 = b.create<type::Vector>(b.create<type::F16>(), width);
                    auto* vec_i32 = b.create<type::Vector>(b.create<type::I32>(), width);
                    auto* vec_ai = b.create<type::Vector>(b.create<type::AbstractInt>(), width);
                    auto* abstract =
                        BuildStruct(b, prefix + "_abstract", {{"fract", ty}, {"exp", vec_ai}});
                    abstract->SetConcreteTypes(utils::Vector{
                        BuildStruct(b, prefix + "_f32", {{"fract", vec_f32}, {"exp", vec_i32}}),
                        BuildStruct(b, prefix + "_f16", {{"fract", vec_f16}, {"exp", vec_i32}}),
                    });
                    return abstract;
                },
                [&](Default) {
                    TINT_ICE(Resolver, b.Diagnostics())
                        << "unhandled frexp type: " << b.FriendlyName(ty);
                    return nullptr;
                });
        },
        [&](Default) {
            TINT_ICE(Resolver, b.Diagnostics()) << "unhandled frexp type: " << b.FriendlyName(ty);
            return nullptr;
        });
}

sem::Struct* CreateAtomicCompareExchangeResult(ProgramBuilder& b, const type::Type* ty) {
    return BuildStruct(
        b, "__atomic_compare_exchange_result" + ty->FriendlyName(),
        {{"old_value", const_cast<type::Type*>(ty)}, {"exchanged", b.create<type::Bool>()}});
}

}  // namespace tint::resolver
