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

#ifndef SRC_DAWN_NATIVE_D3D12_COMPUTEPIPELINED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_COMPUTEPIPELINED3D12_H_

#include "dawn/native/ComputePipeline.h"

#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

class ComputePipeline final : public ComputePipelineBase {
  public:
    static Ref<ComputePipeline> CreateUninitialized(Device* device,
                                                    const ComputePipelineDescriptor* descriptor);
    static void InitializeAsync(Ref<ComputePipelineBase> computePipeline,
                                WGPUCreateComputePipelineAsyncCallback callback,
                                void* userdata);
    ComputePipeline() = delete;

    ID3D12PipelineState* GetPipelineState() const;

    MaybeError Initialize() override;

    // Dawn API
    void SetLabelImpl() override;

    bool UsesNumWorkgroups() const;

    ComPtr<ID3D12CommandSignature> GetDispatchIndirectCommandSignature();

  private:
    ~ComputePipeline() override;

    void DestroyImpl() override;

    using ComputePipelineBase::ComputePipelineBase;
    ComPtr<ID3D12PipelineState> mPipelineState;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_COMPUTEPIPELINED3D12_H_
