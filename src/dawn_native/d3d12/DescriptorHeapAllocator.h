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

#ifndef DAWNNATIVE_D3D12_DESCRIPTORHEAPALLOCATOR_H_
#define DAWNNATIVE_D3D12_DESCRIPTORHEAPALLOCATOR_H_

#include "dawn_native/d3d12/d3d12_platform.h"

#include <array>
#include <vector>
#include "common/SerialQueue.h"

#include "dawn_native/Error.h"
#include "dawn_native/RingBufferAllocator.h"

namespace dawn_native { namespace d3d12 {

    class Device;

    class DescriptorHeapHandle {
      public:
        DescriptorHeapHandle();
        DescriptorHeapHandle(ComPtr<ID3D12DescriptorHeap> descriptorHeap,
                             uint32_t sizeIncrement,
                             uint64_t offset);

        ID3D12DescriptorHeap* Get() const;
        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t index) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t index) const;

      private:
        ComPtr<ID3D12DescriptorHeap> mDescriptorHeap;
        uint32_t mSizeIncrement;
        uint64_t mOffset;
    };

    class DescriptorHeapAllocator {
      public:
        DescriptorHeapAllocator(Device* device);

        ResultOrError<DescriptorHeapHandle> AllocateGPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                            uint32_t count);
        ResultOrError<DescriptorHeapHandle> AllocateCPUHeap(D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                            uint32_t count);
        void Deallocate(uint64_t lastCompletedSerial);

      private:
        struct DescriptorHeapInfo {
            ComPtr<ID3D12DescriptorHeap> heap;
            RingBufferAllocator allocator;
        };

        ResultOrError<DescriptorHeapHandle> Allocate(D3D12_DESCRIPTOR_HEAP_TYPE type,
                                                     uint32_t count,
                                                     uint32_t allocationSize,
                                                     DescriptorHeapInfo* heapInfo,
                                                     D3D12_DESCRIPTOR_HEAP_FLAGS flags);

        Device* mDevice;

        std::array<uint32_t, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES> mSizeIncrements;
        std::array<DescriptorHeapInfo, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES>
            mCpuDescriptorHeapInfos;
        std::array<DescriptorHeapInfo, D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES>
            mGpuDescriptorHeapInfos;
    };

}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_DESCRIPTORHEAPALLOCATOR_H_
