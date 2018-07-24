// Copyright 2018 The Dawn Authors
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

#ifndef BACKEND_OPENGL_DEVICEGL_H_
#define BACKEND_OPENGL_DEVICEGL_H_

#include "dawn/dawncpp.h"

#include "common/Platform.h"
#include "dawn_native/Device.h"
#include "dawn_native/opengl/Forward.h"

#include "glad/glad.h"

// Remove windows.h macros after glad's include of windows.h
#if defined(DAWN_PLATFORM_WINDOWS)
#    include "common/windows_with_undefs.h"
#endif

namespace backend { namespace opengl {

    class Device : public DeviceBase {
      public:
        BindGroupBase* CreateBindGroup(BindGroupBuilder* builder) override;
        BlendStateBase* CreateBlendState(BlendStateBuilder* builder) override;
        BufferBase* CreateBuffer(BufferBuilder* builder) override;
        BufferViewBase* CreateBufferView(BufferViewBuilder* builder) override;
        CommandBufferBase* CreateCommandBuffer(CommandBufferBuilder* builder) override;
        ComputePipelineBase* CreateComputePipeline(ComputePipelineBuilder* builder) override;
        DepthStencilStateBase* CreateDepthStencilState(DepthStencilStateBuilder* builder) override;
        InputStateBase* CreateInputState(InputStateBuilder* builder) override;
        RenderPassDescriptorBase* CreateRenderPassDescriptor(
            RenderPassDescriptorBuilder* builder) override;
        RenderPipelineBase* CreateRenderPipeline(RenderPipelineBuilder* builder) override;
        ShaderModuleBase* CreateShaderModule(ShaderModuleBuilder* builder) override;
        SwapChainBase* CreateSwapChain(SwapChainBuilder* builder) override;
        TextureBase* CreateTexture(TextureBuilder* builder) override;
        TextureViewBase* CreateTextureView(TextureViewBuilder* builder) override;

        void TickImpl() override;

      private:
        ResultOrError<BindGroupLayoutBase*> CreateBindGroupLayoutImpl(
            const dawn::BindGroupLayoutDescriptor* descriptor) override;
        ResultOrError<PipelineLayoutBase*> CreatePipelineLayoutImpl(
            const dawn::PipelineLayoutDescriptor* descriptor) override;
        ResultOrError<QueueBase*> CreateQueueImpl() override;
        ResultOrError<SamplerBase*> CreateSamplerImpl(
            const dawn::SamplerDescriptor* descriptor) override;
    };

}}  // namespace backend::opengl

#endif  // BACKEND_OPENGL_DEVICEGL_H_
