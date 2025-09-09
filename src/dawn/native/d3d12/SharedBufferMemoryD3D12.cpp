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

#include "dawn/native/d3d12/SharedBufferMemoryD3D12.h"

#include <utility>

#include "dawn/native/Buffer.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/SharedFenceD3D.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/native/d3d12/BufferD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/QueueD3D12.h"

namespace dawn::native::d3d12 {

namespace {

enum class HeapAccessType {
    Upload,
    Readback,
    GPUQueueAccessible,
};

ResultOrError<HeapAccessType> MapToHeapAccessType(const D3D12_HEAP_PROPERTIES& heapProperties,
                                                  const Device* device) {
    switch (heapProperties.Type) {
        case D3D12_HEAP_TYPE_UPLOAD:
            return HeapAccessType::Upload;
        case D3D12_HEAP_TYPE_READBACK:
            return HeapAccessType::Readback;
        case D3D12_HEAP_TYPE_DEFAULT:
            return HeapAccessType::GPUQueueAccessible;
        case D3D12_HEAP_TYPE_CUSTOM:
            if (device->GetDeviceInfo().isUMA) {
                // On UMA systems, all heaps are always GPU accessible.
                return HeapAccessType::GPUQueueAccessible;
            }

            // Map D3D12_HEAP_TYPE_CUSTOM heap to one of the standard heap types if possible.
            // See:
            // https://learn.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-getcustomheapproperties(uint_d3d12_heap_type)
            if (heapProperties.CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_NOT_AVAILABLE &&
                heapProperties.MemoryPoolPreference == D3D12_MEMORY_POOL_L1) {
                // A CUSTOM heap with no CPU access and in L1 is equivalent to a DEFAULT heap.
                return HeapAccessType::GPUQueueAccessible;
            } else if (heapProperties.CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK &&
                       heapProperties.MemoryPoolPreference == D3D12_MEMORY_POOL_L0) {
                // A CUSTOM heap with WRITE_BACK + L0 is equivalent to a READBACK heap.
                return HeapAccessType::Readback;
            } else if (heapProperties.CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE &&
                       heapProperties.MemoryPoolPreference == D3D12_MEMORY_POOL_L0) {
                // A CUSTOM heap with WRITE_COMBINE + L0 is equivalent to a UPLOAD heap.
                return HeapAccessType::Upload;
            } else {
                return DAWN_VALIDATION_ERROR("ID3D12Resources allocated on unsupported heap.");
            }
        default:
            return DAWN_VALIDATION_ERROR("ID3D12Resources allocated on unsupported heap.");
    }
}
}  // namespace

SharedBufferMemory::SharedBufferMemory(Device* device,
                                       StringView label,
                                       SharedBufferMemoryProperties properties,
                                       ComPtr<ID3D12Resource> resource)
    : SharedBufferMemoryBase(device, label, properties), mResource(std::move(resource)) {}

void SharedBufferMemory::DestroyImpl() {
    ToBackend(GetDevice())->ReferenceUntilUnused(std::move(mResource));
}

// static
ResultOrError<Ref<SharedBufferMemory>> SharedBufferMemory::Create(
    Device* device,
    StringView label,
    const SharedBufferMemoryD3D12ResourceDescriptor* descriptor) {
    DAWN_INVALID_IF(!descriptor->resource, "D3D12 resource is missing.");

    ComPtr<ID3D12Resource> d3d12Resource = descriptor->resource;

    ComPtr<ID3D12Device> resourceDevice;
    d3d12Resource->GetDevice(__uuidof(resourceDevice), &resourceDevice);
    DAWN_INVALID_IF(resourceDevice.Get() != device->GetD3D12Device(),
                    "The D3D12 device of the resource and the D3D12 device of %s must be same.",
                    device);

    D3D12_RESOURCE_DESC desc = d3d12Resource->GetDesc();
    DAWN_INVALID_IF(desc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER,
                    "Resource dimension (%d) was not Buffer", desc.Dimension);

    D3D12_HEAP_PROPERTIES heapProperties;
    D3D12_HEAP_FLAGS heapFlags;
    d3d12Resource->GetHeapProperties(&heapProperties, &heapFlags);

    wgpu::BufferUsage usages = wgpu::BufferUsage::None;
    HeapAccessType heapType;
    DAWN_TRY_ASSIGN(heapType, MapToHeapAccessType(heapProperties, device));

    switch (heapType) {
        case HeapAccessType::Upload:
            usages |= wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
            break;
        case HeapAccessType::Readback:
            usages |= wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
            break;
        case HeapAccessType::GPUQueueAccessible:
            usages |= wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                      wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
                      wgpu::BufferUsage::Indirect | wgpu::BufferUsage::QueryResolve;
            if (desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS) {
                usages |= wgpu::BufferUsage::Storage;
            }
            if (IsAligned(desc.Width, D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT)) {
                usages |= wgpu::BufferUsage::Uniform;
            }

            if (device->GetDeviceInfo().isUMA &&
                device->HasFeature(Feature::BufferMapExtendedUsages)) {
                // On UMA systems, buffers with WRITE_COMBINE or WRITE_BACK heaps can also be
                // mapped.
                if (heapProperties.CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_COMBINE) {
                    usages |= wgpu::BufferUsage::MapWrite;
                } else if (heapProperties.CPUPageProperty == D3D12_CPU_PAGE_PROPERTY_WRITE_BACK) {
                    // On cache-coherent UMA systems, writes are immediately visible to the GPU. On
                    // non-cache-coherent UMA systems, writes are flushed to the GPU when unmapping
                    // or submitting work to the queue (driver dependent). Since Dawn doesn't
                    // support submitting work to the queue while the buffer is mapped, it should be
                    // safe to allow MapWrite on WRITE_BACK heaps. For reads, the data is guaranteed
                    // to be available to the CPU after Map().
                    usages |= wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
                }
            }
            break;
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

MaybeError SharedBufferMemory::BeginAccessImpl(
    BufferBase* buffer,
    const UnpackedPtr<BeginAccessDescriptor>& descriptor) {
    DAWN_TRY(descriptor.ValidateSubset<>());
    for (size_t i = 0; i < descriptor->fenceCount; ++i) {
        SharedFenceBase* fence = descriptor->fences[i];

        SharedFenceExportInfo exportInfo;
        DAWN_TRY(fence->ExportInfo(&exportInfo));
        switch (exportInfo.type) {
            case wgpu::SharedFenceType::DXGISharedHandle:
                DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceDXGISharedHandle),
                                "Required feature (%s) is missing.",
                                wgpu::FeatureName::SharedFenceDXGISharedHandle);
                break;
            default:
                return DAWN_VALIDATION_ERROR("Unsupported fence type %s.", exportInfo.type);
        }
    }

    return {};
}

ResultOrError<FenceAndSignalValue> SharedBufferMemory::EndAccessImpl(
    BufferBase* buffer,
    ExecutionSerial lastUsageSerial,
    UnpackedPtr<EndAccessState>& state) {
    DAWN_TRY(state.ValidateSubset<>());
    DAWN_INVALID_IF(!GetDevice()->HasFeature(Feature::SharedFenceDXGISharedHandle),
                    "Required feature (%s) is missing.",
                    wgpu::FeatureName::SharedFenceDXGISharedHandle);

    Ref<d3d::SharedFence> sharedFence;
    DAWN_TRY_ASSIGN(sharedFence, ToBackend(GetDevice()->GetQueue())->GetOrCreateSharedFence());

    return FenceAndSignalValue{std::move(sharedFence), static_cast<uint64_t>(lastUsageSerial)};
}

}  // namespace dawn::native::d3d12
