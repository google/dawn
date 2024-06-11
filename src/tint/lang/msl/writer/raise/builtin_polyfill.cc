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

#include <atomic>
#include <utility>

#include "src/tint/lang/core/fluent_types.h"
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/constant.h"
#include "src/tint/lang/core/ir/core_builtin_call.h"
#include "src/tint/lang/core/ir/function.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/msl/barrier_type.h"
#include "src/tint/lang/msl/builtin_fn.h"
#include "src/tint/lang/msl/ir/builtin_call.h"
#include "src/tint/lang/msl/ir/member_builtin_call.h"
#include "src/tint/lang/msl/ir/memory_order.h"
#include "src/tint/utils/containers/hashmap.h"

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

    /// A map from an atomic pointer type to an atomicCompareExchangeWeak polyfill.
    Hashmap<const core::type::Type*, core::ir::Function*, 2> atomic_compare_exchange_polyfills{};

    /// Process the module.
    void Process() {
        // Find the builtins that need replacing.
        Vector<core::ir::CoreBuiltinCall*, 4> worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (builtin->Func()) {
                    case core::BuiltinFn::kAtomicAdd:
                    case core::BuiltinFn::kAtomicAnd:
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    case core::BuiltinFn::kAtomicExchange:
                    case core::BuiltinFn::kAtomicLoad:
                    case core::BuiltinFn::kAtomicMax:
                    case core::BuiltinFn::kAtomicMin:
                    case core::BuiltinFn::kAtomicOr:
                    case core::BuiltinFn::kAtomicStore:
                    case core::BuiltinFn::kAtomicSub:
                    case core::BuiltinFn::kAtomicXor:
                    case core::BuiltinFn::kTextureSample:
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
            switch (builtin->Func()) {
                // Atomics.
                case core::BuiltinFn::kAtomicAdd:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchAddExplicit);
                    break;
                case core::BuiltinFn::kAtomicAnd:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchAndExplicit);
                    break;
                case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    AtomicCompareExchangeWeak(builtin);
                    break;
                case core::BuiltinFn::kAtomicExchange:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicExchangeExplicit);
                    break;
                case core::BuiltinFn::kAtomicLoad:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicLoadExplicit);
                    break;
                case core::BuiltinFn::kAtomicMax:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchMaxExplicit);
                    break;
                case core::BuiltinFn::kAtomicMin:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchMinExplicit);
                    break;
                case core::BuiltinFn::kAtomicOr:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchOrExplicit);
                    break;
                case core::BuiltinFn::kAtomicStore:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicStoreExplicit);
                    break;
                case core::BuiltinFn::kAtomicSub:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchSubExplicit);
                    break;
                case core::BuiltinFn::kAtomicXor:
                    AtomicCall(builtin, msl::BuiltinFn::kAtomicFetchXorExplicit);
                    break;

                // Texture builtins.
                case core::BuiltinFn::kTextureSample:
                    TextureSample(builtin);
                    break;

                // Barriers.
                case core::BuiltinFn::kStorageBarrier:
                    ThreadgroupBarrier(builtin, BarrierType::kDevice);
                    break;
                case core::BuiltinFn::kWorkgroupBarrier:
                    ThreadgroupBarrier(builtin, BarrierType::kThreadGroup);
                    break;
                case core::BuiltinFn::kTextureBarrier:
                    ThreadgroupBarrier(builtin, BarrierType::kTexture);
                    break;

                default:
                    break;
            }
        }
    }

    /// Replace an atomic builtin call with an equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void AtomicCall(core::ir::CoreBuiltinCall* builtin, msl::BuiltinFn intrinsic) {
        auto args = Vector<core::ir::Value*, 4>{builtin->Args()};
        args.Push(ir.allocators.values.Create<msl::ir::MemoryOrder>(
            b.ConstantValue(u32(std::memory_order_relaxed))));
        auto* call = b.CallWithResult<msl::ir::BuiltinCall>(builtin->DetachResult(), intrinsic,
                                                            std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Replace an atomicCompareExchangeWeak builtin call with an equivalent MSL polyfill.
    /// @param builtin the builtin call instruction
    void AtomicCompareExchangeWeak(core::ir::CoreBuiltinCall* builtin) {
        // Get or generate a polyfill function.
        auto* atomic_ptr = builtin->Args()[0]->Type();
        auto* polyfill = atomic_compare_exchange_polyfills.GetOrAdd(atomic_ptr, [&] {
            // The polyfill function performs the equivalent to the following:
            //     int old_value = cmp;
            //     bool exchanged = atomic_compare_exchange_weak_explicit(
            //                         atomic_ptr, old_value, val,
            //                         memory_order_relaxed, memory_order_relaxed);
            //     return __atomic_compare_exchange_result_i32(old_value, exchanged);
            auto* ptr = b.FunctionParam("atomic_ptr", atomic_ptr);
            auto* cmp = b.FunctionParam("cmp", builtin->Args()[1]->Type());
            auto* val = b.FunctionParam("val", builtin->Args()[2]->Type());
            auto* func = b.Function(builtin->Result(0)->Type());
            func->SetParams({ptr, cmp, val});
            b.Append(func->Block(), [&] {
                auto* old_value = b.Var<function>("old_value", cmp)->Result(0);
                auto* order = ir.allocators.values.Create<msl::ir::MemoryOrder>(
                    b.ConstantValue(u32(std::memory_order_relaxed)));
                auto* call = b.Call<msl::ir::BuiltinCall>(
                    ty.bool_(), BuiltinFn::kAtomicCompareExchangeWeakExplicit,
                    Vector{ptr, old_value, val, order, order});
                auto* result =
                    b.Construct(builtin->Result(0)->Type(), Vector{
                                                                b.Load(old_value)->Result(0),
                                                                call->Result(0),
                                                            });
                b.Return(func, result);
            });
            return func;
        });

        // Call the polyfill function.
        auto args = Vector<core::ir::Value*, 4>{builtin->Args()};
        auto* call = b.CallWithResult(builtin->DetachResult(), polyfill, std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Replace a textureSample call with the equivalent MSL intrinsic.
    /// @param builtin the builtin call instruction
    void TextureSample(core::ir::CoreBuiltinCall* builtin) {
        // The MSL intrinsic is a member function, so we split the first argument off as the object.
        auto args = Vector<core::ir::Value*, 4>(builtin->Args().Offset(1));
        auto* call = b.MemberCallWithResult<msl::ir::MemberBuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kSample, builtin->Args()[0], std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
    }

    /// Replace a barrier builtin with the `threadgroupBarrier()` intrinsic.
    /// @param builtin the builtin call instruction
    /// @param type the barrier type
    void ThreadgroupBarrier(core::ir::CoreBuiltinCall* builtin, BarrierType type) {
        // Replace the builtin call with a call to the msl.threadgroup_barrier intrinsic.
        auto args = Vector<core::ir::Value*, 1>{b.Constant(u32(type))};
        auto* call = b.CallWithResult<msl::ir::BuiltinCall>(
            builtin->DetachResult(), msl::BuiltinFn::kThreadgroupBarrier, std::move(args));
        call->InsertBefore(builtin);
        builtin->Destroy();
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
