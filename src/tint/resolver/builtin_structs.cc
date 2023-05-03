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
    std::string_view name;
    const type::Type* type;
};

type::Struct* BuildStruct(ProgramBuilder& b,
                          builtin::Builtin name,
                          std::initializer_list<NameAndType> member_names_and_types) {
    uint32_t offset = 0;
    uint32_t max_align = 0;
    utils::Vector<const type::StructMember*, 4> members;
    for (auto& m : member_names_and_types) {
        uint32_t align = std::max<uint32_t>(m.type->Align(), 1);
        uint32_t size = m.type->Size();
        offset = utils::RoundUp(align, offset);
        max_align = std::max(max_align, align);
        members.Push(b.create<type::StructMember>(
            /* name */ b.Sym(m.name),
            /* type */ m.type,
            /* index */ static_cast<uint32_t>(members.Length()),
            /* offset */ offset,
            /* align */ align,
            /* size */ size,
            /* attributes */ type::StructMemberAttributes{}));
        offset += size;
    }
    uint32_t size_without_padding = offset;
    uint32_t size_with_padding = utils::RoundUp(max_align, offset);
    return b.create<type::Struct>(
        /* name */ b.Sym(name),
        /* members */ std::move(members),
        /* align */ max_align,
        /* size */ size_with_padding,
        /* size_no_padding */ size_without_padding);
}
}  // namespace

constexpr std::array kModfVecF32Names{
    builtin::Builtin::kModfResultVec2F32,
    builtin::Builtin::kModfResultVec3F32,
    builtin::Builtin::kModfResultVec4F32,
};
constexpr std::array kModfVecF16Names{
    builtin::Builtin::kModfResultVec2F16,
    builtin::Builtin::kModfResultVec3F16,
    builtin::Builtin::kModfResultVec4F16,
};
constexpr std::array kModfVecAbstractNames{
    builtin::Builtin::kModfResultVec2Abstract,
    builtin::Builtin::kModfResultVec3Abstract,
    builtin::Builtin::kModfResultVec4Abstract,
};

type::Struct* CreateModfResult(ProgramBuilder& b, const type::Type* ty) {
    return Switch(
        ty,
        [&](const type::F32*) {
            return BuildStruct(b, builtin::Builtin::kModfResultF32, {{"fract", ty}, {"whole", ty}});
        },  //
        [&](const type::F16*) {
            return BuildStruct(b, builtin::Builtin::kModfResultF16, {{"fract", ty}, {"whole", ty}});
        },
        [&](const type::AbstractFloat*) {
            auto* abstract = BuildStruct(b, builtin::Builtin::kModfResultAbstract,
                                         {{"fract", ty}, {"whole", ty}});
            auto* f32 = b.create<type::F32>();
            auto* f16 = b.create<type::F16>();
            abstract->SetConcreteTypes(utils::Vector{
                BuildStruct(b, builtin::Builtin::kModfResultF32, {{"fract", f32}, {"whole", f32}}),
                BuildStruct(b, builtin::Builtin::kModfResultF16, {{"fract", f16}, {"whole", f16}}),
            });
            return abstract;
        },
        [&](const type::Vector* vec) {
            auto width = vec->Width();
            return Switch(
                vec->type(),  //
                [&](const type::F32*) {
                    return BuildStruct(b, kModfVecF32Names[width - 2],
                                       {{"fract", vec}, {"whole", vec}});
                },
                [&](const type::F16*) {
                    return BuildStruct(b, kModfVecF16Names[width - 2],
                                       {{"fract", vec}, {"whole", vec}});
                },
                [&](const type::AbstractFloat*) {
                    auto* vec_f32 = b.create<type::Vector>(b.create<type::F32>(), width);
                    auto* vec_f16 = b.create<type::Vector>(b.create<type::F16>(), width);
                    auto* abstract = BuildStruct(b, kModfVecAbstractNames[width - 2],
                                                 {{"fract", vec}, {"whole", vec}});
                    abstract->SetConcreteTypes(utils::Vector{
                        BuildStruct(b, kModfVecF32Names[width - 2],
                                    {{"fract", vec_f32}, {"whole", vec_f32}}),
                        BuildStruct(b, kModfVecF16Names[width - 2],
                                    {{"fract", vec_f16}, {"whole", vec_f16}}),
                    });
                    return abstract;
                },
                [&](Default) {
                    TINT_ICE(Resolver, b.Diagnostics())
                        << "unhandled modf type: " << ty->FriendlyName();
                    return nullptr;
                });
        },
        [&](Default) {
            TINT_ICE(Resolver, b.Diagnostics()) << "unhandled modf type: " << ty->FriendlyName();
            return nullptr;
        });
}

