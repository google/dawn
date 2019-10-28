// Copyright 2019 The Dawn Authors
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

#include "dawn_native/d3d12/ResourceAllocatorManagerD3D12.h"

#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/HeapAllocatorD3D12.h"
#include "dawn_native/d3d12/HeapD3D12.h"

namespace dawn_native { namespace d3d12 {
    namespace {
        D3D12_HEAP_TYPE GetD3D12HeapType(ResourceHeapKind resourceHeapKind) {
            switch (resourceHeapKind) {
                case Readback_OnlyBuffers:
                    return D3D12_HEAP_TYPE_READBACK;
                case Default_OnlyBuffers:
                case Default_OnlyNonRenderableOrDepthTextures:
                case Default_OnlyRenderableOrDepthTextures:
                    return D3D12_HEAP_TYPE_DEFAULT;
                case Upload_OnlyBuffers:
                    return D3D12_HEAP_TYPE_UPLOAD;
                default:
                    UNREACHABLE();
            }
        }

        D3D12_HEAP_FLAGS GetD3D12HeapFlags(ResourceHeapKind resourceHeapKind) {
            switch (resourceHeapKind) {
                case Default_OnlyBuffers:
                case Readback_OnlyBuffers:
                case Upload_OnlyBuffers:
                    return D3D12_HEAP_FLAG_ALLOW_ONLY_BUFFERS;
                case Default_OnlyNonRenderableOrDepthTextures:
                    return D3D12_HEAP_FLAG_ALLOW_ONLY_NON_RT_DS_TEXTURES;
                case Default_OnlyRenderableOrDepthTextures:
                    return D3D12_HEAP_FLAG_ALLOW_ONLY_RT_DS_TEXTURES;
                default:
                    UNREACHABLE();
            }
        }

        ResourceHeapKind GetResourceHeapKind(D3D12_RESOURCE_DIMENSION dimension,
                                             D3D12_HEAP_TYPE heapType,
                                             D3D12_RESOURCE_FLAGS flags) {
            switch (dimension) {
                case D3D12_RESOURCE_DIMENSION_BUFFER: {
                    switch (heapType) {
                        case D3D12_HEAP_TYPE_UPLOAD:
                            return Upload_OnlyBuffers;
                        case D3D12_HEAP_TYPE_DEFAULT:
                            return Default_OnlyBuffers;
                        case D3D12_HEAP_TYPE_READBACK:
                            return Readback_OnlyBuffers;
                        default:
                            UNREACHABLE();
                    }
                } break;
                case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
                case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
                case D3D12_RESOURCE_DIMENSION_TEXTURE3D: {
                    switch (heapType) {
                        case D3D12_HEAP_TYPE_DEFAULT: {
                            if ((flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL) ||
                                (flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET)) {
                                return Default_OnlyRenderableOrDepthTextures;
                            } else {
                                return Default_OnlyNonRenderableOrDepthTextures;
                            }
                        } break;
                        default:
                            UNREACHABLE();
                    }
                } break;
                default:
                    UNREACHABLE();
            }
        }
    }  // namespace

    ResourceAllocatorManager::ResourceAllocatorManager(Device* device) : mDevice(device) {
        for (uint32_t i = 0; i < ResourceHeapKind::EnumCount; i++) {
            const ResourceHeapKind resourceHeapKind = static_cast<ResourceHeapKind>(i);
            mHeapAllocators[i] = std::make_unique<HeapAllocator>(
                mDevice, GetD3D12HeapType(resourceHeapKind), GetD3D12HeapFlags(resourceHeapKind));
            mSubAllocatedResourceAllocators[i] = std::make_unique<BuddyMemoryAllocator>(
                kMaxHeapSize, kMinHeapSize, mHeapAllocators[i].get());
        }
    }

    ResultOrError<ResourceHeapAllocation> ResourceAllocatorManager::AllocateMemory(
        D3D12_HEAP_TYPE heapType,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage) {
        // TODO(bryan.bernhart@intel.com): Conditionally disable sub-allocation.
        // For very large resources, there is no benefit to suballocate.
        // For very small resources, it is inefficent to suballocate given the min. heap
        // size could be much larger then the resource allocation.
        // Attempt to satisfy the request using sub-allocation (placed resource in a heap).
        ResourceHeapAllocation subAllocation;
        DAWN_TRY_ASSIGN(subAllocation,
                        CreatePlacedResource(heapType, resourceDescriptor, initialUsage));
        if (subAllocation.GetInfo().mMethod != AllocationMethod::kInvalid) {
            return subAllocation;
        }

        // If sub-allocation fails, fall-back to direct allocation (committed resource).
        ResourceHeapAllocation directAllocation;
        DAWN_TRY_ASSIGN(directAllocation,
                        CreateCommittedResource(heapType, resourceDescriptor, initialUsage));

        return directAllocation;
    }

