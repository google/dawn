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

namespace dawn_native { namespace d3d12 {

    // Check that d3d heap type enum correctly mirrors the type index used by the static arrays.
    static_assert(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV == 0, "");
    static_assert(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER == 1, "");

    // Thresholds should be adjusted (lower == faster) to avoid tests taking too long to complete.
    static constexpr const uint32_t kShaderVisibleSmallHeapSizes[] = {1024, 512};

    uint32_t GetD3D12ShaderVisibleHeapSize(D3D12_DESCRIPTOR_HEAP_TYPE heapType, bool useSmallSize) {
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

    ShaderVisibleDescriptorAllocator::ShaderVisibleDescriptorAllocator(Device* device)
        : mDevice(device),
          mSizeIncrements{
              device->GetD3D12Device()->GetDescriptorHandleIncrementSize(
                  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
              device->GetD3D12Device()->GetDescriptorHandleIncrementSize(
                  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
          } {
    }

    MaybeError ShaderVisibleDescriptorAllocator::Initialize() {
        ASSERT(mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap.Get() == nullptr);
        mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heapType =
            D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        ASSERT(mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].heap.Get() == nullptr);
        mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].heapType =
            D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;

        DAWN_TRY(AllocateAndSwitchShaderVisibleHeaps());

        return {};
    }

    MaybeError ShaderVisibleDescriptorAllocator::AllocateAndSwitchShaderVisibleHeaps() {
        DAWN_TRY(AllocateGPUHeap(&mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV]));
        DAWN_TRY(AllocateGPUHeap(&mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER]));

        // Invalidate all bindgroup allocations on previously bound heaps by incrementing the heap
        // serial. When a bindgroup attempts to re-populate, it will compare with its recorded
        // heap serial.
        mShaderVisibleHeapsSerial++;

        return {};
    }

    ResultOrError<DescriptorHeapAllocation>
    ShaderVisibleDescriptorAllocator::AllocateGPUDescriptors(uint32_t descriptorCount,
                                                             Serial pendingSerial,
                                                             D3D12_DESCRIPTOR_HEAP_TYPE heapType) {
        ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
               heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        ASSERT(mShaderVisibleBuffers[heapType].heap != nullptr);
        const uint64_t startOffset =
            mShaderVisibleBuffers[heapType].allocator.Allocate(descriptorCount, pendingSerial);
        if (startOffset == RingBufferAllocator::kInvalidOffset) {
            return DescriptorHeapAllocation{};  // Invalid
        }

        ID3D12DescriptorHeap* descriptorHeap = mShaderVisibleBuffers[heapType].heap.Get();

        D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptor =
            descriptorHeap->GetCPUDescriptorHandleForHeapStart();
        baseCPUDescriptor.ptr += mSizeIncrements[heapType] * startOffset;

        D3D12_GPU_DESCRIPTOR_HANDLE baseGPUDescriptor =
            descriptorHeap->GetGPUDescriptorHandleForHeapStart();
        baseGPUDescriptor.ptr += mSizeIncrements[heapType] * startOffset;

        return DescriptorHeapAllocation{mSizeIncrements[heapType], baseCPUDescriptor,
                                        baseGPUDescriptor};
    }

