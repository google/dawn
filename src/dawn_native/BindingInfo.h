// Copyright 2020 The Dawn Authors
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

#ifndef DAWNNATIVE_BINDINGINFO_H_
#define DAWNNATIVE_BINDINGINFO_H_

#include "dawn_native/Format.h"
#include "dawn_native/dawn_platform.h"

#include <cstdint>

namespace dawn_native {

    // TODO(enga): Can we have strongly typed integers so you can't convert between them
    // by accident? And also range-assertions (ex. kMaxBindingsPerGroup) in Debug?

    // Binding numbers in the shader and BindGroup/BindGroupLayoutDescriptors
    using BindingNumber = uint32_t;

    // Binding numbers get mapped to a packed range of indices
    using BindingIndex = uint32_t;

    struct BindingInfo {
        wgpu::ShaderStage visibility;
        wgpu::BindingType type;
        Format::Type textureComponentType = Format::Type::Float;
        wgpu::TextureViewDimension textureDimension = wgpu::TextureViewDimension::Undefined;
        wgpu::TextureFormat storageTextureFormat = wgpu::TextureFormat::Undefined;
        bool hasDynamicOffset = false;
        bool multisampled = false;
    };

}  // namespace dawn_native

#endif  // DAWNNATIVE_BINDINGINFO_H_
