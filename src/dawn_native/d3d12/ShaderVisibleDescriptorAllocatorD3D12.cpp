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

#include "dawn_native/d3d12/ShaderVisibleDescriptorAllocatorD3D12.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"
#include "dawn_native/d3d12/GPUDescriptorHeapAllocationD3D12.h"
#include "dawn_native/d3d12/ResidencyManagerD3D12.h"

namespace dawn_native { namespace d3d12 {

    // Limits the min/max heap size to always be some known value for testing.
    // Thresholds should be adjusted (lower == faster) to avoid tests taking too long to complete.
    static constexpr const uint32_t kShaderVisibleSmallHeapSizes[] = {1024, 512};

    uint32_t GetD3D12ShaderVisibleHeapMinSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                                              bool useSmallSize) {
        if (useSmallSize) {
            return kShaderVisibleSmallHeapSizes[heapType];
        }

        // Minimum heap size must be large enough to satisfy the largest descriptor allocation
        // request and to amortize the cost of sub-allocation. But small enough to avoid wasting
        // memory should only a tiny fraction ever be used.
        // TODO(dawn:155): Figure out these values.
        switch (heapType) {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
                return 4096;
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                return 256;
            default:
                UNREACHABLE();
        }
    }

    uint32_t GetD3D12ShaderVisibleHeapMaxSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType,
                                              bool useSmallSize) {
        if (useSmallSize) {
            return kShaderVisibleSmallHeapSizes[heapType];
        }

        switch (heapType) {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
                return D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1;
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                return D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE;
            default:
                UNREACHABLE();
        }
    }

    D3D12_DESCRIPTOR_HEAP_FLAGS GetD3D12HeapFlags(D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
        switch (heapType) {
            case D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV:
            case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:
                return D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
            default:
                UNREACHABLE();
        }
    }

    // static
    ResultOrError<std::unique_ptr<ShaderVisibleDescriptorAllocator>>
    ShaderVisibleDescriptorAllocator::Create(Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
        std::unique_ptr<ShaderVisibleDescriptorAllocator> allocator =
            std::make_unique<ShaderVisibleDescriptorAllocator>(device, heapType);
        DAWN_TRY(allocator->AllocateAndSwitchShaderVisibleHeap());
        return std::move(allocator);
    }

    ShaderVisibleDescriptorAllocator::ShaderVisibleDescriptorAllocator(
        Device* device,
        D3D12_DESCRIPTOR_HEAP_TYPE heapType)
        : mHeapType(heapType),
          mDevice(device),
          mSizeIncrement(device->GetD3D12Device()->GetDescriptorHandleIncrementSize(heapType)),
          mDescriptorCount(GetD3D12ShaderVisibleHeapMinSize(
              heapType,
              mDevice->IsToggleEnabled(Toggle::UseD3D12SmallShaderVisibleHeapForTesting))) {
        ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
               heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
    }

    bool ShaderVisibleDescriptorAllocator::AllocateGPUDescriptors(
        uint32_t descriptorCount,
        ExecutionSerial pendingSerial,
        D3D12_CPU_DESCRIPTOR_HANDLE* baseCPUDescriptor,
        GPUDescriptorHeapAllocation* allocation) {
        ASSERT(mHeap != nullptr);
        const uint64_t startOffset = mAllocator.Allocate(descriptorCount, pendingSerial);
        if (startOffset == RingBufferAllocator::kInvalidOffset) {
            return false;
        }

        ID3D12DescriptorHeap* descriptorHeap = mHeap->GetD3D12DescriptorHeap();

        const uint64_t heapOffset = mSizeIncrement * startOffset;

        // Check for 32-bit overflow since CPU heap start handle uses size_t.
        const size_t cpuHeapStartPtr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr;

        ASSERT(heapOffset <= std::numeric_limits<size_t>::max() - cpuHeapStartPtr);

        *baseCPUDescriptor = {cpuHeapStartPtr + static_cast<size_t>(heapOffset)};

        const D3D12_GPU_DESCRIPTOR_HANDLE baseGPUDescriptor = {
            descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + heapOffset};

        // Record both the device and heap serials to determine later if the allocations are
        // still valid.
        *allocation = GPUDescriptorHeapAllocation{baseGPUDescriptor, pendingSerial, mHeapSerial};

        return true;
    }

    ID3D12DescriptorHeap* ShaderVisibleDescriptorAllocator::GetShaderVisibleHeap() const {
        return mHeap->GetD3D12DescriptorHeap();
    }

    void ShaderVisibleDescriptorAllocator::Tick(ExecutionSerial completedSerial) {
        mAllocator.Deallocate(completedSerial);
    }

    ResultOrError<std::unique_ptr<ShaderVisibleDescriptorHeap>>
    ShaderVisibleDescriptorAllocator::AllocateHeap(uint32_t descriptorCount) const {
        // The size in bytes of a descriptor heap is best calculated by the increment size
        // multiplied by the number of descriptors. In practice, this is only an estimate and
        // the actual size may vary depending on the driver.
        const uint64_t kSize = mSizeIncrement * descriptorCount;

        DAWN_TRY(mDevice->GetResidencyManager()->EnsureCanAllocate(kSize, MemorySegment::Local));

        ComPtr<ID3D12DescriptorHeap> d3d12DescriptorHeap;
        D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
        heapDescriptor.Type = mHeapType;
        heapDescriptor.NumDescriptors = descriptorCount;
        heapDescriptor.Flags = GetD3D12HeapFlags(mHeapType);
        heapDescriptor.NodeMask = 0;
        DAWN_TRY(CheckOutOfMemoryHRESULT(mDevice->GetD3D12Device()->CreateDescriptorHeap(
                                             &heapDescriptor, IID_PPV_ARGS(&d3d12DescriptorHeap)),
                                         "ID3D12Device::CreateDescriptorHeap"));

        std::unique_ptr<ShaderVisibleDescriptorHeap> descriptorHeap =
            std::make_unique<ShaderVisibleDescriptorHeap>(std::move(d3d12DescriptorHeap), kSize);

        // We must track the allocation in the LRU when it is created, otherwise the residency
        // manager will see the allocation as non-resident in the later call to LockAllocation.
        mDevice->GetResidencyManager()->TrackResidentAllocation(descriptorHeap.get());

        return std::move(descriptorHeap);
    }

    // Creates a GPU descriptor heap that manages descriptors in a FIFO queue.
    MaybeError ShaderVisibleDescriptorAllocator::AllocateAndSwitchShaderVisibleHeap() {
        std::unique_ptr<ShaderVisibleDescriptorHeap> descriptorHeap;
        // Dynamically allocate using a two-phase allocation strategy.
        // The first phase increasingly grows a small heap in binary sizes for light users while the
        // second phase pool-allocates largest sized heaps for heavy users.
        if (mHeap != nullptr) {
            mDevice->GetResidencyManager()->UnlockAllocation(mHeap.get());

            const uint32_t maxDescriptorCount = GetD3D12ShaderVisibleHeapMaxSize(
                mHeapType,
                mDevice->IsToggleEnabled(Toggle::UseD3D12SmallShaderVisibleHeapForTesting));
            if (mDescriptorCount < maxDescriptorCount) {
                // Phase #1. Grow the heaps in powers-of-two.
                mDevice->ReferenceUntilUnused(mHeap->GetD3D12DescriptorHeap());
                mDescriptorCount = std::min(mDescriptorCount * 2, maxDescriptorCount);
            } else {
                // Phase #2. Pool-allocate heaps.
                // Return the switched out heap to the pool and retrieve the oldest heap that is no
                // longer used by GPU. This maintains a heap buffer to avoid frequently re-creating
                // heaps for heavy users.
                // TODO(dawn:256): Consider periodically triming to avoid OOM.
                mPool.push_back({mDevice->GetPendingCommandSerial(), std::move(mHeap)});
                if (mPool.front().heapSerial <= mDevice->GetCompletedCommandSerial()) {
                    descriptorHeap = std::move(mPool.front().heap);
                    mPool.pop_front();
                }
            }
        }

        if (descriptorHeap == nullptr) {
            DAWN_TRY_ASSIGN(descriptorHeap, AllocateHeap(mDescriptorCount));
        }

        DAWN_TRY(mDevice->GetResidencyManager()->LockAllocation(descriptorHeap.get()));

        // Create a FIFO buffer from the recently created heap.
        mHeap = std::move(descriptorHeap);
        mAllocator = RingBufferAllocator(mDescriptorCount);

        // Invalidate all bindgroup allocations on previously bound heaps by incrementing the heap
        // serial. When a bindgroup attempts to re-populate, it will compare with its recorded
        // heap serial.
        mHeapSerial++;

        return {};
    }

    HeapVersionID ShaderVisibleDescriptorAllocator::GetShaderVisibleHeapSerialForTesting() const {
        return mHeapSerial;
    }

    uint64_t ShaderVisibleDescriptorAllocator::GetShaderVisibleHeapSizeForTesting() const {
        return mAllocator.GetSize();
    }

    uint64_t ShaderVisibleDescriptorAllocator::GetShaderVisiblePoolSizeForTesting() const {
        return mPool.size();
    }

    bool ShaderVisibleDescriptorAllocator::IsShaderVisibleHeapLockedResidentForTesting() const {
        return mHeap->IsResidencyLocked();
    }

    bool ShaderVisibleDescriptorAllocator::IsLastShaderVisibleHeapInLRUForTesting() const {
        ASSERT(!mPool.empty());
        return mPool.back().heap->IsInResidencyLRUCache();
    }

    bool ShaderVisibleDescriptorAllocator::IsAllocationStillValid(
        const GPUDescriptorHeapAllocation& allocation) const {
        // Consider valid if allocated for the pending submit and the shader visible heaps
        // have not switched over.
        return (allocation.GetLastUsageSerial() > mDevice->GetCompletedCommandSerial() &&
                allocation.GetHeapSerial() == mHeapSerial);
    }

    ShaderVisibleDescriptorHeap::ShaderVisibleDescriptorHeap(
        ComPtr<ID3D12DescriptorHeap> d3d12DescriptorHeap,
        uint64_t size)
        : Pageable(d3d12DescriptorHeap, MemorySegment::Local, size),
          mD3d12DescriptorHeap(std::move(d3d12DescriptorHeap)) {
    }

    ID3D12DescriptorHeap* ShaderVisibleDescriptorHeap::GetD3D12DescriptorHeap() const {
        return mD3d12DescriptorHeap.Get();
    }
}}  // namespace dawn_native::d3d12
