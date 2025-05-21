// Copyright 2025 The Dawn & Tint Authors
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

#include "src/tint/lang/spirv/reader/lower/texture.h"

#include <optional>
#include <tuple>
#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
#include "src/tint/lang/spirv/builtin_fn.h"
#include "src/tint/lang/spirv/ir/builtin_call.h"
#include "src/tint/lang/spirv/type/image.h"

namespace tint::spirv::reader::lower {
namespace {

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

enum class ImageOperandsMask : uint32_t {
    kBias = 0x00000001,
    kLod = 0x00000002,
    kGrad = 0x00000004,
    kConstOffset = 0x00000008,
    kSample = 0x00000040,
};

/// PIMPL state for the transform.
struct State {
    /// The IR module.
    core::ir::Module& ir;

    /// The IR builder.
    core::ir::Builder b{ir};

    /// The type manager.
    core::type::Manager& ty{ir.Types()};

    /// Map of all OpSampledImages seen
    Hashmap<core::ir::Value*, core::ir::Instruction*, 4> sampled_images_{};

    /// Process the module.
    void Process() {
        for (auto* inst : *ir.root_block) {
            auto* var = inst->As<core::ir::Var>();
            if (!var) {
                continue;
            }

            auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
            TINT_ASSERT(ptr);

            auto* type = ptr->UnwrapPtr();
            if (!type->Is<spirv::type::Image>()) {
                continue;
            }

            auto* new_ty = TypeFor(type);
            var->Result()->SetType(ty.ptr(ptr->AddressSpace(), new_ty, ptr->Access()));

            // TODO(dsinclair): Propagate through functions

            for (auto& usage : var->Result()->UsagesUnsorted()) {
                if (usage->instruction->Is<core::ir::Load>()) {
                    usage->instruction->Result()->SetType(new_ty);
                }
            }
        }

        // TODO(dsinclair): Propagate OpTypeSampledImage through function params by replacing with
        // the texture/sampler

        Vector<spirv::ir::BuiltinCall*, 4> builtin_worklist;
        for (auto* inst : ir.Instructions()) {
            if (auto* builtin = inst->As<spirv::ir::BuiltinCall>()) {
                switch (builtin->Func()) {
                    case spirv::BuiltinFn::kSampledImage:
                    case spirv::BuiltinFn::kImageFetch:
                    case spirv::BuiltinFn::kImageGather:
                    case spirv::BuiltinFn::kImageQueryLevels:
                    case spirv::BuiltinFn::kImageQuerySamples:
                    case spirv::BuiltinFn::kImageQuerySize:
                    case spirv::BuiltinFn::kImageQuerySizeLod:
                    case spirv::BuiltinFn::kImageSampleExplicitLod:
                    case spirv::BuiltinFn::kImageSampleImplicitLod:
                    case spirv::BuiltinFn::kImageSampleProjImplicitLod:
                    case spirv::BuiltinFn::kImageSampleProjExplicitLod:
                    case spirv::BuiltinFn::kImageWrite:
                        builtin_worklist.Push(builtin);
                        break;
                    default:
                        TINT_UNREACHABLE() << "unknown spirv builtin: " << builtin->Func();
                }
            }
        }

        for (auto* builtin : builtin_worklist) {
            switch (builtin->Func()) {
                case spirv::BuiltinFn::kSampledImage:
                    SampledImage(builtin);
                    break;
                case spirv::BuiltinFn::kImageFetch:
                    ImageFetch(builtin);
                    break;
                case spirv::BuiltinFn::kImageGather:
                    ImageGather(builtin);
                    break;
                case spirv::BuiltinFn::kImageQueryLevels:
                    ImageQuery(builtin, core::BuiltinFn::kTextureNumLevels);
                    break;
                case spirv::BuiltinFn::kImageQuerySamples:
                    ImageQuery(builtin, core::BuiltinFn::kTextureNumSamples);
                    break;
                case spirv::BuiltinFn::kImageQuerySize:
                case spirv::BuiltinFn::kImageQuerySizeLod:
                    ImageQuerySize(builtin);
                    break;
                case spirv::BuiltinFn::kImageSampleExplicitLod:
                case spirv::BuiltinFn::kImageSampleImplicitLod:
                case spirv::BuiltinFn::kImageSampleProjImplicitLod:
                case spirv::BuiltinFn::kImageSampleProjExplicitLod:
                    ImageSample(builtin);
                    break;
                case spirv::BuiltinFn::kImageWrite:
                    ImageWrite(builtin);
                    break;
                default:
                    break;
            }
        }

        // Destroy all the OpSampledImage instructions.
        for (auto res : sampled_images_) {
            res.value->Destroy();
        }
    }