    std::array<ID3D12DescriptorHeap*, 2> ShaderVisibleDescriptorAllocator::GetShaderVisibleHeaps()
        const {
        return {mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV].heap.Get(),
                mShaderVisibleBuffers[D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER].heap.Get()};
    }

    void ShaderVisibleDescriptorAllocator::Tick(uint64_t completedSerial) {
        for (uint32_t i = 0; i < mShaderVisibleBuffers.size(); i++) {
            ASSERT(mShaderVisibleBuffers[i].heap != nullptr);
            mShaderVisibleBuffers[i].allocator.Deallocate(completedSerial);
        }
    }

    // Creates a GPU descriptor heap that manages descriptors in a FIFO queue.
    MaybeError ShaderVisibleDescriptorAllocator::AllocateGPUHeap(
        ShaderVisibleBuffer* shaderVisibleBuffer) {
        ComPtr<ID3D12DescriptorHeap> heap;
        // Return the switched out heap to the pool and retrieve the oldest heap that is no longer
        // used by GPU. This maintains a heap buffer to avoid frequently re-creating heaps for heavy
        // users.
        // TODO(dawn:256): Consider periodically triming to avoid OOM.
        if (shaderVisibleBuffer->heap != nullptr) {
            shaderVisibleBuffer->pool.push_back(
                {mDevice->GetPendingCommandSerial(), std::move(shaderVisibleBuffer->heap)});
        }

        // Recycle existing heap if possible.
        if (!shaderVisibleBuffer->pool.empty() &&
            shaderVisibleBuffer->pool.front().heapSerial <= mDevice->GetCompletedCommandSerial()) {
            heap = std::move(shaderVisibleBuffer->pool.front().heap);
            shaderVisibleBuffer->pool.pop_front();
        }

        const D3D12_DESCRIPTOR_HEAP_TYPE heapType = shaderVisibleBuffer->heapType;

        // TODO(bryan.bernhart@intel.com): Allocating to max heap size wastes memory
        // should the developer not allocate any bindings for the heap type.
        // Consider dynamically re-sizing GPU heaps.
        const uint32_t descriptorCount = GetD3D12ShaderVisibleHeapSize(
            heapType, mDevice->IsToggleEnabled(Toggle::UseD3D12SmallShaderVisibleHeapForTesting));

        if (heap == nullptr) {
            D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
            heapDescriptor.Type = heapType;
            heapDescriptor.NumDescriptors = descriptorCount;
            heapDescriptor.Flags = GetD3D12HeapFlags(heapType);
            heapDescriptor.NodeMask = 0;
            DAWN_TRY(CheckOutOfMemoryHRESULT(mDevice->GetD3D12Device()->CreateDescriptorHeap(
                                                 &heapDescriptor, IID_PPV_ARGS(&heap)),
                                             "ID3D12Device::CreateDescriptorHeap"));
        }

        // Create a FIFO buffer from the recently created heap.
        shaderVisibleBuffer->heap = std::move(heap);
        shaderVisibleBuffer->allocator = RingBufferAllocator(descriptorCount);
        return {};
    }

    Serial ShaderVisibleDescriptorAllocator::GetShaderVisibleHeapsSerial() const {
        return mShaderVisibleHeapsSerial;
    }

    uint64_t ShaderVisibleDescriptorAllocator::GetShaderVisibleHeapSizeForTesting(
        D3D12_DESCRIPTOR_HEAP_TYPE heapType) const {
        ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
               heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        return mShaderVisibleBuffers[heapType].allocator.GetSize();
    }

    ComPtr<ID3D12DescriptorHeap> ShaderVisibleDescriptorAllocator::GetShaderVisibleHeapForTesting(
        D3D12_DESCRIPTOR_HEAP_TYPE heapType) const {
        ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
               heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        return mShaderVisibleBuffers[heapType].heap;
    }

    uint64_t ShaderVisibleDescriptorAllocator::GetShaderVisiblePoolSizeForTesting(
        D3D12_DESCRIPTOR_HEAP_TYPE heapType) const {
        ASSERT(heapType == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
               heapType == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        return mShaderVisibleBuffers[heapType].pool.size();
    }

    bool ShaderVisibleDescriptorAllocator::IsAllocationStillValid(Serial lastUsageSerial,
                                                                  Serial heapSerial) const {
        // Consider valid if allocated for the pending submit and the shader visible heaps
        // have not switched over.
        return (lastUsageSerial > mDevice->GetCompletedCommandSerial() &&
                heapSerial == mShaderVisibleHeapsSerial);
    }
}}  // namespace dawn_native::d3d12