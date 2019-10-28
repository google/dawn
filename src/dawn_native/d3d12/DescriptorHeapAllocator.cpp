// Copyright 2017 The Dawn Authors
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

#include "dawn_native/d3d12/DescriptorHeapAllocator.h"

#include "common/Assert.h"
#include "dawn_native/d3d12/D3D12Error.h"
#include "dawn_native/d3d12/DeviceD3D12.h"

namespace dawn_native { namespace d3d12 {

    DescriptorHeapHandle::DescriptorHeapHandle()
        : mDescriptorHeap(nullptr), mSizeIncrement(0), mOffset(0) {
    }

    DescriptorHeapHandle::DescriptorHeapHandle(ComPtr<ID3D12DescriptorHeap> descriptorHeap,
                                               uint32_t sizeIncrement,
                                               uint64_t offset)
        : mDescriptorHeap(descriptorHeap), mSizeIncrement(sizeIncrement), mOffset(offset) {
    }

    ID3D12DescriptorHeap* DescriptorHeapHandle::Get() const {
        return mDescriptorHeap.Get();
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapHandle::GetCPUHandle(uint32_t index) const {
        ASSERT(mDescriptorHeap);
        auto handle = mDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
        handle.ptr += mSizeIncrement * (index + mOffset);
        return handle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapHandle::GetGPUHandle(uint32_t index) const {
        ASSERT(mDescriptorHeap);
        auto handle = mDescriptorHeap->GetGPUDescriptorHandleForHeapStart();
        handle.ptr += mSizeIncrement * (index + mOffset);
        return handle;
    }

    DescriptorHeapAllocator::DescriptorHeapAllocator(Device* device)
        : mDevice(device),
          mSizeIncrements{
              device->GetD3D12Device()->GetDescriptorHandleIncrementSize(
                  D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV),
              device->GetD3D12Device()->GetDescriptorHandleIncrementSize(
                  D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER),
              device->GetD3D12Device()->GetDescriptorHandleIncrementSize(
                  D3D12_DESCRIPTOR_HEAP_TYPE_RTV),
              device->GetD3D12Device()->GetDescriptorHandleIncrementSize(
                  D3D12_DESCRIPTOR_HEAP_TYPE_DSV),
          } {
    }

    ResultOrError<DescriptorHeapHandle> DescriptorHeapAllocator::Allocate(
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t count,
        uint32_t allocationSize,
        DescriptorHeapInfo* heapInfo,
        D3D12_DESCRIPTOR_HEAP_FLAGS flags) {
        const Serial pendingSerial = mDevice->GetPendingCommandSerial();
        uint64_t startOffset = (heapInfo->heap == nullptr)
                                   ? RingBufferAllocator::kInvalidOffset
                                   : heapInfo->allocator.Allocate(count, pendingSerial);
        if (startOffset != RingBufferAllocator::kInvalidOffset) {
            return DescriptorHeapHandle{heapInfo->heap, mSizeIncrements[type], startOffset};
        }

        // If the pool has no more space, replace the pool with a new one of the specified size

        D3D12_DESCRIPTOR_HEAP_DESC heapDescriptor;
        heapDescriptor.Type = type;
        heapDescriptor.NumDescriptors = allocationSize;
        heapDescriptor.Flags = flags;
        heapDescriptor.NodeMask = 0;
        ComPtr<ID3D12DescriptorHeap> heap;
        DAWN_TRY(CheckHRESULT(
            mDevice->GetD3D12Device()->CreateDescriptorHeap(&heapDescriptor, IID_PPV_ARGS(&heap)),
            "ID3D12Device::CreateDescriptorHeap"));

        mDevice->ReferenceUntilUnused(heap);

        *heapInfo = {heap, RingBufferAllocator(allocationSize)};

        startOffset = heapInfo->allocator.Allocate(count, pendingSerial);

        ASSERT(startOffset != RingBufferAllocator::kInvalidOffset);

        return DescriptorHeapHandle(heap, mSizeIncrements[type], startOffset);
    }

    ResultOrError<DescriptorHeapHandle> DescriptorHeapAllocator::AllocateCPUHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t count) {
        return Allocate(type, count, count, &mCpuDescriptorHeapInfos[type],
                        D3D12_DESCRIPTOR_HEAP_FLAG_NONE);
    }

    ResultOrError<DescriptorHeapHandle> DescriptorHeapAllocator::AllocateGPUHeap(
        D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t count) {
        ASSERT(type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ||
               type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER);
        unsigned int heapSize = (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                                     ? D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_1
                                     : D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE);
        return Allocate(type, count, heapSize, &mGpuDescriptorHeapInfos[type],
                        D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE);
    }

    void DescriptorHeapAllocator::Deallocate(uint64_t lastCompletedSerial) {
        for (uint32_t i = 0; i < mCpuDescriptorHeapInfos.size(); i++) {
            if (mCpuDescriptorHeapInfos[i].heap != nullptr) {
                mCpuDescriptorHeapInfos[i].allocator.Deallocate(lastCompletedSerial);
            }
        }

        for (uint32_t i = 0; i < mGpuDescriptorHeapInfos.size(); i++) {
            if (mGpuDescriptorHeapInfos[i].heap != nullptr) {
                mGpuDescriptorHeapInfos[i].allocator.Deallocate(lastCompletedSerial);
            }
        }
    }
}}  // namespace dawn_native::d3d12
