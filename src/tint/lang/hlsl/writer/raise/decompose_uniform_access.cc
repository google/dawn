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

#include "src/tint/lang/hlsl/writer/raise/decompose_uniform_access.h"

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/utils/result/result.h"

namespace tint::hlsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    using VarTypePair = std::pair<core::ir::Var*, const core::type::Type*>;
    /// Maps a struct to the load function
    Hashmap<VarTypePair, core::ir::Function*, 2> var_and_type_to_load_fn_{};

    /// Process the module.
    void Process() {
        Vector<core::ir::Var*, 4> var_worklist;
        for (auto* inst : *ir.root_block) {
            // Allow this to run before or after PromoteInitializers by handling non-var root_block
            // entries
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            // DecomposeStorageAccess maybe have converte the var pointers into ByteAddressBuffer
            // objects. Since they've been changed, then they're Storage buffers and we don't care
            // about them here.
            auto* var_ty = var->Result(0)->Type()->As<core::type::Pointer>();
            if (!var_ty) {
                continue;
            }

            // Only care about uniform address space variables.
            if (var_ty->AddressSpace() != core::AddressSpace::kUniform) {
                continue;
            }

            var_worklist.Push(var);
        }

        for (auto* var : var_worklist) {
            auto* result = var->Result(0);

            auto usage_worklist = result->Usages().Vector();
            auto* var_ty = result->Type()->As<core::type::Pointer>();
            while (!usage_worklist.IsEmpty()) {
                auto usage = usage_worklist.Pop();
                auto* inst = usage.instruction;

                // Load instructions can be destroyed by the replacing access function
                if (!inst->Alive()) {
                    continue;
                }

                Switch(
                    inst,  //
                    [&](core::ir::LoadVectorElement* l) { LoadVectorElement(l, var); },
                    [&](core::ir::Load* l) { Load(l, var); },
                    //                    [&](core::ir::Access* a) {
                    //                        OffsetData offset;
                    //                        Access(a, var, a->Object()->Type(), &offset);
                    //                    },
                    [&](core::ir::Let* let) {
                        // The `let` is, essentially, an alias for the `var` as it's assigned
                        // directly. Gather all the `let` usages into our worklist, and then replace
                        // the `let` with the `var` itself.
                        for (auto& use : let->Result(0)->Usages()) {
                            usage_worklist.Push(use);
                        }
                        let->Result(0)->ReplaceAllUsesWith(result);
                        let->Destroy();
                    },
                    TINT_ICE_ON_NO_MATCH);
            }

            // Swap the result type of the `var` to the new HLSL result type
            auto array_length = (var_ty->StoreType()->Size() + 15) / 16;
            result->SetType(ty.ptr(var_ty->AddressSpace(), ty.array(ty.vec4<u32>(), array_length),
                                   var_ty->Access()));
        }
    }

    void Load(core::ir::Load* ld, core::ir::Var* var) {
        b.InsertBefore(ld, [&] {
            auto* access = b.Access(ty.ptr(uniform, ty.vec4<u32>()), var, 0_u);
            auto* load = b.Load(access);
            auto* bitcast = b.Bitcast(ld->Result(0)->Type(), load);
            ld->Result(0)->ReplaceAllUsesWith(bitcast->Result(0));
        });
        ld->Destroy();
    }

    // A direct vector load on the `var` means the `var` is a vector. Replace the `var` usage` with
    // an access chain into the `var` array `0` element a `load_vector_element` to retrieve the item
    // and a `bitcast` to the correct type.
    void LoadVectorElement(core::ir::LoadVectorElement* lve, core::ir::Var* var) {
        b.InsertBefore(lve, [&] {
            auto* access = b.Access(ty.ptr(uniform, ty.vec4<u32>()), var, 0_u);
            auto* load = b.LoadVectorElement(access, lve->Index());
            auto* bitcast = b.Bitcast(lve->Result(0)->Type(), load);
            lve->Result(0)->ReplaceAllUsesWith(bitcast->Result(0));
        });
        lve->Destroy();
    }
};

}  // namespace

Result<SuccessType> DecomposeUniformAccess(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "DecomposeUniformAccess transform");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
