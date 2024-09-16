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

#include "src/tint/lang/glsl/writer/raise/builtin_polyfill.h"

#include <string>
#include <tuple>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/glsl/builtin_fn.h"
#include "src/tint/lang/glsl/ir/builtin_call.h"
#include "src/tint/lang/glsl/ir/ternary.h"

namespace tint::glsl::writer::raise {
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

    // Polyfill functions for bitcast expression, BitcastType indicates the source type and the
    // destination type.
    using BitcastType =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::Type*, const core::type::Type*>>;
    Hashmap<BitcastType, core::ir::Function*, 4> bitcast_funcs_{};

    /// Process the module.
    void Process() {
        Vector<core::ir::CoreBuiltinCall*, 4> call_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kSelect:
                    case core::BuiltinFn::kStorageBarrier:
                    case core::BuiltinFn::kTextureBarrier:
                    case core::BuiltinFn::kWorkgroupBarrier:
                        call_worklist.Push(call);
                        break;
                    default:
                        break;
                }
                continue;
            }
        }

        // Replace the builtin calls that we found
        for (auto* call : call_worklist) {
            switch (call->Func()) {
                case core::BuiltinFn::kSelect:
                    Select(call);
                    break;
                case core::BuiltinFn::kStorageBarrier:
                case core::BuiltinFn::kTextureBarrier:
                case core::BuiltinFn::kWorkgroupBarrier:
                    Barrier(call);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
        }
    }

    void Barrier(core::ir::CoreBuiltinCall* call) {
        b.InsertBefore(call, [&] {
            b.Call<glsl::ir::BuiltinCall>(ty.void_(), glsl::BuiltinFn::kBarrier);

            switch (call->Func()) {
                case core::BuiltinFn::kStorageBarrier:
                    b.Call<glsl::ir::BuiltinCall>(ty.void_(),
                                                  glsl::BuiltinFn::kMemoryBarrierBuffer);
                    break;
                case core::BuiltinFn::kTextureBarrier:
                    b.Call<glsl::ir::BuiltinCall>(ty.void_(), glsl::BuiltinFn::kMemoryBarrierImage);
                    break;
                default:
                    break;
            }
        });

        call->Destroy();
    }

    void Select(core::ir::CoreBuiltinCall* call) {
        Vector<core::ir::Value*, 4> args = call->Args();

        // GLSL does not support ternary expressions with a bool vector conditional,
        // so polyfill by manually creating a vector with each of the
        // individual scalar ternaries.
        if (auto* vec = call->Result(0)->Type()->As<core::type::Vector>()) {
            Vector<core::ir::Value*, 4> construct_args;

            b.InsertBefore(call, [&] {
                auto* elm_ty = vec->Type();
                for (uint32_t i = 0; i < vec->Width(); i++) {
                    auto* false_ = b.Swizzle(elm_ty, args[0], {i})->Result(0);
                    auto* true_ = b.Swizzle(elm_ty, args[1], {i})->Result(0);
                    auto* cond = b.Swizzle(elm_ty, args[2], {i})->Result(0);

                    auto* ternary = b.ir.CreateInstruction<glsl::ir::Ternary>(
                        b.InstructionResult(elm_ty),
                        Vector<core::ir::Value*, 3>{false_, true_, cond});
                    ternary->InsertBefore(call);

                    construct_args.Push(ternary->Result(0));
                }

                b.ConstructWithResult(call->DetachResult(), construct_args);
            });

        } else {
            auto* ternary = b.ir.CreateInstruction<glsl::ir::Ternary>(call->DetachResult(), args);
            ternary->InsertBefore(call);
        }
        call->Destroy();
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

}  // namespace tint::glsl::writer::raise
