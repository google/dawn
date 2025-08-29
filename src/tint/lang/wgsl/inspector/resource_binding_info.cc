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

#include "src/tint/lang/wgsl/inspector/resource_binding_info.h"

#include "src/tint/lang/core/type/depth_multisampled_texture.h"
#include "src/tint/lang/core/type/depth_texture.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/multisampled_texture.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/utils/rtti/switch.h"

namespace tint::inspector {

ResourceType TypeToResourceType(const core::type::Type* in_type) {
    return tint::Switch(
        in_type,
        [&](const core::type::SampledTexture* sa) {
            switch (sa->Dim()) {
                case core::type::TextureDimension::k1d:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) { return ResourceType::kTexture1d_f32; },
                        [&](const core::type::I32*) { return ResourceType::kTexture1d_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture1d_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::k2d:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) { return ResourceType::kTexture2d_f32; },
                        [&](const core::type::I32*) { return ResourceType::kTexture2d_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture2d_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::k2dArray:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) { return ResourceType::kTexture2dArray_f32; },
                        [&](const core::type::I32*) { return ResourceType::kTexture2dArray_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture2dArray_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::k3d:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) { return ResourceType::kTexture3d_f32; },
                        [&](const core::type::I32*) { return ResourceType::kTexture3d_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTexture3d_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::kCube:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) { return ResourceType::kTextureCube_f32; },
                        [&](const core::type::I32*) { return ResourceType::kTextureCube_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTextureCube_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::kCubeArray:
                    return tint::Switch(
                        sa->Type(),
                        [&](const core::type::F32*) { return ResourceType::kTextureCubeArray_f32; },
                        [&](const core::type::I32*) { return ResourceType::kTextureCubeArray_i32; },
                        [&](const core::type::U32*) { return ResourceType::kTextureCubeArray_u32; },
                        TINT_ICE_ON_NO_MATCH);
                case core::type::TextureDimension::kNone:
                    TINT_UNREACHABLE();
            }
        },
        [&](const core::type::MultisampledTexture* ms) {
            return tint::Switch(
                ms->Type(),
                [&](const core::type::F32*) { return ResourceType::kTextureMultisampled2d_f32; },
                [&](const core::type::I32*) { return ResourceType::kTextureMultisampled2d_i32; },
                [&](const core::type::U32*) { return ResourceType::kTextureMultisampled2d_u32; },
                TINT_ICE_ON_NO_MATCH);
        },
        [&](const core::type::DepthMultisampledTexture*) {
            return ResourceType::kTextureDepthMultisampled2d;
        },
        [&](const core::type::DepthTexture* de) {
            switch (de->Dim()) {
                case core::type::TextureDimension::k2d:
                    return ResourceType::kTextureDepth2d;
                case core::type::TextureDimension::k2dArray:
                    return ResourceType::kTextureDepth2dArray;
                case core::type::TextureDimension::kCube:
                    return ResourceType::kTextureDepthCube;
                case core::type::TextureDimension::kCubeArray:
                    return ResourceType::kTextureDepthCubeArray;
                default:
                    TINT_UNREACHABLE();
            }
        },
        TINT_ICE_ON_NO_MATCH);
}

}  // namespace tint::inspector
