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

#include <utility>

#include "src/tint/lang/core/ir/builder.h"
#include "src/tint/lang/core/ir/module.h"
#include "src/tint/lang/core/ir/validator.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/storage_texture.h"
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

            // TOOD(dsinclair): Replace var usages
        }
    }

    const core::type::Type* TypeFor(const core::type::Type* src_ty) {
        TINT_ASSERT(src_ty->Is<spirv::type::Image>());

        if (auto* img = src_ty->As<spirv::type::Image>()) {
            return TypeForImage(img);
        }

        TINT_UNREACHABLE();
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
            return ty.depth_texture(ConvertDim(img->GetDim(), img->GetArrayed()));
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
