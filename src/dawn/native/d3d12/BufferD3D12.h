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

#ifndef SRC_DAWN_NATIVE_D3D12_BUFFERD3D12_H_
#define SRC_DAWN_NATIVE_D3D12_BUFFERD3D12_H_

#include <limits>

#include "dawn/native/Buffer.h"

#include "dawn/native/d3d12/ResourceHeapAllocationD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class CommandRecordingContext;
class Device;

class Buffer final : public BufferBase {
  public:
    static ResultOrError<Ref<Buffer>> Create(Device* device, const BufferDescriptor* descriptor);

    ID3D12Resource* GetD3D12Resource() const;
    D3D12_GPU_VIRTUAL_ADDRESS GetVA() const;

    bool TrackUsageAndGetResourceBarrier(CommandRecordingContext* commandContext,
                                         D3D12_RESOURCE_BARRIER* barrier,
                                         wgpu::BufferUsage newUsage);
    void TrackUsageAndTransitionNow(CommandRecordingContext* commandContext,
                                    wgpu::BufferUsage newUsage);

    bool CheckAllocationMethodForTesting(AllocationMethod allocationMethod) const;
    bool CheckIsResidentForTesting() const;

    MaybeError EnsureDataInitialized(CommandRecordingContext* commandContext);
    ResultOrError<bool> EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                           uint64_t offset,
                                                           uint64_t size);
    MaybeError EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                  const CopyTextureToBufferCmd* copy);

    // Dawn API
    void SetLabelImpl() override;

  private:
    Buffer(Device* device, const BufferDescriptor* descriptor);
    ~Buffer() override;

    MaybeError Initialize(bool mappedAtCreation);
    MaybeError MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) override;
    void UnmapImpl() override;
    void DestroyImpl() override;
    bool IsCPUWritableAtCreation() const override;
    MaybeError MapAtCreationImpl() override;
    void* GetMappedPointer() override;

    MaybeError MapInternal(bool isWrite, size_t start, size_t end, const char* contextInfo);

    MaybeError InitializeToZero(CommandRecordingContext* commandContext);
    MaybeError ClearBuffer(CommandRecordingContext* commandContext,
                           uint8_t clearValue,
                           uint64_t offset = 0,
                           uint64_t size = 0);

    ResourceHeapAllocation mResourceAllocation;
    bool mFixedResourceState = false;
    wgpu::BufferUsage mLastUsage = wgpu::BufferUsage::None;
    ExecutionSerial mLastUsedSerial = std::numeric_limits<ExecutionSerial>::max();

    D3D12_RANGE mWrittenMappedRange = {0, 0};
    void* mMappedData = nullptr;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_BUFFERD3D12_H_
