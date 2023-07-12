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

#include "src/tint/switch.h"
#include "src/tint/symbol_table.h"
#include "src/tint/type/abstract_float.h"
#include "src/tint/type/abstract_int.h"
#include "src/tint/type/bool.h"
#include "src/tint/type/f16.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/manager.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"
#include "src/tint/utils/string.h"

namespace tint::resolver {

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
type::Struct* CreateModfResult(type::Manager& types, SymbolTable& symbols, const type::Type* ty) {
    auto build = [&](builtin::Builtin name, const type::Type* t) {
        return types.Struct(symbols.Register(utils::ToString(name)),
                            {{symbols.Register("fract"), t}, {symbols.Register("whole"), t}});
    };
    return Switch(
        ty,  //
        [&](const type::F32*) { return build(builtin::Builtin::kModfResultF32, ty); },
        [&](const type::F16*) { return build(builtin::Builtin::kModfResultF16, ty); },
        [&](const type::AbstractFloat*) {
            auto* abstract = build(builtin::Builtin::kModfResultAbstract, ty);
            abstract->SetConcreteTypes(utils::Vector{
                build(builtin::Builtin::kModfResultF32, types.f32()),
                build(builtin::Builtin::kModfResultF16, types.f16()),
            });
            return abstract;
        },
        [&](const type::Vector* vec) {
            auto width = vec->Width();
            return Switch(
                vec->type(),  //
                [&](const type::F32*) { return build(kModfVecF32Names[width - 2], vec); },
                [&](const type::F16*) { return build(kModfVecF16Names[width - 2], vec); },
                [&](const type::AbstractFloat*) {
                    auto* abstract = build(kModfVecAbstractNames[width - 2], vec);
                    abstract->SetConcreteTypes(utils::Vector{
                        build(kModfVecF32Names[width - 2], types.vec(types.f32(), width)),
                        build(kModfVecF16Names[width - 2], types.vec(types.f16(), width)),
                    });
                    return abstract;
                },
                [&](Default) {
                    TINT_ASSERT(Resolver, false && "unhandled modf type");
                    return nullptr;
                });
        },
        [&](Default) {
            TINT_ASSERT(Resolver, false && "unhandled modf type");
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
type::Struct* CreateFrexpResult(type::Manager& types, SymbolTable& symbols, const type::Type* ty) {
    auto build = [&](builtin::Builtin name, const type::Type* fract_ty, const type::Type* exp_ty) {
        return types.Struct(
            symbols.Register(utils::ToString(name)),
            {{symbols.Register("fract"), fract_ty}, {symbols.Register("exp"), exp_ty}});
    };
    return Switch(
        ty,  //
        [&](const type::F32*) { return build(builtin::Builtin::kFrexpResultF32, ty, types.i32()); },
        [&](const type::F16*) { return build(builtin::Builtin::kFrexpResultF16, ty, types.i32()); },
        [&](const type::AbstractFloat*) {
            auto* abstract = build(builtin::Builtin::kFrexpResultAbstract, ty, types.AInt());
            abstract->SetConcreteTypes(utils::Vector{
                build(builtin::Builtin::kFrexpResultF32, types.f32(), types.i32()),
                build(builtin::Builtin::kFrexpResultF16, types.f16(), types.i32()),
            });
            return abstract;
        },
        [&](const type::Vector* vec) {
            auto width = vec->Width();
            return Switch(
                vec->type(),  //
                [&](const type::F32*) {
                    return build(kFrexpVecF32Names[width - 2], ty, types.vec(types.i32(), width));
                },
                [&](const type::F16*) {
                    return build(kFrexpVecF16Names[width - 2], ty, types.vec(types.i32(), width));
                },
                [&](const type::AbstractFloat*) {
                    auto* vec_f32 = types.vec(types.f32(), width);
                    auto* vec_f16 = types.vec(types.f16(), width);
                    auto* vec_i32 = types.vec(types.i32(), width);
                    auto* vec_ai = types.vec(types.AInt(), width);
                    auto* abstract = build(kFrexpVecAbstractNames[width - 2], ty, vec_ai);
                    abstract->SetConcreteTypes(utils::Vector{
                        build(kFrexpVecF32Names[width - 2], vec_f32, vec_i32),
                        build(kFrexpVecF16Names[width - 2], vec_f16, vec_i32),
                    });
                    return abstract;
                },
                [&](Default) {
                    TINT_ASSERT(Resolver, false && "unhandled frexp type");
                    return nullptr;
                });
        },
        [&](Default) {
            TINT_ASSERT(Resolver, false && "unhandled frexp type");
            return nullptr;
        });
}

type::Struct* CreateAtomicCompareExchangeResult(type::Manager& types,
                                                SymbolTable& symbols,
                                                const type::Type* ty) {
    auto build = [&](builtin::Builtin name) {
        return types.Struct(symbols.Register(utils::ToString(name)),
                            {
                                {symbols.Register("old_value"), ty},
                                {symbols.Register("exchanged"), types.bool_()},
                            });
    };
    return Switch(
        ty,  //
        [&](const type::I32*) { return build(builtin::Builtin::kAtomicCompareExchangeResultI32); },
        [&](const type::U32*) { return build(builtin::Builtin::kAtomicCompareExchangeResultU32); },
        [&](Default) {
            TINT_ASSERT(Resolver, false && "unhandled atomic_compare_exchange type");
            return nullptr;
        });
}

}  // namespace tint::resolver