    // Record the sampled image so we can extract the texture/sampler information as we process the
    // builtins. It will be destroyed after all builtins are done.
    void SampledImage(spirv::ir::BuiltinCall* call) { sampled_images_.Add(call->Result(), call); }

    std::pair<core::ir::Value*, core::ir::Value*> GetTextureSampler(core::ir::Value* sampled) {
        auto res = sampled_images_.Get(sampled);
        TINT_ASSERT(res);

        core::ir::Instruction* inst = *res;
        TINT_ASSERT(inst->Operands().Length() == 2);

        return {inst->Operands()[0], inst->Operands()[1]};
    }

    void ProcessCoords(const core::type::Type* type,
                       bool is_proj,
                       core::ir::Value* coords,
                       Vector<core::ir::Value*, 5>& new_args) {
        if (!is_proj && !IsTextureArray(type->As<core::type::Texture>()->Dim())) {
            new_args.Push(coords);
            return;
        }

        auto* coords_ty = coords->Type()->As<core::type::Vector>();
        TINT_ASSERT(coords_ty);

        uint32_t new_coords_width = coords_ty->Width() - 1;
        auto* new_coords_ty = ty.MatchWidth(coords_ty->Type(), new_coords_width);

        Vector<uint32_t, 3> swizzle_idx = {0};
        if (new_coords_width >= 2) {
            swizzle_idx.Push(1);
        }
        if (new_coords_width == 3) {
            swizzle_idx.Push(2);
        }
        auto* swizzle = b.Swizzle(new_coords_ty, coords, swizzle_idx);
        core::ir::Value* last =
            b.Swizzle(coords_ty->Type(), coords, Vector{new_coords_width})->Result();

        if (is_proj) {
            // New coords
            // Divide the coordinates by the last value to simulate the
            // projection behaviour.
            new_args.Push(b.Divide(new_coords_ty, swizzle, last)->Result());
        } else {
            TINT_ASSERT(new_coords_ty->Is<core::type::Vector>());

            // New coords
            new_args.Push(swizzle->Result());
            // Array index
            if (!last->Type()->Is<core::type::I32>()) {
                last = b.Convert(ty.i32(), last)->Result();
            }
            new_args.Push(last);
        }
    }

    void ProcessOffset(core::ir::Value* offset, Vector<core::ir::Value*, 5>& new_args) {
        if (offset->Type()->IsUnsignedIntegerVector()) {
            offset = b.Convert(ty.MatchWidth(ty.i32(), offset->Type()), offset)->Result();
        }
        new_args.Push(offset);
    }

    uint32_t GetOperandMask(core::ir::Value* val) {
        auto* op = val->As<core::ir::Constant>();
        TINT_ASSERT(op);
        return op->Value()->ValueAs<uint32_t>();
    }

