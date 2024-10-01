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

#include "src/tint/lang/glsl/writer/raise/texture_polyfill.h"

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
#include "src/tint/lang/glsl/ir/builtin_call.h"
#include "src/tint/lang/glsl/ir/member_builtin_call.h"

namespace tint::glsl::writer::raise {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The configuration
    const TexturePolyfillConfig& cfg;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Process the module.
    void Process() {
        /// Converts all 1D texture types and accesses to 2D. This is required for GLSL ES, which
        /// does not support 1D textures. We do this for desktop GL as well for consistency. This
        /// could be relaxed in the future if desired.
        UpgradeTexture1DVars();
        UpgradeTexture1DParams();

        Vector<core::ir::CoreBuiltinCall*, 4> call_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* call = inst->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kTextureDimensions:
                    case core::BuiltinFn::kTextureLoad:
                    case core::BuiltinFn::kTextureNumLayers:
                    case core::BuiltinFn::kTextureStore:
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

    std::optional<const core::type::Type*> UpgradeTexture1D(core::ir::Value* value) {
        bool is_1d = false;
        const core::type::Type* new_type = nullptr;
        tint::Switch(
            value->Type()->UnwrapPtr(),
            [&](const core::type::SampledTexture* s) {
                is_1d = s->Dim() == core::type::TextureDimension::k1d;
                new_type = ty.Get<core::type::SampledTexture>(core::type::TextureDimension::k2d,
                                                              s->Type());
            },
            [&](const core::type::StorageTexture* s) {
                is_1d = s->Dim() == core::type::TextureDimension::k1d;
                new_type = ty.Get<core::type::StorageTexture>(
                    core::type::TextureDimension::k2d, s->TexelFormat(), s->Access(), s->Type());
            });
        if (!is_1d) {
            return std::nullopt;
        }

        if (auto* ptr = value->Type()->As<core::type::Pointer>()) {
            new_type = ty.ptr(ptr->AddressSpace(), new_type, ptr->Access());
        }

        // For each 1d texture usage we have to make sure return values and arguments are modified
        // to fit the 2d texture.
        for (auto usage : value->UsagesUnsorted()) {
            if (auto* call = usage->instruction->As<core::ir::CoreBuiltinCall>()) {
                switch (call->Func()) {
                    case core::BuiltinFn::kTextureDimensions: {
                        // Upgrade result to a vec2 and swizzle out the `x` component.
                        auto* res = call->DetachResult();
                        call->SetResults(b.InstructionResult(ty.vec2<u32>()));

                        b.InsertAfter(call, [&] {
                            auto* s = b.Swizzle(res->Type(), call, Vector<uint32_t, 1>{0});
                            res->ReplaceAllUsesWith(s->Result(0));
                        });
                        break;
                    }
                    case core::BuiltinFn::kTextureLoad:
                    case core::BuiltinFn::kTextureStore: {
                        // Add a new coord item so it's a vec2.
                        auto arg = call->Args()[1];
                        b.InsertBefore(call, [&] {
                            call->SetArg(1,
                                         b.Construct(ty.vec2(arg->Type()), arg, b.Zero(arg->Type()))
                                             ->Result(0));
                        });
                        break;
                    }
                    case core::BuiltinFn::kTextureSample: {
                        // Add a new coord item so it's a vec2.
                        auto arg = call->Args()[1];
                        b.InsertBefore(call, [&] {
                            call->SetArg(1,
                                         b.Construct(ty.vec2(arg->Type()), arg, 0.5_f)->Result(0));
                        });
                        break;
                    }
                    case core::BuiltinFn::kTextureNumLevels:
                        // No changes needed.
                        break;
                    default:
                        TINT_UNREACHABLE() << "unknown usage instruction for texture";
                }
            }
        }

        return {new_type};
    }

    void UpgradeTexture1DVars() {
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }
            auto new_type = UpgradeTexture1D(var->Result(0));
            if (!new_type.has_value()) {
                continue;
            }
            var->Result(0)->SetType(new_type.value());

            // All of the usages of the textures should involve loading them as the `var`
            // declarations will be pointers and the function usages require non-pointer textures.
            for (auto usage : var->Result(0)->UsagesUnsorted()) {
                UpgradeLoadOf1DTexture(usage->instruction);
            }
        }
    }

    void UpgradeTexture1DParams() {
        for (auto func : ir.functions) {
            for (auto* param : func->Params()) {
                auto new_type = UpgradeTexture1D(param);
                if (!new_type.has_value()) {
                    continue;
                }
                param->SetType(new_type.value());
            }
        }
    }

    void UpgradeLoadOf1DTexture(core::ir::Instruction* inst) {
        auto* ld = inst->As<core::ir::Load>();
        TINT_ASSERT(ld);

        auto new_type = UpgradeTexture1D(ld->Result(0));
        if (!new_type.has_value()) {
            return;
        }
        ld->Result(0)->SetType(new_type.value());
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

            b.BitcastWithResult(call->DetachResult(), result)->Result(0);
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
};

}  // namespace

Result<SuccessType> TexturePolyfill(core::ir::Module& ir, const TexturePolyfillConfig& cfg) {
    auto result = ValidateAndDumpIfNeeded(ir, "glsl.TexturePolyfill transform");
    if (result != Success) {
        return result.Failure();
    }

    State{ir, cfg}.Process();

    return Success;
}

}  // namespace tint::glsl::writer::raise
