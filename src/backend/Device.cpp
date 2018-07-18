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

    void DeviceBase::SetErrorCallback(dawn::DeviceErrorCallback callback,
                                      dawn::CallbackUserdata userdata) {
        mErrorCallback = callback;
        mErrorUserdata = userdata;
    }

    DeviceBase* DeviceBase::GetDevice() {
        return this;
    }

    ResultOrError<BindGroupLayoutBase*> DeviceBase::GetOrCreateBindGroupLayout(
        const dawn::BindGroupLayoutDescriptor* descriptor) {
        BindGroupLayoutBase blueprint(this, descriptor, true);

        auto iter = mCaches->bindGroupLayouts.find(&blueprint);
        if (iter != mCaches->bindGroupLayouts.end()) {
            (*iter)->Reference();
            return *iter;
        }

        BindGroupLayoutBase* backendObj;
        DAWN_TRY_ASSIGN(backendObj, CreateBindGroupLayoutImpl(descriptor));
        mCaches->bindGroupLayouts.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheBindGroupLayout(BindGroupLayoutBase* obj) {
        mCaches->bindGroupLayouts.erase(obj);
    }

    // Object creation API methods

    BindGroupBuilder* DeviceBase::CreateBindGroupBuilder() {
        return new BindGroupBuilder(this);
    }
    BindGroupLayoutBase* DeviceBase::CreateBindGroupLayout(
        const dawn::BindGroupLayoutDescriptor* descriptor) {
        BindGroupLayoutBase* result = nullptr;

        if (ConsumedError(CreateBindGroupLayoutInternal(&result, descriptor))) {
            return nullptr;
        }

        return result;
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
    PipelineLayoutBase* DeviceBase::CreatePipelineLayout(
        const dawn::PipelineLayoutDescriptor* descriptor) {
        PipelineLayoutBase* result = nullptr;

        if (ConsumedError(CreatePipelineLayoutInternal(&result, descriptor))) {
            return nullptr;
        }

        return result;
    }
    QueueBase* DeviceBase::CreateQueue() {
        QueueBase* result = nullptr;

        if (ConsumedError(CreateQueueInternal(&result))) {
            return nullptr;
        }

        return result;
    }
    RenderPassDescriptorBuilder* DeviceBase::CreateRenderPassDescriptorBuilder() {
        return new RenderPassDescriptorBuilder(this);
    }
    RenderPipelineBuilder* DeviceBase::CreateRenderPipelineBuilder() {
        return new RenderPipelineBuilder(this);
    }
    SamplerBase* DeviceBase::CreateSampler(const dawn::SamplerDescriptor* descriptor) {
        SamplerBase* result = nullptr;

        if (ConsumedError(CreateSamplerInternal(&result, descriptor))) {
            return nullptr;
        }

        return result;
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

    // Other Device API methods

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

    // Implementation details of object creation

    MaybeError DeviceBase::CreateBindGroupLayoutInternal(
        BindGroupLayoutBase** result,
        const dawn::BindGroupLayoutDescriptor* descriptor) {
        DAWN_TRY(ValidateBindGroupLayoutDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, GetOrCreateBindGroupLayout(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreatePipelineLayoutInternal(
        PipelineLayoutBase** result,
        const dawn::PipelineLayoutDescriptor* descriptor) {
        DAWN_TRY(ValidatePipelineLayoutDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, CreatePipelineLayoutImpl(descriptor));
        return {};
    }

    MaybeError DeviceBase::CreateQueueInternal(QueueBase** result) {
        DAWN_TRY_ASSIGN(*result, CreateQueueImpl());
        return {};
    }

    MaybeError DeviceBase::CreateSamplerInternal(SamplerBase** result,
                                                 const dawn::SamplerDescriptor* descriptor) {
        DAWN_TRY(ValidateSamplerDescriptor(this, descriptor));
        DAWN_TRY_ASSIGN(*result, CreateSamplerImpl(descriptor));
        return {};
    }

    // Other implementation details

    void DeviceBase::ConsumeError(ErrorData* error) {
        ASSERT(error != nullptr);
        HandleError(error->GetMessage().c_str());
        delete error;
    }

}  // namespace backend
