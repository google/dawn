// Copyright 2026 The Dawn & Tint Authors
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

#include "src/tint/lang/hlsl/writer/raise/bitcast_polyfill.h"

#include <tuple>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator/validate.h"
#include "src/tint/lang/core/type/f16.h"
#include "src/tint/lang/core/type/manager.h"
#include "src/tint/lang/core/type/u16.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/u64.h"
#include "src/tint/lang/core/type/vector.h"
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

    // Polyfill functions for bitcast expression, BitcastType indicates the source type and the
    // destination type.
    using BitcastType =
        tint::UnorderedKeyWrapper<std::tuple<const core::type::Type*, const core::type::Type*>>;
    Hashmap<BitcastType, core::ir::Function*, 4> bitcast_funcs_{};

    /// Process the module.
    void Process() {
        // Find the bitcasts that need replacing.
        Vector<core::ir::CoreBuiltinCall*, 4> bitcast_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>();
                call && call->Func() == core::BuiltinFn::kBitcast) {
                bitcast_worklist.Push(call);
            }
        }

        // Replace the bitcasts that we found.
        for (auto* bitcast : bitcast_worklist) {
            auto* src_type = bitcast->Args()[0]->Type();
            auto* dst_type = bitcast->Result()->Type();
            auto* dst_deepest = dst_type->DeepestElement();
            auto* src_deepest = src_type->DeepestElement();

            if (src_type == dst_type) {
                ReplaceBitcastWithValue(bitcast);
            } else if (src_deepest->Size() == 2 && dst_deepest->Size() == 2) {
                // Same-width 16-bit bitcast (e.g. f16 <-> u16), scalar or vector.
                // Must be checked before the f16 vector cases below.
                Replace16BitBitcastWith16BitConstruct(bitcast);
            } else if (src_deepest->Size() == 2 && src_type->Is<core::type::Vector>()) {
                // 16-bit vector to 32-bit bitcast (e.g., vec2<f16> -> u32 or vec2<u16> -> u32)
                ReplaceBitcastWithFrom16BitPolyfill(bitcast);
            } else if (dst_deepest->Size() == 2 && dst_type->Is<core::type::Vector>()) {
                // 32-bit to 16-bit vector bitcast (e.g., u32 -> vec2<f16> or u32 -> vec2<u16>)
                ReplaceBitcastWithTo16BitPolyfill(bitcast);
            } else if (src_deepest->Size() == 4 && dst_deepest->Size() == 8) {
                // 32-bit vec2 to 64-bit bitcast (e.g. vec2<u32> -> u64)
                ReplaceBitcastWithTo64BitPolyfill(bitcast);
            } else {
                ReplaceBitcastWithAs(bitcast);
            }
        }
    }

    /// Replaces an identity bitcast result with the value.
    void ReplaceBitcastWithValue(core::ir::CoreBuiltinCall* bitcast) {
        bitcast->Result()->ReplaceAllUsesWith(bitcast->Args()[0]);
        bitcast->Destroy();
    }

    /// Replaces a bitcast with a 16-bit as function for f16 <-> u16 conversions (scalar and vec).
    /// Uses asuint16 (f16 -> u16) or asfloat16 (u16 -> f16).
    void Replace16BitBitcastWith16BitConstruct(core::ir::CoreBuiltinCall* bitcast) {
        auto* dst_type = bitcast->Result()->Type();
        auto* dst_deepest = dst_type->DeepestElement();

        BuiltinFn fn = BuiltinFn::kNone;
        if (dst_deepest->Is<core::type::U16>()) {
            fn = BuiltinFn::kAsuint16;
        } else if (dst_deepest->Is<core::type::F16>()) {
            fn = BuiltinFn::kAsfloat16;
        } else {
            TINT_IR_ICE(ir) << "unexpected 16-bit bitcast destination type: "
                            << dst_deepest->FriendlyName();
        }

        b.InsertBefore(bitcast, [&] {
            b.CallWithResult<hlsl::ir::BuiltinCall>(bitcast->DetachResult(), fn,
                                                    bitcast->Args()[0]);
        });
        bitcast->Destroy();
    }

    void ReplaceBitcastWithAs(core::ir::CoreBuiltinCall* bitcast) {
        auto* dst_type = bitcast->Result()->Type();
        auto* dst_deepest = dst_type->DeepestElement();

        BuiltinFn fn = BuiltinFn::kNone;
        tint::Switch(
            dst_deepest,                                                //
            [&](const core::type::I32*) { fn = BuiltinFn::kAsint; },    //
            [&](const core::type::U32*) { fn = BuiltinFn::kAsuint; },   //
            [&](const core::type::F32*) { fn = BuiltinFn::kAsfloat; },  //
            TINT_ICE_ON_NO_MATCH);

        // TODO(crbug.com/361794783): work around DXC failing on 'as' casts of constant integral
        // splats by wrapping it with an explicit vector constructor.
        // e.g. asuint(123.xx) -> asuint(int2(123.xx)))
        bool castToSrcType = false;
        auto* src_type = bitcast->Args()[0]->Type();
        if (src_type->IsIntegerVector()) {
            if (auto* c = bitcast->Args()[0]->As<core::ir::Constant>()) {
                castToSrcType = c->Value()->Is<core::constant::Splat>();
            }
        }

        b.InsertBefore(bitcast, [&] {
            auto* source = bitcast->Args()[0];
            if (castToSrcType) {
                source = b.Construct(src_type, source);
            }
            b.CallWithResult<hlsl::ir::BuiltinCall>(bitcast->DetachResult(), fn, source);
        });
        bitcast->Destroy();
    }

    // Bitcast 16-bit vector types (f16 or u16) to 32-bit types by bitcasting via u16.
    core::ir::Function* CreateBitcastFrom16Bit(const core::type::Type* src_type,
                                               const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_IR_ASSERT(ir, src_type->Is<core::type::Vector>());

                ir.properties.Add(core::ir::Property::kAllow16BitIntegers);
                auto* src_vec = src_type->As<core::type::Vector>();
                bool src_is_u16 = src_vec->Type()->Is<core::type::U16>();
                auto* u16_vec_ty = src_is_u16 ? src_type : ty.MatchWidth(ty.u16(), src_vec);

                // Generates a helper that packs a 16-bit vector into a 32-bit scalar.
                // For a vec2<u16> source, the emitted HLSL looks like:
                //
                //   uint tint_bitcast_from_u16(vector<uint16_t, 2> src) {
                //     uint2 r_uint = (uint2(src) & 65535u.xx) << uint2(0u, 16u);
                //     return r_uint.x | r_uint.y;
                //   }
                //
                // For a vec2<f16> source, asuint16 is applied first:
                //
                //   uint tint_bitcast_from_f16(vector<float16_t, 2> src) {
                //     vector<uint16_t, 2> r = asuint16(src);
                //     uint2 r_uint = (uint2(r) & 65535u.xx) << uint2(0u, 16u);
                //     return r_uint.x | r_uint.y;
                //   }

                auto fn_name =
                    b.ir.symbols.New(src_is_u16 ? "tint_bitcast_from_u16" : "tint_bitcast_from_f16")
                        .Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});

                b.Append(f->Block(), [&] {
                    // If source is f16, first reinterpret as u16 via asuint16
                    core::ir::Value* u16_src = src;
                    if (!src_is_u16) {
                        u16_src =
                            b.Call<hlsl::ir::BuiltinCall>(u16_vec_ty, BuiltinFn::kAsuint16, src)
                                ->Result();
                    }

                    auto* uint_src_ty = ty.MatchWidth(ty.u32(), src_type);
                    core::ir::Value* v = b.Convert(uint_src_ty, u16_src);

                    auto width = src_vec->Width();
                    v = b.And(v, b.Splat(uint_src_ty, 0xffff_u));
                    if (width == 2) {
                        v = b.ShiftLeft(v, b.Construct(uint_src_ty, 0_u, 16_u));
                        auto* a1 = b.Access(ty.u32(), v, 0_u);
                        v = b.Or(a1, b.Access(ty.u32(), v, 1_u));
                    } else {
                        v = b.ShiftLeft(v, b.Construct(uint_src_ty, 0_u, 16_u, 0_u, 16_u));
                        auto* a1 = b.Access(ty.u32(), v, 0_u);
                        auto* v1 = b.Or(a1, b.Access(ty.u32(), v, 1_u));
                        auto* a2 = b.Access(ty.u32(), v, 2_u);
                        auto* v2 = b.Or(a2, b.Access(ty.u32(), v, 3_u));
                        v = b.Construct(ty.vec2(ty.u32()), v1, v2);
                    }
                    if (dst_type->DeepestElement()->Is<core::type::F32>()) {
                        v = b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsfloat, v)
                                ->Result();
                    } else if (dst_type->DeepestElement()->Is<core::type::I32>()) {
                        v = b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsint, v)->Result();
                    }
                    b.Return(f, v);
                });
                return f;
            });
    }

    /// Replaces a bitcast with a call to the From16Bit polyfill for the given types
    void ReplaceBitcastWithFrom16BitPolyfill(core::ir::CoreBuiltinCall* bitcast) {
        auto* src_type = bitcast->Args()[0]->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastFrom16Bit(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    // Bitcast 32-bit types to 16-bit vector types (f16 or u16) by reinterpreting
    // their bits as u16.
    core::ir::Function* CreateBitcastTo16Bit(const core::type::Type* src_type,
                                             const core::type::Type* dst_type) {
        return bitcast_funcs_.GetOrAdd(
            BitcastType{{src_type, dst_type}}, [&]() -> core::ir::Function* {
                TINT_IR_ASSERT(ir, dst_type->Is<core::type::Vector>());

                ir.properties.Add(core::ir::Property::kAllow16BitIntegers);
                auto* dst_vec = dst_type->As<core::type::Vector>();
                bool dst_is_u16 = dst_vec->Type()->Is<core::type::U16>();
                auto* u16_vec_ty = dst_is_u16 ? dst_type : ty.MatchWidth(ty.u16(), dst_vec);

                // Generates a helper that converts via 16-bit integers.
                // For a vec2<u16> destination, the emitted HLSL looks like:
                //
                //   vector<uint16_t, 2> tint_bitcast_to_u16(float src) {
                //     uint v = asuint(src);
                //     uint2 v2 = v.xx;
                //     vector<uint16_t, 2> r =
                //       vector<uint16_t, 2>((v2 >> uint2(0u, 16u)) & (65535u).xx);
                //     return r;
                //   }
                //
                // For a vec2<f16> destination, asfloat16 is applied to the final u16 result:
                //
                //   vector<float16_t, 2> tint_bitcast_to_f16(float src) {
                //     uint v = asuint(src);
                //     uint2 v2 = v.xx;
                //     vector<uint16_t, 2> r =
                //       vector<uint16_t, 2>((v2 >> uint2(0u, 16u)) & (65535u).xx);
                //     return asfloat16(r);
                //   }

                auto fn_name =
                    b.ir.symbols.New(dst_is_u16 ? "tint_bitcast_to_u16" : "tint_bitcast_to_f16")
                        .Name();

                auto* f = b.Function(fn_name, dst_type);
                auto* src = b.FunctionParam("src", src_type);
                f->SetParams({src});
                b.Append(f->Block(), [&] {
                    const core::type::Type* uint_ty = ty.MatchWidth(ty.u32(), src_type);

                    core::ir::Value* v = nullptr;
                    tint::Switch(
                        src_type->DeepestElement(),                                      //
                        [&](const core::type::U32*) { v = b.Let("v", src)->Result(); },  //
                        [&](const core::type::I32*) {
                            v = b.Let("v", b.Call<hlsl::ir::BuiltinCall>(uint_ty,
                                                                         BuiltinFn::kAsuint, src))
                                    ->Result();
                        },
                        [&](const core::type::F32*) {
                            v = b.Let("v", b.Call<hlsl::ir::BuiltinCall>(uint_ty,
                                                                         BuiltinFn::kAsuint, src))
                                    ->Result();
                        },
                        TINT_ICE_ON_NO_MATCH);

                    bool src_vec = src_type->Is<core::type::Vector>();
                    const core::type::Type* uint_dst_ty = ty.MatchWidth(ty.u32(), dst_type);

                    // v is now the uint bitcast of src
                    core::ir::Value* shifted = nullptr;
                    core::ir::Value* masked = nullptr;
                    // Make a double wide src and then shift and mask the result to produce u16
                    // values.
                    if (src_vec) {
                        // Since the src was vec2, we swizzle so the value is
                        // equivalent to: src.xxyy
                        auto* swizzle = b.Swizzle(uint_dst_ty, v, {0_u, 0_u, 1_u, 1_u});
                        shifted =
                            b.ShiftRight(swizzle, b.Construct(uint_dst_ty, 0_u, 16_u, 0_u, 16_u));
                    } else {
                        v = b.Construct(uint_dst_ty, v, v);
                        shifted = b.ShiftRight(v, b.Construct(uint_dst_ty, 0_u, 16_u));
                    }
                    masked = b.And(shifted, b.Splat(uint_dst_ty, 0xffff_u));
                    core::ir::Instruction* v16 = b.Let("v16", b.Convert(u16_vec_ty, masked));
                    if (!dst_is_u16) {
                        v16 = b.Call<hlsl::ir::BuiltinCall>(dst_type, BuiltinFn::kAsfloat16, v16);
                    }
                    b.Return(f, v16);
                });
                return f;
            });
    }

    /// Replaces a bitcast with a call to the To16Bit polyfill for the given types
    void ReplaceBitcastWithTo16BitPolyfill(core::ir::CoreBuiltinCall* bitcast) {
        auto* src_type = bitcast->Args()[0]->Type();
        auto* dst_type = bitcast->Result()->Type();

        auto* f = CreateBitcastTo16Bit(src_type, dst_type);
        b.InsertBefore(bitcast,
                       [&] { b.CallWithResult(bitcast->DetachResult(), f, bitcast->Args()[0]); });
        bitcast->Destroy();
    }

    void ReplaceBitcastWithTo64BitPolyfill(core::ir::CoreBuiltinCall* bitcast) {
        auto* src = bitcast->Args()[0];
        auto* dst_type = bitcast->Result()->Type();
        TINT_IR_ASSERT(ir, dst_type->Is<core::type::U64>());
        TINT_IR_ASSERT(ir, src->Type()->Is<core::type::Vector>());

        b.InsertBefore(bitcast, [&] {
            // Bitcast the source to vec2<u32> to ensure we can swizzle it correctly.
            auto* low = b.Swizzle(ty.u32(), src, {0u});
            auto* high = b.Swizzle(ty.u32(), src, {1u});
            auto* low64 = b.Convert(ty.u64(), low);
            auto* high64 = b.Convert(ty.u64(), high);
            auto* shifted_high = b.ShiftLeft(high64, b.Convert(ty.u64(), b.Constant(u32(32))));
            b.OrReplaceResult(bitcast->DetachResult(), shifted_high, low64);
        });
        bitcast->Destroy();
    }
};

}  // namespace

Result<SuccessType> BitcastPolyfill(core::ir::Module& ir) {
    AssertValid(ir, "before hlsl.BitcastPolyfill");

    State{ir}.Process();

    return Success;
}

}  // namespace tint::hlsl::writer::raise
