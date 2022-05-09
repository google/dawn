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

#ifndef SRC_DAWN_NATIVE_METAL_COMPUTEPIPELINEMTL_H_
#define SRC_DAWN_NATIVE_METAL_COMPUTEPIPELINEMTL_H_

#include <vector>

#include "dawn/native/ComputePipeline.h"

#include "dawn/common/NSRef.h"

#import <Metal/Metal.h>

namespace dawn::native::metal {

class Device;

class ComputePipeline final : public ComputePipelineBase {
  public:
    static Ref<ComputePipeline> CreateUninitialized(Device* device,
                                                    const ComputePipelineDescriptor* descriptor);
    static void InitializeAsync(Ref<ComputePipelineBase> computePipeline,
                                WGPUCreateComputePipelineAsyncCallback callback,
                                void* userdata);

    ComputePipeline(DeviceBase* device, const ComputePipelineDescriptor* descriptor);
    ~ComputePipeline() override;

    void Encode(id<MTLComputeCommandEncoder> encoder);
    MTLSize GetLocalWorkGroupSize() const;
    bool RequiresStorageBufferLength() const;

  private:
    using ComputePipelineBase::ComputePipelineBase;
    MaybeError Initialize() override;

    NSPRef<id<MTLComputePipelineState>> mMtlComputePipelineState;
    MTLSize mLocalWorkgroupSize;
    bool mRequiresStorageBufferLength;
    std::vector<uint32_t> mWorkgroupAllocations;
};

}  // namespace dawn::native::metal

#endif  // SRC_DAWN_NATIVE_METAL_COMPUTEPIPELINEMTL_H_
