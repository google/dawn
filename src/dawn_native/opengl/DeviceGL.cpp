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

#include "dawn_native/opengl/DeviceGL.h"

#include "dawn_native/BindGroup.h"
#include "dawn_native/BindGroupLayout.h"
#include "dawn_native/OpenGLBackend.h"
#include "dawn_native/RenderPassDescriptor.h"
#include "dawn_native/opengl/BlendStateGL.h"
#include "dawn_native/opengl/BufferGL.h"
#include "dawn_native/opengl/CommandBufferGL.h"
#include "dawn_native/opengl/ComputePipelineGL.h"
#include "dawn_native/opengl/DepthStencilStateGL.h"
#include "dawn_native/opengl/InputStateGL.h"
#include "dawn_native/opengl/PipelineLayoutGL.h"
#include "dawn_native/opengl/QueueGL.h"
#include "dawn_native/opengl/RenderPipelineGL.h"
#include "dawn_native/opengl/SamplerGL.h"
#include "dawn_native/opengl/ShaderModuleGL.h"
#include "dawn_native/opengl/SwapChainGL.h"
#include "dawn_native/opengl/TextureGL.h"

namespace dawn_native { namespace opengl {

    dawnDevice CreateDevice(void* (*getProc)(const char*)) {
        gladLoadGLLoader(reinterpret_cast<GLADloadproc>(getProc));

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_SCISSOR_TEST);
        glEnable(GL_PRIMITIVE_RESTART_FIXED_INDEX);

        return reinterpret_cast<dawnDevice>(new Device);
    }

    // Device

    Device::Device() {
        CollectPCIInfo();
    }

    Device::~Device() {
        CheckPassedFences();
        ASSERT(mFencesInFlight.empty());

        // Some operations might have been started since the last submit and waiting
        // on a serial that doesn't have a corresponding fence enqueued. Force all
        // operations to look as if they were completed (because they were).
        mCompletedSerial = mLastSubmittedSerial + 1;
        Tick();
    }

    ResultOrError<BindGroupBase*> Device::CreateBindGroupImpl(
        const BindGroupDescriptor* descriptor) {
        return new BindGroup(this, descriptor);
    }
    ResultOrError<BindGroupLayoutBase*> Device::CreateBindGroupLayoutImpl(
        const BindGroupLayoutDescriptor* descriptor) {
        return new BindGroupLayout(this, descriptor);
    }
    BlendStateBase* Device::CreateBlendState(BlendStateBuilder* builder) {
        return new BlendState(builder);
    }
    ResultOrError<BufferBase*> Device::CreateBufferImpl(const BufferDescriptor* descriptor) {
        return new Buffer(this, descriptor);
    }
    CommandBufferBase* Device::CreateCommandBuffer(CommandBufferBuilder* builder) {
        return new CommandBuffer(builder);
    }
    ResultOrError<ComputePipelineBase*> Device::CreateComputePipelineImpl(
        const ComputePipelineDescriptor* descriptor) {
        return new ComputePipeline(this, descriptor);
    }
    DepthStencilStateBase* Device::CreateDepthStencilState(DepthStencilStateBuilder* builder) {
        return new DepthStencilState(builder);
    }
    InputStateBase* Device::CreateInputState(InputStateBuilder* builder) {
        return new InputState(builder);
    }
    ResultOrError<PipelineLayoutBase*> Device::CreatePipelineLayoutImpl(
        const PipelineLayoutDescriptor* descriptor) {
        return new PipelineLayout(this, descriptor);
    }
    ResultOrError<QueueBase*> Device::CreateQueueImpl() {
        return new Queue(this);
    }
    RenderPassDescriptorBase* Device::CreateRenderPassDescriptor(
        RenderPassDescriptorBuilder* builder) {
        return new RenderPassDescriptor(builder);
    }
    ResultOrError<RenderPipelineBase*> Device::CreateRenderPipelineImpl(
        const RenderPipelineDescriptor* descriptor) {
        return new RenderPipeline(this, descriptor);
    }
    ResultOrError<SamplerBase*> Device::CreateSamplerImpl(const SamplerDescriptor* descriptor) {
        return new Sampler(this, descriptor);
    }
    ResultOrError<ShaderModuleBase*> Device::CreateShaderModuleImpl(
        const ShaderModuleDescriptor* descriptor) {
        return new ShaderModule(this, descriptor);
    }
    SwapChainBase* Device::CreateSwapChain(SwapChainBuilder* builder) {
        return new SwapChain(builder);
    }
    ResultOrError<TextureBase*> Device::CreateTextureImpl(const TextureDescriptor* descriptor) {
        return new Texture(this, descriptor);
    }
    ResultOrError<TextureViewBase*> Device::CreateTextureViewImpl(
        TextureBase* texture,
        const TextureViewDescriptor* descriptor) {
        return new TextureView(texture, descriptor);
    }

    void Device::SubmitFenceSync() {
        GLsync sync = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
        mLastSubmittedSerial++;
        mFencesInFlight.emplace(sync, mLastSubmittedSerial);
    }

    Serial Device::GetCompletedCommandSerial() const {
        return mCompletedSerial;
    }

    Serial Device::GetLastSubmittedCommandSerial() const {
        return mLastSubmittedSerial;
    }

    void Device::TickImpl() {
        CheckPassedFences();
    }

    void Device::CheckPassedFences() {
        while (!mFencesInFlight.empty()) {
            GLsync sync = mFencesInFlight.front().first;
            Serial fenceSerial = mFencesInFlight.front().second;

            GLint status = 0;
            GLsizei length;
            glGetSynciv(sync, GL_SYNC_CONDITION, sizeof(GLint), &length, &status);
            ASSERT(length == 1);

            // Fence are added in order, so we can stop searching as soon
            // as we see one that's not ready.
            if (!status) {
                return;
            }

            glDeleteSync(sync);

            mFencesInFlight.pop();

            ASSERT(fenceSerial > mCompletedSerial);
            mCompletedSerial = fenceSerial;
        }
    }

    const dawn_native::PCIInfo& Device::GetPCIInfo() const {
        return mPCIInfo;
    }

    void Device::CollectPCIInfo() {
        mPCIInfo.name = reinterpret_cast<const char*>(glGetString(GL_RENDERER));
    }

}}  // namespace dawn_native::opengl
