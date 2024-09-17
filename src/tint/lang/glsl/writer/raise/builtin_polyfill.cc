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

    // The bitcast worklist is a member because some polyfills add bitcast calls. When they do, they
    // can add the bitcast to the worklist to be fixed up as needed.
    Vector<core::ir::Bitcast*, 4> bitcast_worklist{};

    /// Process the module.
    void Process() {
        Vector<core::ir::CoreBuiltinCall*, 4> call_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* bitcast = inst->As<core::ir::Bitcast>()) {
                bitcast_worklist.Push(bitcast);
                continue;
            }

            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    case core::BuiltinFn::kAtomicSub:
                    case core::BuiltinFn::kAtomicLoad:
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
                case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    AtomicCompareExchangeWeak(call);
                    break;
                case core::BuiltinFn::kAtomicSub:
                    AtomicSub(call);
                    break;
                case core::BuiltinFn::kAtomicLoad:
                    AtomicLoad(call);
                    break;
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

        // Replace the bitcasts that we found. These are done after the other builtins as some of
        // them also create bitcasts which will need to be updated.
        for (auto* bitcast : bitcast_worklist) {
            auto* src_type = bitcast->Val()->Type();
            auto* dst_type = bitcast->Result(0)->Type();
            auto* dst_deepest = dst_type->DeepestElement();

            if (src_type == dst_type) {
                ReplaceBitcastWithValue(bitcast);
            } else if (src_type->DeepestElement()->Is<core::type::F16>()) {
                ReplaceBitcastWithFromF16Polyfill(bitcast);
            } else if (dst_deepest->Is<core::type::F16>()) {
                ReplaceBitcastWithToF16Polyfill(bitcast);
            } else if (src_type->DeepestElement()->Is<core::type::F32>()) {
                ReplaceBitcastFromF32(bitcast);
            } else if (dst_type->DeepestElement()->Is<core::type::F32>()) {
                ReplaceBitcastToF32(bitcast);
            } else {
                ReplaceBitcast(bitcast);
            }
        }
    }

    void ReplaceBitcastWithValue(core::ir::Bitcast* bitcast) {
        bitcast->Result(0)->ReplaceAllUsesWith(bitcast->Val());
        bitcast->Destroy();
    }

    core::ir::Value* CreateBitcastFromF32(const core::type::Type* type,
                                          const core::type::Type* result_type,
                                          core::ir::Value* val) {
        BuiltinFn fn = BuiltinFn::kNone;
        tint::Switch(
            type,                                                               //
            [&](const core::type::I32*) { fn = BuiltinFn::kFloatBitsToInt; },   //
            [&](const core::type::U32*) { fn = BuiltinFn::kFloatBitsToUint; },  //
            TINT_ICE_ON_NO_MATCH);

        return b.Call<glsl::ir::BuiltinCall>(result_type, fn, val)->Result(0);
    }

    void ReplaceBitcastFromF32(core::ir::Bitcast* bitcast) {
        auto* dst_type = bitcast->Result(0)->Type();
        auto* dst_deepest = dst_type->DeepestElement();

        b.InsertBefore(bitcast, [&] {
            auto* bc =
                CreateBitcastFromF32(dst_deepest, bitcast->Result(0)->Type(), bitcast->Val());
            bitcast->Result(0)->ReplaceAllUsesWith(bc);
        });
        bitcast->Destroy();
    }

    core::ir::Value* CreateBitcastToF32(const core::type::Type* src_type,
                                        const core::type::Type* dst_type,
                                        core::ir::Value* val) {
        BuiltinFn fn = BuiltinFn::kNone;
        tint::Switch(
            src_type,                                                           //
            [&](const core::type::I32*) { fn = BuiltinFn::kIntBitsToFloat; },   //
            [&](const core::type::U32*) { fn = BuiltinFn::kUintBitsToFloat; },  //
            TINT_ICE_ON_NO_MATCH);

        return b.Call<glsl::ir::BuiltinCall>(dst_type, fn, val)->Result(0);
    }

    void ReplaceBitcastToF32(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* src_deepest = src_type->DeepestElement();

        b.InsertBefore(bitcast, [&] {
            auto* bc = CreateBitcastToF32(src_deepest, bitcast->Result(0)->Type(), bitcast->Val());
            bitcast->Result(0)->ReplaceAllUsesWith(bc);
        });
        bitcast->Destroy();
    }

    void ReplaceBitcast(core::ir::Bitcast* bitcast) {
        b.InsertBefore(bitcast,
                       [&] { b.ConvertWithResult(bitcast->DetachResult(), bitcast->Val()); });
        bitcast->Destroy();
    }

    core::ir::Function* CreateBitcastFromF16(const core::type::Type* src_type,
                                             const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(src_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in GLSL):
                //
                // ivec2 tint_bitcast_from_f16(f16vec4 src) {
                //   uvec2 r = uvec2(packFloat2x16(src.xy), packFloat2x16(src.zw));
                //   return ivec2(r);
                // }

                auto fn_name = b.ir.symbols.New("tint_bitcast_from_f16").Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});

                b.Append(f->Block(), [&] {
                    auto* src_vec = src_type->As<core::type::Vector>();

                    core::ir::Value* packed = nullptr;
                    if (src_vec->Width() == 2) {
                        packed = b.Call<glsl::ir::BuiltinCall>(ty.u32(),
                                                               glsl::BuiltinFn::kPackFloat2X16, src)
                                     ->Result(0);
                    } else if (src_vec->Width() == 4) {
                        auto* left =
                            b.Call<glsl::ir::BuiltinCall>(ty.u32(), glsl::BuiltinFn::kPackFloat2X16,
                                                          b.Swizzle(ty.vec2<f16>(), src, {0, 1}));
                        auto* right =
                            b.Call<glsl::ir::BuiltinCall>(ty.u32(), glsl::BuiltinFn::kPackFloat2X16,
                                                          b.Swizzle(ty.vec2<f16>(), src, {2, 3}));
                        packed = b.Construct(ty.vec2<u32>(), left, right)->Result(0);
                    } else {
                        TINT_UNREACHABLE();
                    }

                    if (dst_type->DeepestElement()->Is<core::type::F32>()) {
                        packed =
                            CreateBitcastToF32(packed->Type()->DeepestElement(), dst_type, packed);
                    } else {
                        packed = b.Convert(dst_type, packed)->Result(0);
                    }

                    b.Return(f, packed);
                });
                return f;
            });
    }

    void ReplaceBitcastWithFromF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result(0)->Type();

        auto* f = CreateBitcastFromF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    core::ir::Function* CreateBitcastToF16(const core::type::Type* src_type,
                                           const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(dst_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in GLSL):
                //
                // f16vec4 tint_bitcast_to_f16(ivec2 src) {
                //   uvec2 r = uvec2(src);
                //   f16vec2 v_xy = unpackFloat2x16(r.x);
                //   f16vec2 v_zw = unpackFloat2x16(r.y);
                //   return f16vec4(v_xy.x, v_xy.y, v_zw.x, v_zw.y);
                // }

                auto fn_name = b.ir.symbols.New("tint_bitcast_to_f16").Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});
                b.Append(f->Block(), [&] {
                    core::ir::Value* conv = nullptr;

                    if (src->Type()->DeepestElement()->Is<core::type::F32>()) {
                        conv =
                            CreateBitcastFromF32(ty.u32(), ty.MatchWidth(ty.u32(), src_type), src);
                    } else {
                        conv = b.Convert(ty.MatchWidth(ty.u32(), src->Type()), src)->Result(0);
                    }

                    core::ir::Value* val = nullptr;
                    if (src->Type()->Is<core::type::Vector>()) {
                        auto* left = b.Call<glsl::ir::BuiltinCall>(
                            ty.vec2<f16>(), glsl::BuiltinFn::kUnpackFloat2X16,
                            b.Swizzle(ty.u32(), conv, {0}));
                        auto* right = b.Call<glsl::ir::BuiltinCall>(
                            ty.vec2<f16>(), glsl::BuiltinFn::kUnpackFloat2X16,
                            b.Swizzle(ty.u32(), conv, {1}));

                        val = b.Construct(dst_type, left, right)->Result(0);
                    } else {
                        val = b.Call<glsl::ir::BuiltinCall>(ty.vec2<f16>(),
                                                            glsl::BuiltinFn::kUnpackFloat2X16, conv)
                                  ->Result(0);
                    }
                    b.Return(f, val);
                });
                return f;
            });
    }

    void ReplaceBitcastWithToF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result(0)->Type();

        auto* f = CreateBitcastToF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    void AtomicCompareExchangeWeak(core::ir::BuiltinCall* call) {
        auto args = call->Args();
        auto* type = args[1]->Type();

        auto* dest = args[0];
        auto* compare_value = args[1];
        auto* value = args[2];

        auto* result_type = call->Result(0)->Type();

        b.InsertBefore(call, [&] {
            auto* bitcast_cmp_value = b.Bitcast(type, compare_value);
            auto* bitcast_value = b.Bitcast(type, value);

            bitcast_worklist.Push(bitcast_cmp_value);
            bitcast_worklist.Push(bitcast_value);

            auto* swap = b.Call<glsl::ir::BuiltinCall>(
                type, glsl::BuiltinFn::kAtomicCompSwap,
                Vector<core::ir::Value*, 3>{dest, bitcast_cmp_value->Result(0),
                                            bitcast_value->Result(0)});

            auto* exchanged = b.Equal(ty.bool_(), swap, compare_value);

            auto* result = b.Construct(result_type, swap, exchanged)->Result(0);
            call->Result(0)->ReplaceAllUsesWith(result);
        });
        call->Destroy();
    }

    void AtomicSub(core::ir::BuiltinCall* call) {
        b.InsertBefore(call, [&] {
            auto args = call->Args();

            if (args[1]->Type()->Is<core::type::I32>()) {
                b.CallWithResult(call->DetachResult(), core::BuiltinFn::kAtomicAdd, args[0],
                                 b.Negation(args[1]->Type(), args[1]));
            } else {
                // Negating a u32 isn't possible in the IR, so pass a fake GLSL function and
                // handle in the printer.
                b.CallWithResult<glsl::ir::BuiltinCall>(
                    call->DetachResult(), glsl::BuiltinFn::kAtomicSub,
                    Vector<core::ir::Value*, 2>{args[0], args[1]});
            }
        });
        call->Destroy();
    }

    void AtomicLoad(core::ir::CoreBuiltinCall* call) {
        // GLSL does not have an atomicLoad, so we emulate it with atomicOr using 0 as the OR
        // value
        b.InsertBefore(call, [&] {
            auto args = call->Args();
            b.CallWithResult(
                call->DetachResult(), core::BuiltinFn::kAtomicOr, args[0],
                b.Zero(args[0]->Type()->UnwrapPtr()->As<core::type::Atomic>()->Type()));
        });
        call->Destroy();
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
