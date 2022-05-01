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

#ifndef SRC_DAWN_NATIVE_D3D12_GPUDESCRIPTORHEAPALLOCATIOND3D12_H_
#define SRC_DAWN_NATIVE_D3D12_GPUDESCRIPTORHEAPALLOCATIOND3D12_H_

#include "dawn/native/IntegerTypes.h"
#include "dawn/native/d3d12/IntegerTypes.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

// Wrapper for a handle into a GPU-only descriptor heap.
class GPUDescriptorHeapAllocation {
  public:
    GPUDescriptorHeapAllocation() = default;
    GPUDescriptorHeapAllocation(D3D12_GPU_DESCRIPTOR_HANDLE baseDescriptor,
                                ExecutionSerial lastUsageSerial,
                                HeapVersionID heapSerial);

    D3D12_GPU_DESCRIPTOR_HANDLE GetBaseDescriptor() const;
    ExecutionSerial GetLastUsageSerial() const;
    HeapVersionID GetHeapSerial() const;

  private:
    D3D12_GPU_DESCRIPTOR_HANDLE mBaseDescriptor = {0};
    ExecutionSerial mLastUsageSerial = ExecutionSerial(0);
    HeapVersionID mHeapSerial = HeapVersionID(0);
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_GPUDESCRIPTORHEAPALLOCATIOND3D12_H_
