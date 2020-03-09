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

#ifndef DAWNNATIVE_D3D12_SHADERVISIBLEDESCRIPTORALLOCATOR_H_
#define DAWNNATIVE_D3D12_SHADERVISIBLEDESCRIPTORALLOCATOR_H_

#include "dawn_native/Error.h"
#include "dawn_native/RingBufferAllocator.h"
#include "dawn_native/d3d12/DescriptorHeapAllocationD3D12.h"

#include <array>
#include <list>

namespace dawn_native { namespace d3d12 {

    class Device;

    // Manages descriptor heap allocators used by the device to create descriptors using allocation
    // methods based on the heap type.
    class ShaderVisibleDescriptorAllocator {
      public:
        ShaderVisibleDescriptorAllocator(Device* device);
        MaybeError Initialize();

        ResultOrError<DescriptorHeapAllocation> AllocateGPUDescriptors(
            uint32_t descriptorCount,
            Serial pendingSerial,
            D3D12_DESCRIPTOR_HEAP_TYPE heapType);

        void Tick(uint64_t completedSerial);
        Serial GetShaderVisibleHeapsSerial() const;

        std::array<ID3D12DescriptorHeap*, 2> GetShaderVisibleHeaps() const;
        MaybeError AllocateAndSwitchShaderVisibleHeaps();

        uint64_t GetShaderVisibleHeapSizeForTesting(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const;
        ComPtr<ID3D12DescriptorHeap> GetShaderVisibleHeapForTesting(
            D3D12_DESCRIPTOR_HEAP_TYPE heapType) const;
        uint64_t GetShaderVisiblePoolSizeForTesting(D3D12_DESCRIPTOR_HEAP_TYPE heapType) const;

        bool IsAllocationStillValid(Serial lastUsageSerial, Serial heapSerial) const;

      private:
        struct SerialDescriptorHeap {
            Serial heapSerial;
            ComPtr<ID3D12DescriptorHeap> heap;
        };

        struct ShaderVisibleBuffer {
            ComPtr<ID3D12DescriptorHeap> heap;
            RingBufferAllocator allocator;
            std::list<SerialDescriptorHeap> pool;
            D3D12_DESCRIPTOR_HEAP_TYPE heapType;
        };

        MaybeError AllocateGPUHeap(ShaderVisibleBuffer* shaderVisibleBuffer);

        Device* mDevice;

        // The serial value of 0 means the shader-visible heaps have not been allocated.
        // This value is never returned by GetShaderVisibleHeapsSerial() after Initialize().
        Serial mShaderVisibleHeapsSerial = 0;

        std::array<ShaderVisibleBuffer, 2> mShaderVisibleBuffers;
        std::array<uint32_t, 2> mSizeIncrements;
    };
}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_SHADERVISIBLEDESCRIPTORALLOCATOR_H_