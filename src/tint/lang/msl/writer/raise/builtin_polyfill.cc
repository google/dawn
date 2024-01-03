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

#include "src/tint/lang/msl/writer/raise/builtin_polyfill.h"

#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/msl/barrier_type.h"
#include "src/tint/lang/msl/ir/builtin_call.h"

namespace tint::msl::writer::raise {
namespace {

using namespace tint::core::fluent_types;  // NOLINT

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
        // Find the builtins that need replacing.
        Vector<core::ir::CoreBuiltinCall*, 4> worklist;
        for (auto* inst : ir.instructions.Objects()) {
            if (!inst->Alive()) {
                continue;
            }
            if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (builtin->Func()) {
                    case core::BuiltinFn::kStorageBarrier:
                    case core::BuiltinFn::kWorkgroupBarrier:
                    case core::BuiltinFn::kTextureBarrier:
                        worklist.Push(builtin);
                        break;
                    default:
                        break;
                }
            }
        }

        // Replace the builtins that we found.
        for (auto* builtin : worklist) {
            core::ir::Value* replacement = nullptr;
            switch (builtin->Func()) {
                case core::BuiltinFn::kStorageBarrier:
                    replacement = StorageBarrier(builtin);
                    break;
                case core::BuiltinFn::kWorkgroupBarrier:
                    replacement = WorkgroupBarrier(builtin);
                    break;
                case core::BuiltinFn::kTextureBarrier:
                    replacement = TextureBarrier(builtin);
                    break;
                default:
                    break;
            }
            TINT_ASSERT_OR_RETURN(replacement);

            // Replace the old builtin result with the new value.
            if (auto name = ir.NameOf(builtin->Result(0))) {
                ir.SetName(replacement, name);
            }
            builtin->Result(0)->ReplaceAllUsesWith(replacement);
            builtin->Destroy();
        }
    }

    /// Handle a `workgroupBarrier()` builtin.
    /// @param builtin the builtin call instruction
    /// @returns the replacement value
    core::ir::Value* WorkgroupBarrier(core::ir::CoreBuiltinCall* builtin) {
        // Replace the builtin call with a call to the msl.threadgroup_barrier intrinsic.
        auto args = Vector<core::ir::Value*, 4>{b.Constant(u32(BarrierType::kThreadGroup))};

        auto* call = b.Call<msl::ir::BuiltinCall>(
            builtin->Result(0)->Type(), msl::BuiltinFn::kThreadgroupBarrier, std::move(args));
        call->InsertBefore(builtin);
        return call->Result(0);
    }

    /// Handle a `storageBarrier()` builtin.
    /// @param builtin the builtin call instruction
    /// @returns the replacement value
    core::ir::Value* StorageBarrier(core::ir::CoreBuiltinCall* builtin) {
        // Replace the builtin call with a call to the msl.threadgroup_barrier intrinsic.
        auto args = Vector<core::ir::Value*, 4>{b.Constant(u32(BarrierType::kDevice))};

        auto* call = b.Call<msl::ir::BuiltinCall>(
            builtin->Result(0)->Type(), msl::BuiltinFn::kThreadgroupBarrier, std::move(args));
        call->InsertBefore(builtin);
        return call->Result(0);
    }

    /// Handle a `textureBarrier()` builtin.
    /// @param builtin the builtin call instruction
    /// @returns the replacement value
    core::ir::Value* TextureBarrier(core::ir::CoreBuiltinCall* builtin) {
        // Replace the builtin call with a call to the msl.threadgroup_barrier intrinsic.
        auto args = Vector<core::ir::Value*, 4>{b.Constant(u32(BarrierType::kTexture))};

        auto* call = b.Call<msl::ir::BuiltinCall>(
            builtin->Result(0)->Type(), msl::BuiltinFn::kThreadgroupBarrier, std::move(args));
        call->InsertBefore(builtin);
        return call->Result(0);
    }
};

}  // namespace

Result<SuccessType> BuiltinPolyfill(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "BuiltinPolyfill transform");
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::msl::writer::raise
