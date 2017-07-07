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
#include "backend/Buffer.h"
#include "backend/CommandBuffer.h"
#include "backend/DepthStencilState.h"
#include "backend/Framebuffer.h"
#include "backend/InputState.h"
#include "backend/Pipeline.h"
#include "backend/PipelineLayout.h"
#include "backend/Queue.h"
#include "backend/RenderPass.h"
#include "backend/Sampler.h"
#include "backend/ShaderModule.h"
#include "backend/Texture.h"

#include <unordered_set>

namespace backend {

    // DeviceBase::Caches

    // The caches are unordered_sets of pointers with special hash and compare functions
    // to compare the value of the objects, instead of the pointers.
    using BindGroupLayoutCache = std::unordered_set<BindGroupLayoutBase*, BindGroupLayoutCacheFuncs, BindGroupLayoutCacheFuncs>;

    struct DeviceBase::Caches {
        BindGroupLayoutCache bindGroupLayouts;
    };

    // DeviceBase

    DeviceBase::DeviceBase() {
        caches = new DeviceBase::Caches();
    }

    DeviceBase::~DeviceBase() {
        delete caches;
    }

    void DeviceBase::HandleError(const char* message) {
        if (errorCallback) {
            errorCallback(message, errorUserdata);
        }
    }

    void DeviceBase::SetErrorCallback(nxt::DeviceErrorCallback callback, nxt::CallbackUserdata userdata) {
        this->errorCallback = callback;
        this->errorUserdata = userdata;
    }

    DeviceBase* DeviceBase::GetDevice() {
        return this;
    }

    BindGroupLayoutBase* DeviceBase::GetOrCreateBindGroupLayout(const BindGroupLayoutBase* blueprint, BindGroupLayoutBuilder* builder) {
        // The blueprint is only used to search in the cache and is not modified. However cached
        // objects can be modified, and unordered_set cannot search for a const pointer in a non
        // const pointer set. That's why we do a const_cast here, but the blueprint won't be
        // modified.
        auto iter = caches->bindGroupLayouts.find(const_cast<BindGroupLayoutBase*>(blueprint));
        if (iter != caches->bindGroupLayouts.end()) {
            return *iter;
        }

        BindGroupLayoutBase* backendObj = CreateBindGroupLayout(builder);
        caches->bindGroupLayouts.insert(backendObj);
        return backendObj;
    }

    void DeviceBase::UncacheBindGroupLayout(BindGroupLayoutBase* obj) {
        caches->bindGroupLayouts.erase(obj);
    }

    BindGroupBuilder* DeviceBase::CreateBindGroupBuilder() {
        return new BindGroupBuilder(this);
    }
    BindGroupLayoutBuilder* DeviceBase::CreateBindGroupLayoutBuilder() {
        return new BindGroupLayoutBuilder(this);
    }
    BufferBuilder* DeviceBase::CreateBufferBuilder() {
        return new BufferBuilder(this);
    }
    CommandBufferBuilder* DeviceBase::CreateCommandBufferBuilder() {
        return new CommandBufferBuilder(this);
    }
    DepthStencilStateBuilder* DeviceBase::CreateDepthStencilStateBuilder() {
        return new DepthStencilStateBuilder(this);
    }
    FramebufferBuilder* DeviceBase::CreateFramebufferBuilder() {
        return new FramebufferBuilder(this);
    }
    InputStateBuilder* DeviceBase::CreateInputStateBuilder() {
        return new InputStateBuilder(this);
    }
    PipelineBuilder* DeviceBase::CreatePipelineBuilder() {
        return new PipelineBuilder(this);
    }
    PipelineLayoutBuilder* DeviceBase::CreatePipelineLayoutBuilder() {
        return new PipelineLayoutBuilder(this);
    }
    QueueBuilder* DeviceBase::CreateQueueBuilder() {
        return new QueueBuilder(this);
    }
    RenderPassBuilder* DeviceBase::CreateRenderPassBuilder() {
        return new RenderPassBuilder(this);
    }
    SamplerBuilder* DeviceBase::CreateSamplerBuilder() {
        return new SamplerBuilder(this);
    }
    ShaderModuleBuilder* DeviceBase::CreateShaderModuleBuilder() {
        return new ShaderModuleBuilder(this);
    }
    TextureBuilder* DeviceBase::CreateTextureBuilder() {
        return new TextureBuilder(this);
    }

    void DeviceBase::Tick() {
        TickImpl();
    }

    void DeviceBase::Reference() {
        ASSERT(refCount != 0);
        refCount++;
    }

    void DeviceBase::Release() {
        ASSERT(refCount != 0);
        refCount--;
        if (refCount == 0) {
            delete this;
        }
    }

}
