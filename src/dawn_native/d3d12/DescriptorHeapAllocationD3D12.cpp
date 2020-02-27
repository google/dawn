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

#include "dawn_native/d3d12/DescriptorHeapAllocationD3D12.h"
#include "dawn_native/Error.h"

namespace dawn_native { namespace d3d12 {

    DescriptorHeapAllocation::DescriptorHeapAllocation() : mSizeIncrement(0) {
    }

    DescriptorHeapAllocation::DescriptorHeapAllocation(
        uint32_t sizeIncrement,
        D3D12_CPU_DESCRIPTOR_HANDLE baseCPUDescriptorHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE baseGPUDescriptorHandle)
        : mSizeIncrement(sizeIncrement),
          mBaseCPUDescriptorHandle(baseCPUDescriptorHandle),
          mBaseGPUDescriptorHandle(baseGPUDescriptorHandle) {
    }

    D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetCPUHandle(uint32_t offset) const {
        ASSERT(!IsInvalid());
        D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = mBaseCPUDescriptorHandle;
        cpuHandle.ptr += mSizeIncrement * offset;
        return cpuHandle;
    }

    D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetGPUHandle(uint32_t offset) const {
        ASSERT(!IsInvalid());
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = mBaseGPUDescriptorHandle;
        gpuHandle.ptr += mSizeIncrement * offset;
        return gpuHandle;
    }

    bool DescriptorHeapAllocation::IsInvalid() const {
        return mBaseCPUDescriptorHandle.ptr == 0;
    }
}}  // namespace dawn_native::d3d12