    bool HasBias(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kBias)) != 0;
    }
    bool HasGrad(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kGrad)) != 0;
    }
    bool HasLod(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kLod)) != 0;
    }
    bool HasConstOffset(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kConstOffset)) != 0;
    }
    bool HasSample(uint32_t mask) {
        return (mask & static_cast<uint32_t>(ImageOperandsMask::kSample)) != 0;
    }

    void ImageFetch(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];
            auto* coords = args[1];

            auto* tex_ty = tex->Type();
            uint32_t operand_mask = GetOperandMask(args[2]);

            Vector<core::ir::Value*, 5> new_args = {tex};
            ProcessCoords(tex->Type(), false, coords, new_args);

            uint32_t idx = 3;
            if (HasLod(operand_mask)) {
                core::ir::Value* lod = args[idx++];

                if (!lod->Type()->Is<core::type::I32>()) {
                    lod = b.Convert(ty.i32(), lod)->Result();
                }
                new_args.Push(lod);
            } else if (!tex_ty->IsAnyOf<core::type::DepthMultisampledTexture,
                                        core::type::MultisampledTexture,
                                        core::type::StorageTexture>()) {
                // textureLoad requires an explicit level-of-detail parameter for non-multisampled
                // and non-storage texture types.
                new_args.Push(b.Zero(ty.i32()));
            }
            if (HasSample(operand_mask)) {
                core::ir::Value* sample = args[idx++];

                if (!sample->Type()->Is<core::type::I32>()) {
                    sample = b.Convert(ty.i32(), sample)->Result();
                }
                new_args.Push(sample);
            }

            // Depth textures have a single value return in WGSL, but a vec4 in SPIR-V.
            auto* call_ty = call->Result()->Type();
            if (tex_ty->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>()) {
                call_ty = call_ty->DeepestElement();
            }
            auto* res = b.Call(call_ty, core::BuiltinFn::kTextureLoad, new_args)->Result();

            // Restore the vec4 result by padding with 0's.
            if (call_ty != call->Result()->Type()) {
                auto* vec = call->Result()->Type()->As<core::type::Vector>();
                TINT_ASSERT(vec && vec->Width() == 4);

                auto* z = b.Zero(call_ty);
                res = b.Construct(call->Result()->Type(), res, z, z, z)->Result();
            }
            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    void ImageGather(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        b.InsertBefore(call, [&] {
            core::ir::Value* tex = nullptr;
            core::ir::Value* sampler = nullptr;
            std::tie(tex, sampler) = GetTextureSampler(args[0]);

            auto* coords = args[1];
            auto* component = args[2];

            uint32_t operand_mask = GetOperandMask(args[3]);

            Vector<core::ir::Value*, 5> new_args;
            if (!tex->Type()->Is<core::type::DepthTexture>()) {
                new_args.Push(component);
            }
            new_args.Push(tex);
            new_args.Push(sampler);

            ProcessCoords(tex->Type(), false, coords, new_args);

            if (HasConstOffset(operand_mask)) {
                ProcessOffset(args[4], new_args);
            }

            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kTextureGather, new_args);
        });
        call->Destroy();
    }

    void ImageSample(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        auto* sampled_image = args[0];

        core::ir::Value* tex = nullptr;
        core::ir::Value* sampler = nullptr;
        std::tie(tex, sampler) = GetTextureSampler(sampled_image);

        auto* tex_ty = tex->Type();

        auto* coords = args[1];
        uint32_t operand_mask = GetOperandMask(args[2]);

        bool is_proj = call->Func() == spirv::BuiltinFn::kImageSampleProjImplicitLod ||
                       call->Func() == spirv::BuiltinFn::kImageSampleProjExplicitLod;

        uint32_t idx = 3;
        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 5> new_args;
            new_args.Push(tex);
            new_args.Push(sampler);

            ProcessCoords(tex_ty, is_proj, coords, new_args);

            core::BuiltinFn fn = core::BuiltinFn::kTextureSample;
            if (HasBias(operand_mask)) {
                fn = core::BuiltinFn::kTextureSampleBias;
                new_args.Push(args[idx++]);
            }
            if (HasLod(operand_mask)) {
                fn = core::BuiltinFn::kTextureSampleLevel;

                core::ir::Value* lod = args[idx++];

                // Depth texture LOD in WGSL is i32/u32 but f32 in SPIR-V.
                // Convert to i32
                if (tex_ty->Is<core::type::DepthTexture>()) {
                    lod = b.Convert(ty.i32(), lod)->Result();
                }
                new_args.Push(lod);
            }
            if (HasGrad(operand_mask)) {
                fn = core::BuiltinFn::kTextureSampleGrad;
                new_args.Push(args[idx++]);  // ddx
                new_args.Push(args[idx++]);  // ddy
            }
            if (HasConstOffset(operand_mask)) {
                ProcessOffset(args[idx++], new_args);
            }

            // Depth textures have a single value return in WGSL, but a vec4 in SPIR-V.
            auto* call_ty = call->Result()->Type();
            if (tex_ty->IsAnyOf<core::type::DepthTexture, core::type::DepthMultisampledTexture>()) {
                call_ty = call_ty->DeepestElement();
            }
            auto* res = b.Call(call_ty, fn, new_args)->Result();

            // Restore the vec4 result by padding with 0's.
            if (call_ty != call->Result()->Type()) {
                auto* vec = call->Result()->Type()->As<core::type::Vector>();
                TINT_ASSERT(vec && vec->Width() == 4);

                auto* z = b.Zero(call_ty);
                res = b.Construct(call->Result()->Type(), res, z, z, z)->Result();
            }

            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    void ImageWrite(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        b.InsertBefore(call, [&] {
            core::ir::Value* tex = args[0];
            auto* coords = args[1];
            auto* texel = args[2];

            Vector<core::ir::Value*, 5> new_args;
            new_args.Push(tex);

            ProcessCoords(tex->Type(), false, coords, new_args);

            new_args.Push(texel);

            b.Call(call->Result()->Type(), core::BuiltinFn::kTextureStore, new_args);
        });
        call->Destroy();
    }

    void ImageQuery(spirv::ir::BuiltinCall* call, core::BuiltinFn fn) {
        auto* image = call->Args()[0];

        b.InsertBefore(call, [&] {
            auto* type = call->Result()->Type();

            // WGSL requires a `u32` result component where SPIR-V allows `i32` or `u32`
            core::ir::Value* res =
                b.Call(ty.MatchWidth(ty.u32(), type), fn, Vector{image})->Result();
            if (type->IsSignedIntegerScalarOrVector()) {
                res = b.Convert(type, res)->Result();
            }

            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    void ImageQuerySize(spirv::ir::BuiltinCall* call) {
        auto* image = call->Args()[0];

        auto* tex_ty = image->Type()->As<core::type::Texture>();
        TINT_ASSERT(tex_ty);

        b.InsertBefore(call, [&] {
            auto* type = call->Result()->Type();

            // WGSL requires a `u32` result component where SPIR-V allows `i32` or `u32`
            auto* wgsl_type = ty.MatchWidth(ty.u32(), type);

            // A SPIR-V OpImageQuery will return the array `element` entry with
            // the image query. In WGSL, these are two calls, the
            // `textureDimensions` will get the width,height,depth but we also
            // have to call `textureNumLayers` to get the elements and then
            // re-inject that back into the result.
            if (core::type::IsTextureArray(tex_ty->Dim())) {
                wgsl_type = ty.vec(ty.u32(), wgsl_type->As<core::type::Vector>()->Width() - 1);
            }

            Vector<core::ir::Value*, 2> args = {image};
            if (call->Func() == spirv::BuiltinFn::kImageQuerySizeLod) {
                args.Push(call->Args()[1]);
            }

            core::ir::Value* res =
                b.Call(wgsl_type, core::BuiltinFn::kTextureDimensions, args)->Result();

            if (core::type::IsTextureArray(tex_ty->Dim())) {
                core::ir::Value* layers =
                    b.Call(ty.u32(), core::BuiltinFn::kTextureNumLayers, image)->Result();
                res = b.Construct(ty.MatchWidth(ty.u32(), type), res, layers)->Result();
            }

            if (type->IsSignedIntegerScalarOrVector()) {
                res = b.Convert(type, res)->Result();
            }

            call->Result()->ReplaceAllUsesWith(res);
        });
        call->Destroy();
    }

    const core::type::Type* TypeFor(const core::type::Type* src_ty) {
        if (auto* img = src_ty->As<spirv::type::Image>()) {
            return TypeForImage(img);
        }
        return src_ty;
    }

    core::type::TextureDimension ConvertDim(spirv::type::Dim dim, spirv::type::Arrayed arrayed) {
        switch (dim) {
            case spirv::type::Dim::kD1:
                return core::type::TextureDimension::k1d;
            case spirv::type::Dim::kD2:
                return arrayed == spirv::type::Arrayed::kArrayed
                           ? core::type::TextureDimension::k2dArray
                           : core::type::TextureDimension::k2d;
            case spirv::type::Dim::kD3:
                return core::type::TextureDimension::k3d;
            case spirv::type::Dim::kCube:
                return arrayed == spirv::type::Arrayed::kArrayed
                           ? core::type::TextureDimension::kCubeArray
                           : core::type::TextureDimension::kCube;
            default:
                TINT_UNREACHABLE();
        }
    }

    const core::type::Type* TypeForImage(const spirv::type::Image* img) {
        if (img->GetDim() == spirv::type::Dim::kSubpassData) {
            return ty.input_attachment(img->GetSampledType());
        }

        if (img->GetSampled() == spirv::type::Sampled::kReadWriteOpCompatible) {
            return ty.storage_texture(ConvertDim(img->GetDim(), img->GetArrayed()),
                                      img->GetTexelFormat(), img->GetAccess());
        }

        // TODO(dsinclair): Handle determining depth texture by usage
        if (img->GetDepth() == spirv::type::Depth::kDepth) {
            if (img->GetMultisampled() == spirv::type::Multisampled::kMultisampled) {
                return ty.depth_multisampled_texture(ConvertDim(img->GetDim(), img->GetArrayed()));
            }
            return ty.depth_texture(ConvertDim(img->GetDim(), img->GetArrayed()));
        }

        if (img->GetMultisampled() == spirv::type::Multisampled::kMultisampled) {
            return ty.multisampled_texture(ConvertDim(img->GetDim(), img->GetArrayed()),
                                           img->GetSampledType());
        }

        return ty.sampled_texture(ConvertDim(img->GetDim(), img->GetArrayed()),
                                  img->GetSampledType());
    }
};

}  // namespace

Result<SuccessType> Texture(core::ir::Module& ir) {
    auto result = ValidateAndDumpIfNeeded(ir, "spirv.Texture",
                                          core::ir::Capabilities{
                                              core::ir::Capability::kAllowOverrides,
                                          });
    if (result != Success) {
        return result.Failure();
    }

    State{ir}.Process();

    return Success;
}

}  // namespace tint::spirv::reader::lower
