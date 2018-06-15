// Copyright 2017 The NXT Authors
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

#include "backend/Device.h"

#include "backend/BindGroup.h"
#include "backend/BindGroupLayout.h"
#include "backend/BlendState.h"
#include "backend/Buffer.h"
#include "backend/CommandBuffer.h"
#include "backend/ComputePipeline.h"
#include "backend/DepthStencilState.h"
#include "backend/ErrorData.h"
#include "backend/InputState.h"
#include "backend/PipelineLayout.h"
#include "backend/Queue.h"
#include "backend/RenderPassDescriptor.h"
#include "backend/RenderPipeline.h"
#include "backend/Sampler.h"
#include "backend/ShaderModule.h"
#include "backend/SwapChain.h"
#include "backend/Texture.h"

#include <unordered_set>

namespace backend {

    // DeviceBase::Caches

    // The caches are unordered_sets of pointers with special hash and compare functions
    // to compare the value of the objects, instead of the pointers.
    using BindGroupLayoutCache = std::
        unordered_set<BindGroupLayoutBase*, BindGroupLayoutCacheFuncs, BindGroupLayoutCacheFuncs>;

    struct DeviceBase::Caches {
        BindGroupLayoutCache bindGroupLayouts;
    };

    // DeviceBase

    DeviceBase::DeviceBase() {
        mCaches = new DeviceBase::Caches();
    }

    DeviceBase::~DeviceBase() {
        delete mCaches;
    }

    void DeviceBase::HandleError(const char* message) {
        if (mErrorCallback) {
            mErrorCallback(message, mErrorUserdata);
        }
    }

    void DeviceBase::SetErrorCallback(nxt::DeviceErrorCallback callback,
                                      nxt::CallbackUserdata userdata) {
        mErrorCallback = callback;
        mErrorUserdata = userdata;
    }

    DeviceBase* DeviceBase::GetDevice() {
        return this;
    }

    BindGroupLayoutBase* DeviceBase::GetOrCreateBindGroupLayout(
        const BindGroupLayoutBase* blueprint,
        BindGroupLayoutBuilder* builder) {
        // The blueprint is only used to search in the cache and is not modified. However cached
        // objects can be modified, and unordered_set cannot search for a const pointer in a non
        // const pointer set. That's why we do a const_cast here, but the blueprint won't be
        // modified.
        auto iter = mCaches->bindGroupLayouts.find(const_cast<BindGroupLayoutBase*>(blueprint));
        if (iter != mCaches->bindGroupLayouts.end()) {
            (*iter)->Reference();
            return *iter;
        }

        BindGroupLayoutBase* backendObj = CreateBindGroupLayout(builder);
        mCaches->bindGroupLayouts.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheBindGroupLayout(BindGroupLayoutBase* obj) {
        mCaches->bindGroupLayouts.erase(obj);
    }

    BindGroupBuilder* DeviceBase::CreateBindGroupBuilder() {
        return new BindGroupBuilder(this);
    }
    BindGroupLayoutBuilder* DeviceBase::CreateBindGroupLayoutBuilder() {
        return new BindGroupLayoutBuilder(this);
    }
    BlendStateBuilder* DeviceBase::CreateBlendStateBuilder() {
        return new BlendStateBuilder(this);
    }
    BufferBuilder* DeviceBase::CreateBufferBuilder() {
        return new BufferBuilder(this);
    }
    CommandBufferBuilder* DeviceBase::CreateCommandBufferBuilder() {
        return new CommandBufferBuilder(this);
    }
    ComputePipelineBuilder* DeviceBase::CreateComputePipelineBuilder() {
        return new ComputePipelineBuilder(this);
    }
    DepthStencilStateBuilder* DeviceBase::CreateDepthStencilStateBuilder() {
        return new DepthStencilStateBuilder(this);
    }
    InputStateBuilder* DeviceBase::CreateInputStateBuilder() {
        return new InputStateBuilder(this);
    }
    PipelineLayoutBuilder* DeviceBase::CreatePipelineLayoutBuilder() {
        return new PipelineLayoutBuilder(this);
    }
    QueueBase* DeviceBase::CreateQueue() {
        ResultOrError<QueueBase*> maybeQueue = CreateQueueImpl();
        if (maybeQueue.IsError()) {
            // TODO(cwallez@chromium.org): Implement the WebGPU error handling mechanism.
            delete maybeQueue.AcquireError();
            return nullptr;
        }
        return maybeQueue.AcquireSuccess();
    }
    RenderPassDescriptorBuilder* DeviceBase::CreateRenderPassDescriptorBuilder() {
        return new RenderPassDescriptorBuilder(this);
    }
    RenderPipelineBuilder* DeviceBase::CreateRenderPipelineBuilder() {
        return new RenderPipelineBuilder(this);
    }
    SamplerBase* DeviceBase::CreateSampler(const nxt::SamplerDescriptor* descriptor) {
        MaybeError validation = ValidateSamplerDescriptor(this, descriptor);
        if (validation.IsError()) {
            // TODO(cwallez@chromium.org): Implement the WebGPU error handling mechanism.
            delete validation.AcquireError();
            return nullptr;
        }

        ResultOrError<SamplerBase*> maybeSampler = CreateSamplerImpl(descriptor);
        if (maybeSampler.IsError()) {
            // TODO(cwallez@chromium.org): Implement the WebGPU error handling mechanism.
            delete maybeSampler.AcquireError();
            return nullptr;
        }
        return maybeSampler.AcquireSuccess();
    }
    ShaderModuleBuilder* DeviceBase::CreateShaderModuleBuilder() {
        return new ShaderModuleBuilder(this);
    }
    SwapChainBuilder* DeviceBase::CreateSwapChainBuilder() {
        return new SwapChainBuilder(this);
    }
    TextureBuilder* DeviceBase::CreateTextureBuilder() {
        return new TextureBuilder(this);
    }

    void DeviceBase::Tick() {
        TickImpl();
    }

    void DeviceBase::Reference() {
        ASSERT(mRefCount != 0);
        mRefCount++;
    }

    void DeviceBase::Release() {
        ASSERT(mRefCount != 0);
        mRefCount--;
        if (mRefCount == 0) {
            delete this;
        }
    }

}  // namespace backend
