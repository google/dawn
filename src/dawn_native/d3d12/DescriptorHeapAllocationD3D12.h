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

#ifndef DAWNNATIVE_D3D12_DESCRIPTORHEAPALLOCATIOND3D12_H_
#define DAWNNATIVE_D3D12_DESCRIPTORHEAPALLOCATIOND3D12_H_

#include "dawn_native/d3d12/d3d12_platform.h"

#include <cstdint>

namespace dawn_native { namespace d3d12 {

    // Wrapper for a handle into a descriptor heap.
    class DescriptorHeapAllocation {
      public:
        DescriptorHeapAllocation();
        DescriptorHeapAllocation(uint32_t sizeIncrement,
                                 D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptorHandle,
                                 D3D12_GPU_DESCRIPTOR_HANDLE baseGPUDescriptorHandle);
        ~DescriptorHeapAllocation() = default;

        D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t offset) const;
        D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t offset) const;

        bool IsInvalid() const;

      private:
        uint32_t mSizeIncrement;

        D3D12_CPU_DESCRIPTOR_HANDLE mBaseCPUDescriptorHandle = {0};
        D3D12_GPU_DESCRIPTOR_HANDLE mBaseGPUDescriptorHandle = {0};
    };
}}  // namespace dawn_native::d3d12

#endif  // DAWNNATIVE_D3D12_DESCRIPTORHEAPALLOCATIOND3D12_H_