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

#include "dawn/native/opengl/BindGroupGL.h"

#include "dawn/native/Texture.h"
#include "dawn/native/opengl/BindGroupLayoutGL.h"
#include "dawn/native/opengl/DeviceGL.h"

namespace dawn::native::opengl {

MaybeError ValidateGLBindGroupDescriptor(const BindGroupDescriptor* descriptor) {
    const BindGroupLayoutInternalBase::BindingMap& bindingMap = descriptor->layout->GetBindingMap();
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];

        const auto& it = bindingMap.find(BindingNumber(entry.binding));
        BindingIndex bindingIndex = it->second;
        ASSERT(bindingIndex < descriptor->layout->GetBindingCount());

        const BindingInfo& bindingInfo = descriptor->layout->GetBindingInfo(bindingIndex);
        if (bindingInfo.bindingType == BindingInfoType::StorageTexture) {
            ASSERT(entry.textureView != nullptr);
            const uint32_t textureViewLayerCount = entry.textureView->GetLayerCount();
            DAWN_INVALID_IF(
                textureViewLayerCount != 1 &&
                    textureViewLayerCount != entry.textureView->GetTexture()->GetArrayLayers(),
                "%s binds %u layers. Currently the OpenGL backend only supports either binding "
                "1 layer or the all layers (%u) for storage texture.",
                entry.textureView, textureViewLayerCount,
                entry.textureView->GetTexture()->GetArrayLayers());
        }
    }

    return {};
}

BindGroup::BindGroup(Device* device, const BindGroupDescriptor* descriptor)
    : BindGroupBase(this, device, descriptor) {}

BindGroup::~BindGroup() = default;

void BindGroup::DestroyImpl() {
    BindGroupBase::DestroyImpl();
    ToBackend(GetLayout()->GetInternalBindGroupLayout())->DeallocateBindGroup(this);
}

// static
Ref<BindGroup> BindGroup::Create(Device* device, const BindGroupDescriptor* descriptor) {
    return ToBackend(descriptor->layout->GetInternalBindGroupLayout())
        ->AllocateBindGroup(device, descriptor);
}

}  // namespace dawn::native::opengl
