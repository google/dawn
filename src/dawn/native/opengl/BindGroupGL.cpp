// Copyright 2020 The Dawn & Tint Authors
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

#include "dawn/native/opengl/BindGroupGL.h"

#include "dawn/native/Texture.h"
#include "dawn/native/opengl/BindGroupLayoutGL.h"
#include "dawn/native/opengl/DeviceGL.h"

namespace dawn::native::opengl {

MaybeError ValidateGLBindGroupDescriptor(const BindGroupDescriptor* descriptor) {
    BindGroupLayoutInternalBase* layout = descriptor->layout->GetInternalBindGroupLayout();
    const BindGroupLayoutInternalBase::BindingMap& bindingMap = layout->GetBindingMap();
    for (uint32_t i = 0; i < descriptor->entryCount; ++i) {
        const BindGroupEntry& entry = descriptor->entries[i];

        const auto& it = bindingMap.find(BindingNumber(entry.binding));
        BindingIndex bindingIndex = it->second;
        DAWN_ASSERT(bindingIndex < layout->GetBindingCount());

        const BindingInfo& bindingInfo = layout->GetBindingInfo(bindingIndex);
        if (bindingInfo.bindingType == BindingInfoType::StorageTexture) {
            DAWN_ASSERT(entry.textureView != nullptr);
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
    ToBackend(GetLayout())->DeallocateBindGroup(this);
}

// static
Ref<BindGroup> BindGroup::Create(Device* device, const BindGroupDescriptor* descriptor) {
    return ToBackend(descriptor->layout->GetInternalBindGroupLayout())
        ->AllocateBindGroup(device, descriptor);
}

}  // namespace dawn::native::opengl
