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

#include "src/dawn/native/d3d12/SharedBufferMemoryD3D12.h"

#include <memory>
#include <utility>

#include "absl/cleanup/cleanup.h"
#include "src/dawn/common/Math.h"
#include "src/dawn/native/Buffer.h"
#include "src/dawn/native/ChainUtils.h"
#include "src/dawn/native/d3d/D3DError.h"
#include "src/dawn/native/d3d/SharedFenceD3D.h"
#include "src/dawn/native/d3d/UtilsD3D.h"
#include "src/dawn/native/d3d12/BufferD3D12.h"
#include "src/dawn/native/d3d12/DeviceD3D12.h"
#include "src/dawn/native/d3d12/HeapD3D12.h"
#include "src/dawn/native/d3d12/QueueD3D12.h"
#include "src/dawn/native/d3d12/ResidencyManagerD3D12.h"
#include "src/utils/compiler.h"

namespace dawn::native::d3d12 {

namespace {

enum class HeapAccessType {
    Upload,
    Readback,
    GPUQueueAccessible,
};

ResultOrError<HeapAccessType> MapToHeapAccessType(const D3D12_HEAP_PROPERTIES& heapProperties,
                                                  const D3D12_HEAP_FLAGS& heapFlags,
                                                  const Device* device) {
    switch (heapProperties.Type) {
        case D3D12_HEAP_TYPE_UPLOAD:
            return HeapAccessType::Upload;
        case D3D12_HEAP_TYPE_READBACK:
            return HeapAccessType::Readback;
        case D3D12_HEAP_TYPE_DEFAULT:
            return HeapAccessType::GPUQueueAccessible;
        case D3D12_HEAP_TYPE_CUSTOM:
            if (heapFlags & D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER) {
                // A CUSTOM shared cross-adapter heap is equivalent to a DEFAULT heap.
                // https://learn.microsoft.com/en-us/windows/win32/direct3d12/shared-heaps
                return HeapAccessType::GPUQueueAccessible;
            }

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

ResultOrError<SharedBufferMemoryProperties> GetSharedBufferMemoryProperties(
    Device* device,
    D3D12_HEAP_PROPERTIES heapProperties,
    D3D12_HEAP_FLAGS heapFlags,
    uint64_t size,
    wgpu::BufferUsage blockedUsages = wgpu::BufferUsage::None) {
    HeapAccessType heapType;
    DAWN_TRY_ASSIGN(heapType, MapToHeapAccessType(heapProperties, heapFlags, device));

    wgpu::BufferUsage usages = wgpu::BufferUsage::None;

    switch (heapType) {
        case HeapAccessType::Upload:
            usages |= wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
            break;
        case HeapAccessType::Readback:
            usages |= wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;
            break;
        case HeapAccessType::GPUQueueAccessible:
            // wgpu::BufferUsage::Uniform is not allowed in SharedBufferMemoryBase::CreateBuffer().
            usages |= wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst |
                      wgpu::BufferUsage::Vertex | wgpu::BufferUsage::Index |
                      wgpu::BufferUsage::Indirect | wgpu::BufferUsage::QueryResolve |
                      wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage;
            if (device->GetDeviceInfo().isUMA) {
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
    properties.size = size;
    properties.usage = usages & (~blockedUsages);

    return properties;
}

}  // namespace

SharedBufferMemory::SharedBufferMemory(Device* device,
                                       StringView label,
                                       SharedBufferMemoryProperties properties,
                                       ComPtr<ID3D12Resource> resource)
    : SharedBufferMemoryBase(device, label, properties), mResource(std::move(resource)) {}

SharedBufferMemory::SharedBufferMemory(Device* device,
                                       StringView label,
                                       SharedBufferMemoryProperties properties,
                                       std::unique_ptr<Heap> heap,
                                       ComPtr<ID3D12Resource> resource)
    : SharedBufferMemoryBase(device, label, properties),
      mHeap(std::move(heap)),
      mResource(std::move(resource)) {}

void SharedBufferMemory::DestroyImpl(DestroyReason reason) {
    SharedBufferMemoryBase::DestroyImpl(reason);

    if (mHostPointerDisposeCallback == nullptr) {
        ToBackend(GetDevice())->ReferenceUntilUnused(std::move(mResource));
        return;
    }

    // The host memory aliased by the heap must outlive both the heap and the resource placed on
    // it, so the task owns the resource as well: it is only released once the task runs, which
    // happens after the pending command serial has completed. Releasing the resource from the
    // task (instead of through Device::ReferenceUntilUnused, which is drained by a different
    // mechanism) guarantees the resource is released before the heap, and the heap before the
    // dispose callback.
    struct DisposeTask : TrackTaskCallback {
        DisposeTask(ComPtr<ID3D12Resource> resource,
                    std::unique_ptr<Heap> heap,
                    wgpu::DisposeCallback callback,
                    void* userdata)
            : TrackTaskCallback(nullptr),
              resource(std::move(resource)),
              heap(std::move(heap)),
              callback(callback),
              userdata(userdata) {}
        ~DisposeTask() override = default;

        void FinishImpl() override { Dispose(WGPUCallbackStatus_Success); }
        void HandleDeviceLossImpl() override { Dispose(WGPUCallbackStatus_Error); }
        void HandleShutDownImpl() override { Dispose(WGPUCallbackStatus_Error); }

        void Dispose(WGPUCallbackStatus status) {
            resource = nullptr;
            heap = nullptr;
            callback(status, userdata);
        }

        ComPtr<ID3D12Resource> resource;
        std::unique_ptr<Heap> heap;
        wgpu::DisposeCallback callback;
        raw_ptr<void, DisableDanglingPtrDetection> userdata;
    };
    std::unique_ptr<DisposeTask> request =
        std::make_unique<DisposeTask>(std::move(mResource), std::move(mHeap),
                                      mHostPointerDisposeCallback, mHostPointerDisposeUserdata);
    mHostPointerDisposeCallback = nullptr;
    mHostPointerDisposeUserdata = nullptr;

    // TODO(386255678): TrackTaskAfterEventualFlush() only marks the queue as needing a submit; it
    // doesn't force one to happen immediately. If nothing ever ticks the device again, this task
    // (and thus disposeCallback) may never run.
    GetDevice()->GetQueue()->TrackTaskAfterEventualFlush(std::move(request));
}

// static
ResultOrError<Ref<SharedBufferMemory>> SharedBufferMemory::CreateFromHeap(
    Device* device,
    StringView label,
    ComPtr<ID3D12Heap> d3d12Heap,
    uint64_t size,
    const void* heapPointer,
    wgpu::BufferUsage blockedUsages) {
    D3D12_HEAP_DESC heapDesc = d3d12Heap->GetDesc();

    SharedBufferMemoryProperties properties;
    DAWN_TRY_ASSIGN(properties,
                    GetSharedBufferMemoryProperties(device, heapDesc.Properties, heapDesc.Flags,
                                                    size, blockedUsages));

    // D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER must be specified if and only if
    // D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER is set.
    D3D12_RESOURCE_FLAGS resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    if (heapDesc.Flags & D3D12_HEAP_FLAG_SHARED_CROSS_ADAPTER) {
        resourceFlags |= D3D12_RESOURCE_FLAG_ALLOW_CROSS_ADAPTER;
    }

    D3D12_RESOURCE_DESC resourceDescriptor;
    resourceDescriptor.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
    resourceDescriptor.Alignment = 0;
    resourceDescriptor.Width = size;
    resourceDescriptor.Height = 1;
    resourceDescriptor.DepthOrArraySize = 1;
    resourceDescriptor.MipLevels = 1;
    resourceDescriptor.Format = DXGI_FORMAT_UNKNOWN;
    resourceDescriptor.SampleDesc.Count = 1;
    resourceDescriptor.SampleDesc.Quality = 0;
    resourceDescriptor.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
    resourceDescriptor.Flags = resourceFlags;

    uint64_t heapSize = heapDesc.SizeInBytes;
    D3D12_RESOURCE_ALLOCATION_INFO resourceInfo =
        device->GetD3D12Device()->GetResourceAllocationInfo(0, 1, &resourceDescriptor);
    DAWN_INVALID_IF(resourceInfo.SizeInBytes > heapSize,
                    "Resource required %u bytes, but heap is %u bytes.", resourceInfo.SizeInBytes,
                    heapSize);
    DAWN_INVALID_IF(heapPointer != nullptr &&
                        !IsPtrAligned(heapPointer, checked_cast<size_t>(resourceInfo.Alignment)),
                    "Host pointer (%p) did not satisfy required alignment (%u).", heapPointer,
                    resourceInfo.Alignment);
    auto heap = std::make_unique<Heap>(
        std::move(d3d12Heap),
        device->GetDeviceInfo().isUMA ? MemorySegment::Local : MemorySegment::NonLocal, size);

    ComPtr<ID3D12Resource> placedResource;
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        device->GetD3D12Device()->CreatePlacedResource(heap->GetD3D12Heap(), 0, &resourceDescriptor,
                                                       D3D12_RESOURCE_STATE_COMMON, nullptr,
                                                       IID_PPV_ARGS(&placedResource)),
        "ID3D12Device::CreatePlacedResource"));

    auto result = AcquireRef(new SharedBufferMemory(device, label, properties, std::move(heap),
                                                    std::move(placedResource)));
    result->Initialize();
    return result;
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

    // wgpu::BufferUsage::Uniform is disabled because the external D3D12 resource may not meet the
    // requirement of D3D12 constant buffer alignment(256).
    wgpu::BufferUsage blockedUsages = wgpu::BufferUsage::Uniform;
    if (!(desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS)) {
        blockedUsages |= wgpu::BufferUsage::Storage;
    }
    SharedBufferMemoryProperties properties;
    DAWN_TRY_ASSIGN(properties, GetSharedBufferMemoryProperties(device, heapProperties, heapFlags,
                                                                desc.Width, blockedUsages));

    auto result =
        AcquireRef(new SharedBufferMemory(device, label, properties, std::move(d3d12Resource)));
    result->Initialize();
    return result;
}

// static
ResultOrError<Ref<SharedBufferMemory>> SharedBufferMemory::Create(
    Device* device,
    StringView label,
    const SharedBufferMemoryFromWindowsHandleDescriptor* descriptor) {
    HANDLE sharedMemoryFileHandle = descriptor->handle;
    DAWN_INVALID_IF(sharedMemoryFileHandle == nullptr, "shared HANDLE is missing.");

    constexpr uint32_t kAlignment = kD3D12SharedBufferMemoryFileMappingHandleSizeAlignment;
    DAWN_INVALID_IF(descriptor->size % kAlignment != 0,
                    "shared buffer memory size is not a multiple of (%d).", kAlignment);

    ComPtr<ID3D12Device3> d3d12Device3;
    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->QueryInterface(IID_PPV_ARGS(&d3d12Device3)),
                          "QueryInterface ID3D12Device3"));

