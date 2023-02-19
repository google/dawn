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

#include "src/tint/inspector/resource_binding.h"

#include "src/tint/type/array.h"
#include "src/tint/type/f32.h"
#include "src/tint/type/i32.h"
#include "src/tint/type/matrix.h"
#include "src/tint/type/texture_dimension.h"
#include "src/tint/type/type.h"
#include "src/tint/type/u32.h"
#include "src/tint/type/vector.h"

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
    const builtin::TexelFormat& image_format) {
    switch (image_format) {
        case builtin::TexelFormat::kBgra8Unorm:
            return ResourceBinding::TexelFormat::kBgra8Unorm;
        case builtin::TexelFormat::kR32Uint:
            return ResourceBinding::TexelFormat::kR32Uint;
        case builtin::TexelFormat::kR32Sint:
            return ResourceBinding::TexelFormat::kR32Sint;
        case builtin::TexelFormat::kR32Float:
            return ResourceBinding::TexelFormat::kR32Float;
        case builtin::TexelFormat::kRgba8Unorm:
            return ResourceBinding::TexelFormat::kRgba8Unorm;
        case builtin::TexelFormat::kRgba8Snorm:
            return ResourceBinding::TexelFormat::kRgba8Snorm;
        case builtin::TexelFormat::kRgba8Uint:
            return ResourceBinding::TexelFormat::kRgba8Uint;
        case builtin::TexelFormat::kRgba8Sint:
            return ResourceBinding::TexelFormat::kRgba8Sint;
        case builtin::TexelFormat::kRg32Uint:
            return ResourceBinding::TexelFormat::kRg32Uint;
        case builtin::TexelFormat::kRg32Sint:
            return ResourceBinding::TexelFormat::kRg32Sint;
        case builtin::TexelFormat::kRg32Float:
            return ResourceBinding::TexelFormat::kRg32Float;
        case builtin::TexelFormat::kRgba16Uint:
            return ResourceBinding::TexelFormat::kRgba16Uint;
        case builtin::TexelFormat::kRgba16Sint:
            return ResourceBinding::TexelFormat::kRgba16Sint;
        case builtin::TexelFormat::kRgba16Float:
            return ResourceBinding::TexelFormat::kRgba16Float;
        case builtin::TexelFormat::kRgba32Uint:
            return ResourceBinding::TexelFormat::kRgba32Uint;
        case builtin::TexelFormat::kRgba32Sint:
            return ResourceBinding::TexelFormat::kRgba32Sint;
        case builtin::TexelFormat::kRgba32Float:
            return ResourceBinding::TexelFormat::kRgba32Float;
        case builtin::TexelFormat::kUndefined:
            return ResourceBinding::TexelFormat::kNone;
    }
    return ResourceBinding::TexelFormat::kNone;
}

}  // namespace tint::inspector
