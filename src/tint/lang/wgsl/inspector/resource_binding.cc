// Copyright 2021 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/lang/wgsl/inspector/resource_binding.h"

#include "src/tint/lang/core/type/array.h"
#include "src/tint/lang/core/type/f32.h"
#include "src/tint/lang/core/type/i32.h"
#include "src/tint/lang/core/type/matrix.h"
#include "src/tint/lang/core/type/texture_dimension.h"
#include "src/tint/lang/core/type/type.h"
#include "src/tint/lang/core/type/u32.h"
#include "src/tint/lang/core/type/vector.h"

namespace tint::inspector {

ResourceBinding::TextureDimension TypeTextureDimensionToResourceBindingTextureDimension(
    const type::TextureDimension& type_dim) {
    switch (type_dim) {
        case type::TextureDimension::k1d:
            return ResourceBinding::TextureDimension::k1d;
        case type::TextureDimension::k2d:
            return ResourceBinding::TextureDimension::k2d;
        case type::TextureDimension::k2dArray:
            return ResourceBinding::TextureDimension::k2dArray;
        case type::TextureDimension::k3d:
            return ResourceBinding::TextureDimension::k3d;
        case type::TextureDimension::kCube:
            return ResourceBinding::TextureDimension::kCube;
        case type::TextureDimension::kCubeArray:
            return ResourceBinding::TextureDimension::kCubeArray;
        case type::TextureDimension::kNone:
            return ResourceBinding::TextureDimension::kNone;
    }
    return ResourceBinding::TextureDimension::kNone;
}

ResourceBinding::SampledKind BaseTypeToSampledKind(const type::Type* base_type) {
    if (!base_type) {
        return ResourceBinding::SampledKind::kUnknown;
    }

    if (auto* at = base_type->As<type::Array>()) {
        base_type = at->ElemType();
    } else if (auto* mt = base_type->As<type::Matrix>()) {
        base_type = mt->type();
    } else if (auto* vt = base_type->As<type::Vector>()) {
        base_type = vt->type();
    }

    if (base_type->Is<type::F32>()) {
        return ResourceBinding::SampledKind::kFloat;
    } else if (base_type->Is<type::U32>()) {
        return ResourceBinding::SampledKind::kUInt;
    } else if (base_type->Is<type::I32>()) {
        return ResourceBinding::SampledKind::kSInt;
    } else {
        return ResourceBinding::SampledKind::kUnknown;
    }
}

ResourceBinding::TexelFormat TypeTexelFormatToResourceBindingTexelFormat(
    const core::TexelFormat& image_format) {
    switch (image_format) {
        case core::TexelFormat::kBgra8Unorm:
            return ResourceBinding::TexelFormat::kBgra8Unorm;
        case core::TexelFormat::kR32Uint:
            return ResourceBinding::TexelFormat::kR32Uint;
        case core::TexelFormat::kR32Sint:
            return ResourceBinding::TexelFormat::kR32Sint;
        case core::TexelFormat::kR32Float:
            return ResourceBinding::TexelFormat::kR32Float;
        case core::TexelFormat::kRgba8Unorm:
            return ResourceBinding::TexelFormat::kRgba8Unorm;
        case core::TexelFormat::kRgba8Snorm:
            return ResourceBinding::TexelFormat::kRgba8Snorm;
        case core::TexelFormat::kRgba8Uint:
            return ResourceBinding::TexelFormat::kRgba8Uint;
        case core::TexelFormat::kRgba8Sint:
            return ResourceBinding::TexelFormat::kRgba8Sint;
        case core::TexelFormat::kRg32Uint:
            return ResourceBinding::TexelFormat::kRg32Uint;
        case core::TexelFormat::kRg32Sint:
            return ResourceBinding::TexelFormat::kRg32Sint;
        case core::TexelFormat::kRg32Float:
            return ResourceBinding::TexelFormat::kRg32Float;
        case core::TexelFormat::kRgba16Uint:
            return ResourceBinding::TexelFormat::kRgba16Uint;
        case core::TexelFormat::kRgba16Sint:
            return ResourceBinding::TexelFormat::kRgba16Sint;
        case core::TexelFormat::kRgba16Float:
            return ResourceBinding::TexelFormat::kRgba16Float;
        case core::TexelFormat::kRgba32Uint:
            return ResourceBinding::TexelFormat::kRgba32Uint;
        case core::TexelFormat::kRgba32Sint:
            return ResourceBinding::TexelFormat::kRgba32Sint;
        case core::TexelFormat::kRgba32Float:
            return ResourceBinding::TexelFormat::kRgba32Float;
        case core::TexelFormat::kUndefined:
            return ResourceBinding::TexelFormat::kNone;
    }
    return ResourceBinding::TexelFormat::kNone;
}

}  // namespace tint::inspector
