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
                    case spirv::BuiltinFn::kImageGather:
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
                case spirv::BuiltinFn::kImageGather:
                    ImageGather(builtin);
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

    void ImageGather(spirv::ir::BuiltinCall* call) {
        const auto& args = call->Args();

        auto* sampled_image = args[0];

        core::ir::Value* tex = nullptr;
        core::ir::Value* sampler = nullptr;
        std::tie(tex, sampler) = GetTextureSampler(sampled_image);

        auto* coords = args[1];
        auto* component = args[2];

        b.InsertBefore(call, [&] {
            Vector<core::ir::Value*, 5> new_args;
            if (!tex->Type()->Is<core::type::DepthTexture>()) {
                new_args.Push(component);
            }
            new_args.Push(tex);
            new_args.Push(sampler);

            if (IsTextureArray(tex->Type()->As<core::type::Texture>()->Dim())) {
                auto* coords_ty = coords->Type()->As<core::type::Vector>();
                TINT_ASSERT(coords_ty);

                auto* new_coords_ty = ty.vec(coords_ty->Type(), coords_ty->Width() - 1);
                TINT_ASSERT(new_coords_ty->Width() != 4);

                // New coords
                uint32_t array_idx = 2;
                Vector<uint32_t, 3> swizzle_idx = {0, 1};
                if (new_coords_ty->Width() == 3) {
                    swizzle_idx.Push(2);
                    array_idx = 3;
                }
                new_args.Push(b.Swizzle(new_coords_ty, coords, swizzle_idx)->Result());

                // Array index
                auto* convert =
                    b.Convert(ty.i32(), b.Swizzle(new_coords_ty->Type(), coords, {array_idx}));
                new_args.Push(convert->Result());
            } else {
                new_args.Push(coords);
            }

            if (args.Length() > 4) {
                auto* offset = args[4];
                if (offset->Type()->IsUnsignedIntegerVector()) {
                    offset = b.Convert(ty.MatchWidth(ty.i32(), offset->Type()), offset)->Result();
                }
                new_args.Push(offset);
            }

            b.CallWithResult(call->DetachResult(), core::BuiltinFn::kTextureGather, new_args);
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
