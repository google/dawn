// Copyright 2024 The Dawn & Tint Authors
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

#include <utility>

#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/SharedBufferMemoryD3D12.h"

namespace dawn::native::d3d12 {

SharedBufferMemory::SharedBufferMemory(Device* device,
                                       const char* label,
                                       SharedBufferMemoryProperties properties,
                                       ComPtr<ID3D12Resource> resource)
    : SharedBufferMemoryBase(device, label, properties), mResource(std::move(resource)) {}

// static
ResultOrError<Ref<SharedBufferMemory>> SharedBufferMemory::Create(
    Device* device,
    const char* label,
    const SharedBufferMemoryD3D12ResourceDescriptor* descriptor) {
    DAWN_INVALID_IF(!descriptor->resource, "D3D12 resource is missing.");

    ComPtr<ID3D12Resource> d3d12Resource = descriptor->resource;

    ID3D12Device* resourceDevice = nullptr;
    d3d12Resource->GetDevice(__uuidof(*resourceDevice), reinterpret_cast<void**>(&resourceDevice));
    DAWN_INVALID_IF(resourceDevice != device->GetD3D12Device(),
                    "The D3D12 device of the resource and the D3D12 device of %s must be same.",
                    device);
    resourceDevice->Release();

    D3D12_RESOURCE_DESC desc = d3d12Resource->GetDesc();
    DAWN_INVALID_IF(desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER,
                    "Resource dimension (%d) was not Buffer", desc.Dimension);

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    d3d12Resource->GetHeapProperties(&heapProperties, &heapFlags);

    wgpu::BufferUsage usages = wgpu::BufferUsage::None;

    if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
        usages |=
            wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    } else if (heapProperties.Type == D3D12_HEAP_TYPE_UPLOAD) {
        usages |= wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    } else if (heapProperties.Type == D3D12_HEAP_TYPE_READBACK) {
        usages |= wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
    }

    SharedBufferMemoryProperties properties;
    properties.size = desc.Width;
    properties.usage = usages;

    auto result =
        AcquireRef(new SharedBufferMemory(device, label, properties, std::move(d3d12Resource)));
    result->Initialize();
    return result;
}

ResultOrError<Ref<BufferBase>> SharedBufferMemory::CreateBufferImpl(
    const UnpackedPtr<BufferDescriptor>& descriptor) {
    return Buffer::CreateFromSharedBufferMemory(this, descriptor);
}

ID3D12Resource* SharedBufferMemory::GetD3DResource() const {
    return mResource.Get();
}

}  // namespace dawn::native::d3d12
