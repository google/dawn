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

#include "src/tint/lang/spirv/writer/raise/resource_binding.h"

#include "src/tint/lang/core/type/pointer.h"
#include "src/tint/lang/core/type/sampled_texture.h"
#include "src/tint/lang/spirv/type/resource_binding.h"

using namespace tint::core::fluent_types;     // NOLINT
using namespace tint::core::number_suffixes;  // NOLINT

namespace tint::spirv::writer::raise {
namespace {

const core::type::Type* ResourceTypeToType(core::type::Manager& ty, ResourceType type) {
    switch (type) {
        case ResourceType::kTexture1d_f32:
            return ty.sampled_texture(core::type::TextureDimension::k1d, ty.f32());
        case ResourceType::kTexture1d_i32:
            return ty.sampled_texture(core::type::TextureDimension::k1d, ty.i32());
        case ResourceType::kTexture1d_u32:
            return ty.sampled_texture(core::type::TextureDimension::k1d, ty.u32());
        case ResourceType::kTexture2d_f32:
            return ty.sampled_texture(core::type::TextureDimension::k2d, ty.f32());
        case ResourceType::kTexture2d_i32:
            return ty.sampled_texture(core::type::TextureDimension::k2d, ty.i32());
        case ResourceType::kTexture2d_u32:
            return ty.sampled_texture(core::type::TextureDimension::k2d, ty.u32());
        case ResourceType::kTexture2dArray_f32:
            return ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.f32());
        case ResourceType::kTexture2dArray_i32:
            return ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.i32());
        case ResourceType::kTexture2dArray_u32:
            return ty.sampled_texture(core::type::TextureDimension::k2dArray, ty.u32());
        case ResourceType::kTexture3d_f32:
            return ty.sampled_texture(core::type::TextureDimension::k3d, ty.f32());
        case ResourceType::kTexture3d_i32:
            return ty.sampled_texture(core::type::TextureDimension::k3d, ty.i32());
        case ResourceType::kTexture3d_u32:
            return ty.sampled_texture(core::type::TextureDimension::k3d, ty.u32());
        case ResourceType::kTextureCube_f32:
            return ty.sampled_texture(core::type::TextureDimension::kCube, ty.f32());
        case ResourceType::kTextureCube_i32:
            return ty.sampled_texture(core::type::TextureDimension::kCube, ty.i32());
        case ResourceType::kTextureCube_u32:
            return ty.sampled_texture(core::type::TextureDimension::kCube, ty.u32());
        case ResourceType::kTextureCubeArray_f32:
            return ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.f32());
        case ResourceType::kTextureCubeArray_i32:
            return ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.i32());
        case ResourceType::kTextureCubeArray_u32:
            return ty.sampled_texture(core::type::TextureDimension::kCubeArray, ty.u32());

        case ResourceType::kTextureMultisampled2d_f32:
            return ty.multisampled_texture(core::type::TextureDimension::k2d, ty.f32());
        case ResourceType::kTextureMultisampled2d_i32:
            return ty.multisampled_texture(core::type::TextureDimension::k2d, ty.i32());
        case ResourceType::kTextureMultisampled2d_u32:
            return ty.multisampled_texture(core::type::TextureDimension::k2d, ty.u32());

        case ResourceType::kTextureDepth2d:
            return ty.depth_texture(core::type::TextureDimension::k2d);
        case ResourceType::kTextureDepth2dArray:
            return ty.depth_texture(core::type::TextureDimension::k2dArray);
        case ResourceType::kTextureDepthCube:
            return ty.depth_texture(core::type::TextureDimension::kCube);
        case ResourceType::kTextureDepthCubeArray:
            return ty.depth_texture(core::type::TextureDimension::kCubeArray);
        case ResourceType::kTextureDepthMultisampled2d:
            return ty.depth_multisampled_texture(core::type::TextureDimension::k2d);

        default:
            TINT_UNREACHABLE();
    }
}

}  // namespace

// Returns a map of types to the var which is used to access the memory of that type
Hashmap<const core::type::Type*, core::ir::Var*, 4> ResourceBindingHelper::GenerateAliases(
    core::ir::Builder& b,
    core::ir::Var* var,
    const std::vector<ResourceType>& types) const {
    Hashmap<const core::type::Type*, core::ir::Var*, 4> res;

    auto* ptr = var->Result()->Type()->As<core::type::Pointer>();
    TINT_ASSERT(ptr);

    for (auto& type : types) {
        auto* t = ResourceTypeToType(b.ir.Types(), type);
        b.InsertBefore(var, [&] {
            auto* spv_ty = b.ir.Types().Get<spirv::type::ResourceBinding>(t);
            auto* v = b.Var(b.ir.Types().ptr(handle, spv_ty, ptr->Access()));
            res.Add(t, v);
        });
    }

    return res;
}

}  // namespace tint::spirv::writer::raise
