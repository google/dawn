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

#include "dawn/native/DynamicArrayState.h"

#include "dawn/native/Buffer.h"
#include "dawn/native/Device.h"

namespace dawn::native {

DynamicArrayState::DynamicArrayState(BindingIndex size) {
    mBindings.resize(size);
}

MaybeError DynamicArrayState::Initialize(DeviceBase* device) {
    // Create a storage buffer that will hold the shader-visible metadata for the dynamic array.
    BufferDescriptor metadataDesc{
        .label = "binding array metadata",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
        .size = 4,
        .mappedAtCreation = true,
    };
    DAWN_TRY_ASSIGN(mMetadataBuffer, device->CreateBuffer(&metadataDesc));

    // TODO(https://crbug.com/439522242): For now it only contains the size but we also need to add
    // type information for each entry in the future.
    uint32_t* data = static_cast<uint32_t*>(mMetadataBuffer->GetMappedRange(0, metadataDesc.size));
    *data = uint32_t(mBindings.size());
    DAWN_TRY(mMetadataBuffer->Unmap());

    return {};
}

BindingIndex DynamicArrayState::GetSize() const {
    DAWN_ASSERT(!mDestroyed);
    return mBindings.size();
}

ityp::span<BindingIndex, const Ref<TextureViewBase>> DynamicArrayState::GetBindings() const {
    DAWN_ASSERT(!mDestroyed);
    return {mBindings.data(), mBindings.size()};
}

BufferBase* DynamicArrayState::GetMetadataBuffer() const {
    DAWN_ASSERT(!mDestroyed);
    return mMetadataBuffer.Get();
}

bool DynamicArrayState::IsDestroyed() const {
    return mDestroyed;
}

void DynamicArrayState::Update(BindingIndex i, TextureViewBase* view) {
    DAWN_ASSERT(!mDestroyed);
    mBindings[i] = view;
}

void DynamicArrayState::Destroy() {
    DAWN_ASSERT(!mDestroyed);
    mBindings.clear();
    mMetadataBuffer->Destroy();
    mMetadataBuffer = nullptr;
    mDestroyed = true;
}

}  // namespace dawn::native
