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

#ifndef SRC_DAWN_NATIVE_D3D12_RENDERPIPELINED3D12_H_
#define SRC_DAWN_NATIVE_D3D12_RENDERPIPELINED3D12_H_

#include "dawn/native/RenderPipeline.h"

#include "dawn/native/d3d12/ShaderModuleD3D12.h"
#include "dawn/native/d3d12/d3d12_platform.h"

namespace dawn::native::d3d12 {

class Device;

class RenderPipeline final : public RenderPipelineBase {
  public:
    static Ref<RenderPipeline> CreateUninitialized(Device* device,
                                                   const RenderPipelineDescriptor* descriptor);
    static void InitializeAsync(Ref<RenderPipelineBase> renderPipeline,
                                WGPUCreateRenderPipelineAsyncCallback callback,
                                void* userdata);
    RenderPipeline() = delete;

    MaybeError Initialize() override;

    D3D12_PRIMITIVE_TOPOLOGY GetD3D12PrimitiveTopology() const;
    ID3D12PipelineState* GetPipelineState() const;

    bool UsesVertexOrInstanceIndex() const;

    // Dawn API
    void SetLabelImpl() override;

    ComPtr<ID3D12CommandSignature> GetDrawIndirectCommandSignature();

    ComPtr<ID3D12CommandSignature> GetDrawIndexedIndirectCommandSignature();

  private:
    ~RenderPipeline() override;

    void DestroyImpl() override;

    using RenderPipelineBase::RenderPipelineBase;
    D3D12_INPUT_LAYOUT_DESC ComputeInputLayout(
        std::array<D3D12_INPUT_ELEMENT_DESC, kMaxVertexAttributes>* inputElementDescriptors);

    D3D12_PRIMITIVE_TOPOLOGY mD3d12PrimitiveTopology;
    ComPtr<ID3D12PipelineState> mPipelineState;
    bool mUsesVertexOrInstanceIndex;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_RENDERPIPELINED3D12_H_