    ComPtr<ID3D12Heap> d3d12Heap;
    DAWN_TRY(CheckOutOfMemoryHRESULT(d3d12Device3->OpenExistingHeapFromFileMapping(
                                         sharedMemoryFileHandle, IID_PPV_ARGS(&d3d12Heap)),
                                     "ID3D12Device3::OpenExistingHeapFromFileMapping"));

    return CreateFromHeap(device, label, std::move(d3d12Heap), descriptor->size, nullptr,
                          wgpu::BufferUsage::None);
}

// static
ResultOrError<Ref<SharedBufferMemory>> SharedBufferMemory::Create(
    Device* device,
    StringView label,
    const SharedBufferMemoryHostPointerDescriptor* descriptor) {
    // `disposeCallback` must be called exactly once. If this function returns a SharedBufferMemory,
    // ownership of the callback is transferred to it and it is called on destruction. Otherwise,
    // this guard invokes it here with an error status before returning.
    // TODO(386255678): this only covers errors produced inside this function. It doesn't help if
    // Device::CreateSharedBufferMemory errors before or after calling this, e.g. due to unpacking
    // the chained struct, or a device loss / D3D12 error raised elsewhere in the meantime.
    absl::Cleanup disposeOnFailure = [descriptor] {
        if (descriptor->disposeCallback) {
            descriptor->disposeCallback(WGPUCallbackStatus_Error, descriptor->userdata);
        }
    };

    DAWN_INVALID_IF(descriptor->pointer == nullptr, "Host pointer is missing.");
    DAWN_INVALID_IF(descriptor->size == 0, "Shared buffer memory size must not be 0.");

    // OpenExistingHeapFromAddress requires the address to be aligned to the D3D12 default resource
    // placement alignment, or it fails with E_INVALIDARG.
    DAWN_INVALID_IF(!IsPtrAligned(descriptor->pointer, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT),
                    "Host pointer (%p) is not aligned to %u bytes.", descriptor->pointer,
                    D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT);

    ComPtr<ID3D12Device3> d3d12Device3;
    DAWN_TRY(CheckHRESULT(device->GetD3D12Device()->QueryInterface(IID_PPV_ARGS(&d3d12Device3)),
                          "QueryInterface ID3D12Device3"));

    ComPtr<ID3D12Heap> d3d12Heap;
    DAWN_TRY(CheckOutOfMemoryHRESULT(
        d3d12Device3->OpenExistingHeapFromAddress(descriptor->pointer, IID_PPV_ARGS(&d3d12Heap)),
        "ID3D12Device3::OpenExistingHeapFromAddress"));

    Ref<SharedBufferMemory> result;
    DAWN_TRY_ASSIGN(result, CreateFromHeap(device, label, std::move(d3d12Heap), descriptor->size,
                                           descriptor->pointer, wgpu::BufferUsage::None));
    result->mHostPointerDisposeCallback = descriptor->disposeCallback;
    result->mHostPointerDisposeUserdata = descriptor->userdata;
    std::move(disposeOnFailure).Cancel();
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
    for (SharedFenceBase* fence : descriptor->fences) {
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
