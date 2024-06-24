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

#include "src/tint/lang/hlsl/writer/raise/builtin_polyfill.h"

#include <string>
#include <tuple>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/hlsl/builtin_fn.h"
#include "src/tint/lang/hlsl/ir/builtin_call.h"
#include "src/tint/utils/containers/hashmap.h"
#include "src/tint/utils/math/hash.h"

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

    using BinaryType =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::Type*, const core::type::Type*>>;

    // Polyfill functions for bitcast expression, BinaryType indicates the source type and the
    // destination type.
    Hashmap<BinaryType, core::ir::Function*, 4> bitcast_funcs_{};

    /// Process the module.
    void Process() {
        // Find the bitcasts that need replacing.
        Vector<core::ir::Bitcast*, 4> bitcast_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* bitcast = inst->As<core::ir::Bitcast>()) {
                bitcast_worklist.Push(bitcast);
            }
        }

        // Replace the bitcasts that we found.
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
            } else {
                ReplaceBitcastWithAs(bitcast);
            }
        }
    }

    /// Replaces an identity bitcast result with the value.
    void ReplaceBitcastWithValue(core::ir::Bitcast* bitcast) {
        bitcast->Result(0)->ReplaceAllUsesWith(bitcast->Val());
        bitcast->Destroy();
    }

    void ReplaceBitcastWithAs(core::ir::Bitcast* bitcast) {
        auto* dst_type = bitcast->Result(0)->Type();
        auto* dst_deepest = dst_type->DeepestElement();

        BuiltinFn fn = BuiltinFn::kNone;
        tint::Switch(
            dst_deepest,                                                //
            [&](const core::type::I32*) { fn = BuiltinFn::kAsint; },    //
            [&](const core::type::U32*) { fn = BuiltinFn::kAsuint; },   //
            [&](const core::type::F32*) { fn = BuiltinFn::kAsfloat; },  //
            TINT_ICE_ON_NO_MATCH);

        b.InsertBefore(bitcast, [&] {
            b.CallWithResult<hlsl::ir::BuiltinCall>(bitcast->DetachResult(), fn, bitcast->Val());
        });
        bitcast->Destroy();
    }

    // Bitcast f16 types to others by converting the given f16 value to f32 and call
    // f32tof16 to get the bits. This should be safe, because the conversion is precise
    // for finite and infinite f16 value as they are exactly representable by f32.
    core::ir::Function* CreateBitcastFromF16(const core::type::Type* src_type,
                                             const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BinaryType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(src_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in HLSL):
                //
                // uint tint_bitcast_from_f16(vector<float16_t, 2> src) {
                //   uint2 r = f32tof16(float2(src));
                //   return uint((r.x & 65535u) | ((r.y & 65535u) << 16u));
                // }

                auto fn_name = b.ir.symbols.New(std::string("tint_bitcast_from_f16")).Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});

                b.Append(f->Block(), [&] {
                    auto* src_vec = src_type->As<core::type::Vector>();

                    auto* cast = b.Convert(ty.vec(ty.f32(), src_vec->Width()), src);
                    auto* r =
                        b.Let("r", b.Call<hlsl::ir::BuiltinCall>(ty.vec(ty.u32(), src_vec->Width()),
                                                                 hlsl::BuiltinFn::kF32Tof16, cast));

                    auto* x = b.And(ty.u32(), b.Swizzle(ty.u32(), r, {0_u}), 0xffff_u);
                    auto* y = b.ShiftLeft(
                        ty.u32(), b.And(ty.u32(), b.Swizzle(ty.u32(), r, {1_u}), 0xffff_u), 16_u);

                    auto* s = b.Or(ty.u32(), x, y);
                    core::ir::InstructionResult* result = nullptr;

                    switch (src_vec->Width()) {
                        case 2: {
                            result = s->Result(0);
                            break;
                        }
                        case 4: {
                            auto* z = b.And(ty.u32(), b.Swizzle(ty.u32(), r, {2_u}), 0xffff_u);
                            auto* w = b.ShiftLeft(
                                ty.u32(), b.And(ty.u32(), b.Swizzle(ty.u32(), r, {3_u}), 0xffff_u),
                                16_u);

                            auto* t = b.Or(ty.u32(), z, w);
                            auto* cons = b.Construct(ty.vec2<u32>(), s, t);
                            result = cons->Result(0);
                            break;
                        }
                        default:
                            TINT_UNREACHABLE();
                    }

                    tint::Switch(
                        dst_type->DeepestElement(),  //
                        [&](const core::type::F32*) {
                            b.Return(f, b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsfloat,
                                                                      result));
                        },
                        [&](const core::type::I32*) {
                            b.Return(f, b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsint,
                                                                      result));
                        },
                        [&](const core::type::U32*) { b.Return(f, result); },  //
                        TINT_ICE_ON_NO_MATCH);
                });
                return f;
            });
    }

    /// Replaces a bitcast with a call to the FromF16 polyfill for the given types
    void ReplaceBitcastWithFromF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result(0)->Type();

        auto* f = CreateBitcastFromF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    // Bitcast other types to f16 types by reinterpreting their bits as f16 using
    // f16tof32, and convert the result f32 to f16. This should be safe, because the
    // conversion is precise for finite and infinite f16 result value as they are
    // exactly representable by f32.
    core::ir::Function* CreateBitcastToF16(const core::type::Type* src_type,
                                           const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BinaryType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_ASSERT(dst_type->Is<core::type::Vector>());

                // Generate a helper function that performs the following (in HLSL):
                //
                // vector<float16_t, 2> tint_bitcast_to_f16(float src) {
                //   uint v = asuint(src);
                //   float t_low = f16tof32(v & 65535u);
                //   float t_high = f16tof32((v >> 16u) & 65535u);
                //   return vector<float16_t, 2>(t_low.x, t_high.x);
                // }

                auto fn_name = b.ir.symbols.New(std::string("tint_bitcast_to_f16")).Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});
                b.Append(f->Block(), [&] {
                    const core::type::Type* uint_ty = nullptr;
                    const core::type::Type* float_ty = nullptr;

                    auto* src_vec = src_type->As<core::type::Vector>();
                    if (src_vec) {
                        uint_ty = ty.vec(ty.u32(), src_vec->Width());
                        float_ty = ty.vec(ty.f32(), src_vec->Width());
                    } else {
                        uint_ty = ty.u32();
                        float_ty = ty.f32();
                    }

                    core::ir::Instruction* v = nullptr;
                    tint::Switch(
                        src_type->DeepestElement(),                            //
                        [&](const core::type::U32*) { v = b.Let("v", src); },  //
                        [&](const core::type::I32*) {
                            v = b.Let("v", b.Call<hlsl::ir::BuiltinCall>(uint_ty,
                                                                         BuiltinFn::kAsuint, src));
                        },
                        [&](const core::type::F32*) {
                            v = b.Let("v", b.Call<hlsl::ir::BuiltinCall>(uint_ty,
                                                                         BuiltinFn::kAsuint, src));
                        },
                        TINT_ICE_ON_NO_MATCH);

                    core::ir::Value* mask = nullptr;
                    core::ir::Value* shift = nullptr;
                    if (src_vec) {
                        mask = b.Let("mask", b.Splat(uint_ty, 0xffff_u))->Result(0);
                        shift = b.Let("shift", b.Splat(uint_ty, 16_u))->Result(0);
                    } else {
                        mask = b.Value(b.Constant(0xffff_u));
                        shift = b.Value(b.Constant(16_u));
                    }

                    auto* l = b.And(uint_ty, v, mask);
                    auto* t_low = b.Let(
                        "t_low", b.Call<hlsl::ir::BuiltinCall>(float_ty, BuiltinFn::kF16Tof32, l));

                    auto* h = b.And(uint_ty, b.ShiftRight(uint_ty, v, shift), mask);
                    auto* t_high = b.Let(
                        "t_high", b.Call<hlsl::ir::BuiltinCall>(float_ty, BuiltinFn::kF16Tof32, h));

                    auto* x = b.Swizzle(ty.f16(), t_low, {0_u});
                    auto* y = b.Swizzle(ty.f16(), t_high, {0_u});
                    if (dst_type->As<core::type::Vector>()->Width() == 2) {
                        b.Return(f, b.Construct(dst_type, x, y));
                    } else {
                        auto* z = b.Swizzle(ty.f16(), t_low, {1_u});
                        auto* w = b.Swizzle(ty.f16(), t_high, {1_u});
                        b.Return(f, b.Construct(dst_type, x, y, z, w));
                    }
                });
                return f;
            });
    }

    /// Replaces a bitcast with a call to the ToF16 polyfill for the given types
    void ReplaceBitcastWithToF16Polyfill(core::ir::Bitcast* bitcast) {
        auto* src_type = bitcast->Val()->Type();
        auto* dst_type = bitcast->Result(0)->Type();

        auto* f = CreateBitcastToF16(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
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

}  // namespace tint::hlsl::writer::raise
