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

#include "src/tint/lang/hlsl/writer/raise/decompose_memory_access.h"

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/member_builtin_call.h"
#include "src/tint/lang/hlsl/type/byte_address_buffer.h"
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

            // Var must be a pointer
            auto* var_ty = var->Result(0)->Type()->As<core::type::Pointer>();
            TINT_ASSERT(var_ty);

            // Only care about storage address space variables.
            if (var_ty->AddressSpace() != core::AddressSpace::kStorage) {
                continue;
            }

            var_worklist.Push(var);
        }

        for (auto* var : var_worklist) {
            auto* var_ty = var->Result(0)->Type()->As<core::type::Pointer>();

            core::type::Type* buf_type =
                ty.Get<hlsl::type::ByteAddressBuffer>(var_ty->StoreType(), var_ty->Access());

            // Swap the result type of the `var` to the new HLSL result type
            auto* result = var->Result(0);
            result->SetType(buf_type);

            // Find all the usages of the `var` which is loading or storing.
            Vector<core::ir::Instruction*, 4> usage_worklist;
            for (auto& usage : result->Usages()) {
                Switch(
                    usage->instruction,  //
                    [&](core::ir::LoadVectorElement* lve) { usage_worklist.Push(lve); },
                    [&](core::ir::StoreVectorElement* sve) { usage_worklist.Push(sve); },  //

                    [&](core::ir::Store* st) { usage_worklist.Push(st); },  //
                    [&](core::ir::Load* ld) { usage_worklist.Push(ld); }    //
                );
            }

            for (auto* inst : usage_worklist) {
                Switch(
                    inst,  //
                    [&](core::ir::LoadVectorElement* lve) {
                        // Converts to:
                        //
                        // %1:u32 = v.Load 0u
                        // %b:f32 = bitcast %1

                        auto* idx_value = lve->Index()->As<core::ir::Constant>();
                        TINT_ASSERT(idx_value);

                        uint32_t pos = idx_value->Value()->ValueAs<uint32_t>() *
                                       var_ty->StoreType()->DeepestElement()->Size();

                        auto* builtin = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                            ty.u32(), BuiltinFn::kLoad, var, u32(pos));

                        auto* cast = b.Bitcast(lve->Result(0)->Type(), builtin->Result(0));
                        lve->Result(0)->ReplaceAllUsesWith(cast->Result(0));

                        builtin->InsertBefore(lve);
                        cast->InsertBefore(lve);
                        lve->Destroy();
                    },
                    [&](core::ir::StoreVectorElement* sve) {
                        // Converts to:
                        //
                        // %1 = <sve->Value()>
                        // %2:u32 = bitcast %1
                        // %3:void = v.Store 0u, %2

                        auto* idx_value = sve->Index()->As<core::ir::Constant>();
                        TINT_ASSERT(idx_value);

                        uint32_t pos = idx_value->Value()->ValueAs<uint32_t>() *
                                       var_ty->StoreType()->DeepestElement()->Size();

                        auto* cast = b.Bitcast(ty.u32(), sve->Value());
                        auto* builtin = b.MemberCall<hlsl::ir::MemberBuiltinCall>(
                            ty.void_(), BuiltinFn::kStore, var, u32(pos), cast);

                        cast->InsertBefore(sve);
                        builtin->InsertBefore(sve);
                        sve->Destroy();
                    },

                    [&](core::ir::Store*) {},  //
                    [&](core::ir::Load*) {}    //
                );
            }
        }
    }
};

}  // namespace

Result<SuccessType> DecomposeMemoryAccess(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "DecomposeMemoryAccess transform");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