    void ResourceAllocatorManager::Tick(Serial completedSerial) {
        for (ResourceHeapAllocation& allocation :
             mAllocationsToDelete.IterateUpTo(completedSerial)) {
            if (allocation.GetInfo().mMethod == AllocationMethod::kSubAllocated) {
                FreeMemory(allocation);
            }
        }
        mAllocationsToDelete.ClearUpTo(completedSerial);
    }

    void ResourceAllocatorManager::DeallocateMemory(ResourceHeapAllocation& allocation) {
        if (allocation.GetInfo().mMethod == AllocationMethod::kInvalid) {
            return;
        }

        mAllocationsToDelete.Enqueue(allocation, mDevice->GetPendingCommandSerial());

        // Invalidate the allocation immediately in case one accidentally
        // calls DeallocateMemory again using the same allocation.
        allocation.Invalidate();
    }

    void ResourceAllocatorManager::FreeMemory(ResourceHeapAllocation& allocation) {
        ASSERT(allocation.GetInfo().mMethod == AllocationMethod::kSubAllocated);

        D3D12_HEAP_PROPERTIES heapProp;
        allocation.GetD3D12Resource()->GetHeapProperties(&heapProp, nullptr);

        const D3D12_RESOURCE_DESC resourceDescriptor = allocation.GetD3D12Resource()->GetDesc();

        const size_t resourceHeapKindIndex = GetResourceHeapKind(
            resourceDescriptor.Dimension, heapProp.Type, resourceDescriptor.Flags);

        mSubAllocatedResourceAllocators[resourceHeapKindIndex]->Deallocate(allocation);
    }

    ResultOrError<ResourceHeapAllocation> ResourceAllocatorManager::CreatePlacedResource(
        D3D12_HEAP_TYPE heapType,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage) {
        const size_t resourceHeapKindIndex =
            GetResourceHeapKind(resourceDescriptor.Dimension, heapType, resourceDescriptor.Flags);

        BuddyMemoryAllocator* allocator =
            mSubAllocatedResourceAllocators[resourceHeapKindIndex].get();

        const D3D12_RESOURCE_ALLOCATION_INFO resourceInfo =
            mDevice->GetD3D12Device()->GetResourceAllocationInfo(0, 1, &resourceDescriptor);

        ResourceMemoryAllocation allocation;
        DAWN_TRY_ASSIGN(allocation,
                        allocator->Allocate(resourceInfo.SizeInBytes, resourceInfo.Alignment));
        if (allocation.GetInfo().mMethod == AllocationMethod::kInvalid) {
            return ResourceHeapAllocation{};  // invalid
        }

        ID3D12Heap* heap = static_cast<Heap*>(allocation.GetResourceHeap())->GetD3D12Heap().Get();

        // With placed resources, a single heap can be reused.
        // The resource placed at an offset is only reclaimed
        // upon Tick or after the last command list using the resource has completed
        // on the GPU. This means the same physical memory is not reused
        // within the same command-list and does not require additional synchronization (aliasing
        // barrier).
        // https://docs.microsoft.com/en-us/windows/win32/api/d3d12/nf-d3d12-id3d12device-createplacedresource
        ComPtr<ID3D12Resource> placedResource;
        DAWN_TRY(CheckOutOfMemoryHRESULT(mDevice->GetD3D12Device()->CreatePlacedResource(
                                             heap, allocation.GetOffset(), &resourceDescriptor,
                                             initialUsage, nullptr, IID_PPV_ARGS(&placedResource)),
                                         "ID3D12Device::CreatePlacedResource"));

        return ResourceHeapAllocation{allocation.GetInfo(), allocation.GetOffset(),
                                      std::move(placedResource)};
    }

    ResultOrError<ResourceHeapAllocation> ResourceAllocatorManager::CreateCommittedResource(
        D3D12_HEAP_TYPE heapType,
        const D3D12_RESOURCE_DESC& resourceDescriptor,
        D3D12_RESOURCE_STATES initialUsage) {
        D3D12_HEAP_PROPERTIES heapProperties;
        heapProperties.Type = heapType;
        heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heapProperties.CreationNodeMask = 0;
        heapProperties.VisibleNodeMask = 0;

        // Note: Heap flags are inferred by the resource descriptor and do not need to be explicitly
        // provided to CreateCommittedResource.
        ComPtr<ID3D12Resource> committedResource;
        DAWN_TRY(
            CheckOutOfMemoryHRESULT(mDevice->GetD3D12Device()->CreateCommittedResource(
                                        &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDescriptor,
                                        initialUsage, nullptr, IID_PPV_ARGS(&committedResource)),
                                    "ID3D12Device::CreateCommittedResource"));

        AllocationInfo info;
        info.mMethod = AllocationMethod::kDirect;

        return ResourceHeapAllocation{info,
                                      /*offset*/ 0, std::move(committedResource)};
    }

}}  // namespace dawn_native::d3d12
