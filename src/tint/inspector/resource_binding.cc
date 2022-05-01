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

#include "src/tint/sem/array.h"
#include "src/tint/sem/f32.h"
#include "src/tint/sem/i32.h"
#include "src/tint/sem/matrix.h"
#include "src/tint/sem/type.h"
#include "src/tint/sem/u32.h"
#include "src/tint/sem/vector.h"

namespace tint::inspector {

ResourceBinding::TextureDimension TypeTextureDimensionToResourceBindingTextureDimension(
    const ast::TextureDimension& type_dim) {
    switch (type_dim) {
        case ast::TextureDimension::k1d:
            return ResourceBinding::TextureDimension::k1d;
        case ast::TextureDimension::k2d:
            return ResourceBinding::TextureDimension::k2d;
        case ast::TextureDimension::k2dArray:
            return ResourceBinding::TextureDimension::k2dArray;
        case ast::TextureDimension::k3d:
            return ResourceBinding::TextureDimension::k3d;
        case ast::TextureDimension::kCube:
            return ResourceBinding::TextureDimension::kCube;
        case ast::TextureDimension::kCubeArray:
            return ResourceBinding::TextureDimension::kCubeArray;
        case ast::TextureDimension::kNone:
            return ResourceBinding::TextureDimension::kNone;
    }
    return ResourceBinding::TextureDimension::kNone;
}

ResourceBinding::SampledKind BaseTypeToSampledKind(const sem::Type* base_type) {
    if (!base_type) {
        return ResourceBinding::SampledKind::kUnknown;
    }

    if (auto* at = base_type->As<sem::Array>()) {
        base_type = at->ElemType();
    } else if (auto* mt = base_type->As<sem::Matrix>()) {
        base_type = mt->type();
    } else if (auto* vt = base_type->As<sem::Vector>()) {
        base_type = vt->type();
    }

    if (base_type->Is<sem::F32>()) {
        return ResourceBinding::SampledKind::kFloat;
    } else if (base_type->Is<sem::U32>()) {
        return ResourceBinding::SampledKind::kUInt;
    } else if (base_type->Is<sem::I32>()) {
        return ResourceBinding::SampledKind::kSInt;
    } else {
        return ResourceBinding::SampledKind::kUnknown;
    }
}

ResourceBinding::TexelFormat TypeTexelFormatToResourceBindingTexelFormat(
    const ast::TexelFormat& image_format) {
    switch (image_format) {
        case ast::TexelFormat::kR32Uint:
            return ResourceBinding::TexelFormat::kR32Uint;
        case ast::TexelFormat::kR32Sint:
            return ResourceBinding::TexelFormat::kR32Sint;
        case ast::TexelFormat::kR32Float:
            return ResourceBinding::TexelFormat::kR32Float;
        case ast::TexelFormat::kRgba8Unorm:
            return ResourceBinding::TexelFormat::kRgba8Unorm;
        case ast::TexelFormat::kRgba8Snorm:
            return ResourceBinding::TexelFormat::kRgba8Snorm;
        case ast::TexelFormat::kRgba8Uint:
            return ResourceBinding::TexelFormat::kRgba8Uint;
        case ast::TexelFormat::kRgba8Sint:
            return ResourceBinding::TexelFormat::kRgba8Sint;
        case ast::TexelFormat::kRg32Uint:
            return ResourceBinding::TexelFormat::kRg32Uint;
        case ast::TexelFormat::kRg32Sint:
            return ResourceBinding::TexelFormat::kRg32Sint;
        case ast::TexelFormat::kRg32Float:
            return ResourceBinding::TexelFormat::kRg32Float;
        case ast::TexelFormat::kRgba16Uint:
            return ResourceBinding::TexelFormat::kRgba16Uint;
        case ast::TexelFormat::kRgba16Sint:
            return ResourceBinding::TexelFormat::kRgba16Sint;
        case ast::TexelFormat::kRgba16Float:
            return ResourceBinding::TexelFormat::kRgba16Float;
        case ast::TexelFormat::kRgba32Uint:
            return ResourceBinding::TexelFormat::kRgba32Uint;
        case ast::TexelFormat::kRgba32Sint:
            return ResourceBinding::TexelFormat::kRgba32Sint;
        case ast::TexelFormat::kRgba32Float:
            return ResourceBinding::TexelFormat::kRgba32Float;
        case ast::TexelFormat::kNone:
            return ResourceBinding::TexelFormat::kNone;
    }
    return ResourceBinding::TexelFormat::kNone;
}

}  // namespace tint::inspector