constexpr std::array kFrexpVecF32Names{
    builtin::Builtin::kFrexpResultVec2F32,
    builtin::Builtin::kFrexpResultVec3F32,
    builtin::Builtin::kFrexpResultVec4F32,
};
constexpr std::array kFrexpVecF16Names{
    builtin::Builtin::kFrexpResultVec2F16,
    builtin::Builtin::kFrexpResultVec3F16,
    builtin::Builtin::kFrexpResultVec4F16,
};
constexpr std::array kFrexpVecAbstractNames{
    builtin::Builtin::kFrexpResultVec2Abstract,
    builtin::Builtin::kFrexpResultVec3Abstract,
    builtin::Builtin::kFrexpResultVec4Abstract,
};
type::Struct* CreateFrexpResult(ProgramBuilder& b, const type::Type* ty) {
    return Switch(
        ty,  //
        [&](const type::F32*) {
            auto* i32 = b.create<type::I32>();
            return BuildStruct(b, builtin::Builtin::kFrexpResultF32, {{"fract", ty}, {"exp", i32}});
        },
        [&](const type::F16*) {
            auto* i32 = b.create<type::I32>();
            return BuildStruct(b, builtin::Builtin::kFrexpResultF16, {{"fract", ty}, {"exp", i32}});
        },
        [&](const type::AbstractFloat*) {
            auto* f32 = b.create<type::F32>();
            auto* f16 = b.create<type::F16>();
            auto* i32 = b.create<type::I32>();
            auto* ai = b.create<type::AbstractInt>();
            auto* abstract = BuildStruct(b, builtin::Builtin::kFrexpResultAbstract,
                                         {{"fract", ty}, {"exp", ai}});
            abstract->SetConcreteTypes(utils::Vector{
                BuildStruct(b, builtin::Builtin::kFrexpResultF32, {{"fract", f32}, {"exp", i32}}),
                BuildStruct(b, builtin::Builtin::kFrexpResultF16, {{"fract", f16}, {"exp", i32}}),
            });
            return abstract;
        },
        [&](const type::Vector* vec) {
            auto width = vec->Width();
            return Switch(
                vec->type(),  //
                [&](const type::F32*) {
                    auto* vec_i32 = b.create<type::Vector>(b.create<type::I32>(), width);
                    return BuildStruct(b, kFrexpVecF32Names[width - 2],
                                       {{"fract", ty}, {"exp", vec_i32}});
                },
                [&](const type::F16*) {
                    auto* vec_i32 = b.create<type::Vector>(b.create<type::I32>(), width);
                    return BuildStruct(b, kFrexpVecF16Names[width - 2],
                                       {{"fract", ty}, {"exp", vec_i32}});
                },
                [&](const type::AbstractFloat*) {
                    auto* vec_f32 = b.create<type::Vector>(b.create<type::F32>(), width);
                    auto* vec_f16 = b.create<type::Vector>(b.create<type::F16>(), width);
                    auto* vec_i32 = b.create<type::Vector>(b.create<type::I32>(), width);
                    auto* vec_ai = b.create<type::Vector>(b.create<type::AbstractInt>(), width);
                    auto* abstract = BuildStruct(b, kFrexpVecAbstractNames[width - 2],
                                                 {{"fract", ty}, {"exp", vec_ai}});
                    abstract->SetConcreteTypes(utils::Vector{
                        BuildStruct(b, kFrexpVecF32Names[width - 2],
                                    {{"fract", vec_f32}, {"exp", vec_i32}}),
                        BuildStruct(b, kFrexpVecF16Names[width - 2],
                                    {{"fract", vec_f16}, {"exp", vec_i32}}),
                    });
                    return abstract;
                },
                [&](Default) {
                    TINT_ICE(Resolver, b.Diagnostics())
                        << "unhandled frexp type: " << ty->FriendlyName();
                    return nullptr;
                });
        },
        [&](Default) {
            TINT_ICE(Resolver, b.Diagnostics()) << "unhandled frexp type: " << ty->FriendlyName();
            return nullptr;
        });
}

type::Struct* CreateAtomicCompareExchangeResult(ProgramBuilder& b, const type::Type* ty) {
    return Switch(
        ty,  //
        [&](const type::I32*) {
            return BuildStruct(b, builtin::Builtin::kAtomicCompareExchangeResultI32,
                               {{"old_value", ty}, {"exchanged", b.create<type::Bool>()}});
        },
        [&](const type::U32*) {
            return BuildStruct(b, builtin::Builtin::kAtomicCompareExchangeResultU32,
                               {{"old_value", ty}, {"exchanged", b.create<type::Bool>()}});
        },
        [&](Default) {
            TINT_ICE(Resolver, b.Diagnostics())
                << "unhandled atomic_compare_exchange type: " << ty->FriendlyName();
            return nullptr;
        });
}

}  // namespace tint::resolver
