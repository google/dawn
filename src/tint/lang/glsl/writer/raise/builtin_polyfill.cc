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
#include <utility>

#include "src/tint/lang/core/fluent_types.h"  // IWYU pragma: export
#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/glsl/builtin_fn.h"
#include "src/tint/lang/glsl/ir/builtin_call.h"
#include "src/tint/lang/glsl/ir/member_builtin_call.h"
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

    /// Process the module.
    void Process() {
        Vector<core::ir::CoreBuiltinCall*, 4> call_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kArrayLength:
                    case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    case core::BuiltinFn::kAtomicSub:
                    case core::BuiltinFn::kAtomicLoad:
                    case core::BuiltinFn::kCountOneBits:
                    case core::BuiltinFn::kExtractBits:
                    case core::BuiltinFn::kFma:
                    case core::BuiltinFn::kInsertBits:
                    case core::BuiltinFn::kSelect:
                    case core::BuiltinFn::kStorageBarrier:
                    case core::BuiltinFn::kTextureBarrier:
                    case core::BuiltinFn::kTextureDimensions:
                    case core::BuiltinFn::kTextureLoad:
                    case core::BuiltinFn::kTextureNumLayers:
                    case core::BuiltinFn::kTextureStore:
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
                case core::BuiltinFn::kArrayLength:
                    ArrayLength(call);
                    break;
                case core::BuiltinFn::kAtomicCompareExchangeWeak:
                    AtomicCompareExchangeWeak(call);
                    break;
                case core::BuiltinFn::kAtomicSub:
                    AtomicSub(call);
                    break;
                case core::BuiltinFn::kAtomicLoad:
                    AtomicLoad(call);
                    break;
                case core::BuiltinFn::kCountOneBits:
                    CountOneBits(call);
                    break;
                case core::BuiltinFn::kExtractBits:
                    ExtractBits(call);
                    break;
                case core::BuiltinFn::kFma:
                    FMA(call);
                    break;
                case core::BuiltinFn::kInsertBits:
                    InsertBits(call);
                    break;
                case core::BuiltinFn::kSelect:
                    Select(call);
                    break;
                case core::BuiltinFn::kStorageBarrier:
                case core::BuiltinFn::kTextureBarrier:
                case core::BuiltinFn::kWorkgroupBarrier:
                    Barrier(call);
                    break;
                case core::BuiltinFn::kTextureDimensions:
                    TextureDimensions(call);
                    break;
                case core::BuiltinFn::kTextureLoad:
                    TextureLoad(call);
                    break;
                case core::BuiltinFn::kTextureNumLayers:
                    TextureNumLayers(call);
                    break;
                case core::BuiltinFn::kTextureStore:
                    TextureStore(call);
                    break;
                default:
                    TINT_UNREACHABLE();
            }
        }
    }

    void ArrayLength(core::ir::Call* call) {
        b.InsertBefore(call, [&] {
            auto* len = b.MemberCall<glsl::ir::MemberBuiltinCall>(ty.i32(), BuiltinFn::kLength,
                                                                  call->Args()[0]);
            b.ConvertWithResult(call->DetachResult(), len->Result(0));
        });
        call->Destroy();
    }

    void ExtractBits(core::ir::Call* call) {
        b.InsertBefore(call, [&] {
            auto args = call->Args();
            auto* offset = b.Convert(ty.i32(), args[1]);
            auto* bits = b.Convert(ty.i32(), args[2]);

            b.CallWithResult<glsl::ir::BuiltinCall>(
                call->DetachResult(), glsl::BuiltinFn::kBitfieldExtract, args[0], offset, bits);
        });
        call->Destroy();
    }

    void InsertBits(core::ir::Call* call) {
        b.InsertBefore(call, [&] {
            auto args = call->Args();
            auto* offset = b.Convert(ty.i32(), args[2]);
            auto* bits = b.Convert(ty.i32(), args[3]);

            b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(),
                                                    glsl::BuiltinFn::kBitfieldInsert, args[0],
                                                    args[1], offset, bits);
        });
        call->Destroy();
    }

    // There is no `fma` method in GLSL ES 3.10 so we emulate it. `fma` does exist in desktop after
    // 4.00 but we use the emulated version to be consistent. We could use the real one on desktop
    // if we decide too in the future.
    void FMA(core::ir::Call* call) {
        auto args = call->Args();

        b.InsertBefore(call, [&] {
            auto* res_ty = call->Result(0)->Type();
            auto* mul = b.Multiply(res_ty, args[0], args[1]);
            b.AddWithResult(call->DetachResult(), mul, args[2]);
        });
        call->Destroy();
    }

    // GLSL `bitCount` always returns an `i32` so we need to convert it. Convert to a `bitCount`
    // call to make it clear this isn't `countOneBits`.
    void CountOneBits(core::ir::Call* call) {
        auto* result_ty = call->Result(0)->Type();

        b.InsertBefore(call, [&] {
            auto* c = b.Call<glsl::ir::BuiltinCall>(ty.MatchWidth(ty.i32(), result_ty),
                                                    glsl::BuiltinFn::kBitCount, call->Args()[0]);
            b.ConvertWithResult(call->DetachResult(), c);
        });
        call->Destroy();
    }

    // `textureDimensions` returns an unsigned scalar / vector in WGSL. `textureSize` and
    // `imageSize` return a signed scalar / vector in GLSL.  So, we  need to cast the result to
    // the needed WGSL type.
    void TextureDimensions(core::ir::BuiltinCall* call) {
        auto args = call->Args();
        auto* tex = args[0]->Type()->As<core::type::Texture>();

        b.InsertBefore(call, [&] {
            auto func = glsl::BuiltinFn::kTextureSize;
            if (tex->Is<core::type::StorageTexture>()) {
                func = glsl::BuiltinFn::kImageSize;
            }

            Vector<core::ir::Value*, 2> new_args;
            new_args.Push(args[0]);

            if (!(tex->Is<core::type::StorageTexture>() ||
                  tex->Is<core::type::MultisampledTexture>() ||
                  tex->Is<core::type::DepthMultisampledTexture>())) {
                // Add a LOD to any texture other then storage, and multi-sampled textures which
                // does not already have an LOD.
                if (args.Length() == 1) {
                    new_args.Push(b.Constant(0_i));
                } else {
                    // Make sure the LOD is a i32
                    new_args.Push(b.Bitcast(ty.i32(), args[1])->Result(0));
                }
            }

            auto ret_type = call->Result(0)->Type();

            // In GLSL the array dimensions return a 3rd parameter.
            if (tex->Dim() == core::type::TextureDimension::k2dArray ||
                tex->Dim() == core::type::TextureDimension::kCubeArray) {
                ret_type = ty.vec(ty.i32(), 3);
            } else {
                ret_type = ty.MatchWidth(ty.i32(), call->Result(0)->Type());
            }

            core::ir::Value* result =
                b.Call<glsl::ir::BuiltinCall>(ret_type, func, new_args)->Result(0);

            // `textureSize` on array samplers returns the array size in the final component, WGSL
            // requires a 2 component response, so drop the array size
            if (tex->Dim() == core::type::TextureDimension::k2dArray ||
                tex->Dim() == core::type::TextureDimension::kCubeArray) {
                ret_type = ty.MatchWidth(ty.i32(), call->Result(0)->Type());
                result = b.Swizzle(ret_type, result, {0, 1})->Result(0);
            }

            b.BitcastWithResult(call->DetachResult(), result);
        });
        call->Destroy();
    }

    // `textureNumLayers` returns an unsigned scalar in WGSL. `textureSize` and `imageSize`
    // return a signed scalar / vector in GLSL.
    //
    // For the `textureSize` and `imageSize` calls the valid WGSL values always produce a `vec3` in
    // GLSL so we extract the `z` component for the number of layers.
    void TextureNumLayers(core::ir::BuiltinCall* call) {
        b.InsertBefore(call, [&] {
            auto args = call->Args();
            auto* tex = args[0]->Type()->As<core::type::Texture>();

            auto func = glsl::BuiltinFn::kTextureSize;
            if (tex->Is<core::type::StorageTexture>()) {
                func = glsl::BuiltinFn::kImageSize;
            }

            Vector<core::ir::Value*, 2> new_args;
            new_args.Push(args[0]);

            // Non-storage textures require a LOD
            if (!tex->Is<core::type::StorageTexture>()) {
                new_args.Push(b.Constant(0_i));
            }

            auto* new_call = b.Call<glsl::ir::BuiltinCall>(ty.vec(ty.i32(), 3), func, new_args);

            auto* swizzle = b.Swizzle(ty.i32(), new_call, {2});
            b.BitcastWithResult(call->DetachResult(), swizzle->Result(0));
        });
        call->Destroy();
    }

    void TextureLoad(core::ir::CoreBuiltinCall* call) {
        auto args = call->Args();
        auto* tex = args[0];

        // No loading from a depth texture in GLSL, so we should never have gotten here.
        TINT_ASSERT(!tex->Type()->Is<core::type::DepthTexture>());

        auto* tex_type = tex->Type()->As<core::type::Texture>();

        glsl::BuiltinFn func = glsl::BuiltinFn::kNone;
        if (tex_type->Is<core::type::StorageTexture>()) {
            func = glsl::BuiltinFn::kImageLoad;
        } else {
            func = glsl::BuiltinFn::kTexelFetch;
        }

        bool is_ms = tex_type->Is<core::type::MultisampledTexture>();
        bool is_storage = tex_type->Is<core::type::StorageTexture>();
        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 3> call_args{tex};
            switch (tex_type->Dim()) {
                case core::type::TextureDimension::k1d: {
                    call_args.Push(b.Convert(ty.i32(), args[1])->Result(0));
                    if (!is_storage) {
                        call_args.Push(b.Convert(ty.i32(), args[2])->Result(0));
                    }
                    break;
                }
                case core::type::TextureDimension::k2d: {
                    call_args.Push(b.Convert(ty.vec2<i32>(), args[1])->Result(0));
                    if (is_ms) {
                        call_args.Push(b.Convert(ty.i32(), args[2])->Result(0));
                    } else {
                        if (!is_storage) {
                            call_args.Push(b.Convert(ty.i32(), args[2])->Result(0));
                        }
                    }
                    break;
                }
                case core::type::TextureDimension::k2dArray: {
                    auto* coord = b.Convert(ty.vec2<i32>(), args[1]);
                    auto* ary_idx = b.Convert(ty.i32(), args[2]);
                    call_args.Push(b.Construct(ty.vec3<i32>(), coord, ary_idx)->Result(0));

                    if (!is_storage) {
                        call_args.Push(b.Convert(ty.i32(), args[3])->Result(0));
                    }
                    break;
                }
                case core::type::TextureDimension::k3d: {
                    call_args.Push(b.Convert(ty.vec3<i32>(), args[1])->Result(0));

                    if (!is_storage) {
                        call_args.Push(b.Convert(ty.i32(), args[2])->Result(0));
                    }
                    break;
                }
                default:
                    TINT_UNREACHABLE();
            }

            b.CallWithResult<glsl::ir::BuiltinCall>(call->DetachResult(), func,
                                                    std::move(call_args));
        });
        call->Destroy();
    }

    void TextureStore(core::ir::BuiltinCall* call) {
        auto args = call->Args();
        auto* tex = args[0];
        auto* tex_type = tex->Type()->As<core::type::StorageTexture>();
        TINT_ASSERT(tex_type);

        Vector<core::ir::Value*, 3> new_args;
        new_args.Push(tex);

        b.InsertBefore(call, [&] {
            if (tex_type->Dim() == core::type::TextureDimension::k2dArray) {
                auto* coords = args[1];
                auto* array_idx = args[2];

                auto* coords_ty = coords->Type()->As<core::type::Vector>();
                TINT_ASSERT(coords_ty);

                auto* new_coords = b.Construct(ty.vec3(coords_ty->Type()), coords,
                                               b.Convert(coords_ty->Type(), array_idx));
                new_args.Push(new_coords->Result(0));

                new_args.Push(args[3]);
            } else {
                new_args.Push(args[1]);
                new_args.Push(args[2]);
            }

            b.CallWithResult<glsl::ir::BuiltinCall>(
                call->DetachResult(), glsl::BuiltinFn::kImageStore, std::move(new_args));
        });
        call->Destroy();
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
