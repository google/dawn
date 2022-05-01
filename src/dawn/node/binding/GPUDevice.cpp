// Copyright 2021 The Dawn Authors
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

#include "src/dawn/node/binding/GPUDevice.h"

#include <memory>
#include <utility>
#include <vector>

#include "src/dawn/node/binding/Converter.h"
#include "src/dawn/node/binding/Errors.h"
#include "src/dawn/node/binding/GPUBindGroup.h"
#include "src/dawn/node/binding/GPUBindGroupLayout.h"
#include "src/dawn/node/binding/GPUBuffer.h"
#include "src/dawn/node/binding/GPUCommandBuffer.h"
#include "src/dawn/node/binding/GPUCommandEncoder.h"
#include "src/dawn/node/binding/GPUComputePipeline.h"
#include "src/dawn/node/binding/GPUPipelineLayout.h"
#include "src/dawn/node/binding/GPUQuerySet.h"
#include "src/dawn/node/binding/GPUQueue.h"
#include "src/dawn/node/binding/GPURenderBundleEncoder.h"
#include "src/dawn/node/binding/GPURenderPipeline.h"
#include "src/dawn/node/binding/GPUSampler.h"
#include "src/dawn/node/binding/GPUShaderModule.h"
#include "src/dawn/node/binding/GPUSupportedLimits.h"
#include "src/dawn/node/binding/GPUTexture.h"
#include "src/dawn/node/utils/Debug.h"

namespace wgpu::binding {

namespace {

class DeviceLostInfo : public interop::GPUDeviceLostInfo {
  public:
    DeviceLostInfo(interop::GPUDeviceLostReason reason, std::string message)
        : reason_(reason), message_(message) {}
    std::variant<interop::GPUDeviceLostReason, interop::UndefinedType> getReason(
        Napi::Env env) override {
        return reason_;
    }
    std::string getMessage(Napi::Env) override { return message_; }

  private:
    interop::GPUDeviceLostReason reason_;
    std::string message_;
};

class OOMError : public interop::GPUOutOfMemoryError {};
class ValidationError : public interop::GPUValidationError {
  public:
    explicit ValidationError(std::string message) : message_(std::move(message)) {}

    std::string getMessage(Napi::Env) override { return message_; };

  private:
    std::string message_;
};

}  // namespace

////////////////////////////////////////////////////////////////////////////////
// wgpu::bindings::GPUDevice
////////////////////////////////////////////////////////////////////////////////
GPUDevice::GPUDevice(Napi::Env env, wgpu::Device device)
    : env_(env),
      device_(device),
      async_(std::make_shared<AsyncRunner>(env, device)),
      lost_promise_(env, PROMISE_INFO) {
    device_.SetLoggingCallback(
        [](WGPULoggingType type, char const* message, void* userdata) {
            std::cout << type << ": " << message << std::endl;
        },
        nullptr);
    device_.SetUncapturedErrorCallback(
        [](WGPUErrorType type, char const* message, void* userdata) {
            std::cout << type << ": " << message << std::endl;
        },
        nullptr);

    device_.SetDeviceLostCallback(
        [](WGPUDeviceLostReason reason, char const* message, void* userdata) {
            auto r = interop::GPUDeviceLostReason::kDestroyed;
            switch (reason) {
                case WGPUDeviceLostReason_Force32:
                    UNREACHABLE("WGPUDeviceLostReason_Force32");
                    break;
                case WGPUDeviceLostReason_Destroyed:
                case WGPUDeviceLostReason_Undefined:
                    r = interop::GPUDeviceLostReason::kDestroyed;
                    break;
            }
            auto* self = static_cast<GPUDevice*>(userdata);
            if (self->lost_promise_.GetState() == interop::PromiseState::Pending) {
                self->lost_promise_.Resolve(
                    interop::GPUDeviceLostInfo::Create<DeviceLostInfo>(self->env_, r, message));
            }
        },
        this);
}

GPUDevice::~GPUDevice() {}

interop::Interface<interop::GPUSupportedFeatures> GPUDevice::getFeatures(Napi::Env env) {
    class Features : public interop::GPUSupportedFeatures {
      public:
        bool has(Napi::Env, std::string feature) override { UNIMPLEMENTED(); }
        std::vector<std::string> keys(Napi::Env) override { UNIMPLEMENTED(); }
    };
    return interop::GPUSupportedFeatures::Create<Features>(env);
}

interop::Interface<interop::GPUSupportedLimits> GPUDevice::getLimits(Napi::Env env) {
    wgpu::SupportedLimits limits{};
    if (!device_.GetLimits(&limits)) {
        Napi::Error::New(env, "failed to get device limits").ThrowAsJavaScriptException();
    }
    return interop::GPUSupportedLimits::Create<GPUSupportedLimits>(env, limits);
}

interop::Interface<interop::GPUQueue> GPUDevice::getQueue(Napi::Env env) {
    return interop::GPUQueue::Create<GPUQueue>(env, device_.GetQueue(), async_);
}

void GPUDevice::destroy(Napi::Env env) {
    if (lost_promise_.GetState() == interop::PromiseState::Pending) {
        lost_promise_.Resolve(interop::GPUDeviceLostInfo::Create<DeviceLostInfo>(
            env_, interop::GPUDeviceLostReason::kDestroyed, "device was destroyed"));
    }
    device_.Destroy();
}

interop::Interface<interop::GPUBuffer> GPUDevice::createBuffer(
    Napi::Env env,
    interop::GPUBufferDescriptor descriptor) {
    Converter conv(env);

    wgpu::BufferDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.mappedAtCreation, descriptor.mappedAtCreation) ||
        !conv(desc.size, descriptor.size) || !conv(desc.usage, descriptor.usage)) {
        return {};
    }
    return interop::GPUBuffer::Create<GPUBuffer>(env, device_.CreateBuffer(&desc), desc, device_,
                                                 async_);
}

interop::Interface<interop::GPUTexture> GPUDevice::createTexture(
    Napi::Env env,
    interop::GPUTextureDescriptor descriptor) {
    Converter conv(env);

    wgpu::TextureDescriptor desc{};
    if (!conv(desc.label, descriptor.label) || !conv(desc.usage, descriptor.usage) ||  //
        !conv(desc.size, descriptor.size) ||                                           //
        !conv(desc.dimension, descriptor.dimension) ||                                 //
        !conv(desc.mipLevelCount, descriptor.mipLevelCount) ||                         //
        !conv(desc.sampleCount, descriptor.sampleCount) ||                             //
        !conv(desc.format, descriptor.format)) {
        return {};
    }
    return interop::GPUTexture::Create<GPUTexture>(env, device_.CreateTexture(&desc));
}

interop::Interface<interop::GPUSampler> GPUDevice::createSampler(
    Napi::Env env,
    interop::GPUSamplerDescriptor descriptor) {
    Converter conv(env);

    wgpu::SamplerDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||                //
        !conv(desc.addressModeU, descriptor.addressModeU) ||  //
        !conv(desc.addressModeV, descriptor.addressModeV) ||  //
        !conv(desc.addressModeW, descriptor.addressModeW) ||  //
        !conv(desc.magFilter, descriptor.magFilter) ||        //
        !conv(desc.minFilter, descriptor.minFilter) ||        //
        !conv(desc.mipmapFilter, descriptor.mipmapFilter) ||  //
        !conv(desc.lodMinClamp, descriptor.lodMinClamp) ||    //
        !conv(desc.lodMaxClamp, descriptor.lodMaxClamp) ||    //
        !conv(desc.compare, descriptor.compare) ||            //
        !conv(desc.maxAnisotropy, descriptor.maxAnisotropy)) {
        return {};
    }
    return interop::GPUSampler::Create<GPUSampler>(env, device_.CreateSampler(&desc));
}

interop::Interface<interop::GPUExternalTexture> GPUDevice::importExternalTexture(
    Napi::Env,
    interop::GPUExternalTextureDescriptor descriptor) {
    UNIMPLEMENTED();
}

interop::Interface<interop::GPUBindGroupLayout> GPUDevice::createBindGroupLayout(
    Napi::Env env,
    interop::GPUBindGroupLayoutDescriptor descriptor) {
    Converter conv(env);

    wgpu::BindGroupLayoutDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.entries, desc.entryCount, descriptor.entries)) {
        return {};
    }

    return interop::GPUBindGroupLayout::Create<GPUBindGroupLayout>(
        env, device_.CreateBindGroupLayout(&desc));
}

interop::Interface<interop::GPUPipelineLayout> GPUDevice::createPipelineLayout(
    Napi::Env env,
    interop::GPUPipelineLayoutDescriptor descriptor) {
    Converter conv(env);

    wgpu::PipelineLayoutDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.bindGroupLayouts, desc.bindGroupLayoutCount, descriptor.bindGroupLayouts)) {
        return {};
    }

    return interop::GPUPipelineLayout::Create<GPUPipelineLayout>(
        env, device_.CreatePipelineLayout(&desc));
}

interop::Interface<interop::GPUBindGroup> GPUDevice::createBindGroup(
    Napi::Env env,
    interop::GPUBindGroupDescriptor descriptor) {
    Converter conv(env);

    wgpu::BindGroupDescriptor desc{};
    if (!conv(desc.label, descriptor.label) || !conv(desc.layout, descriptor.layout) ||
        !conv(desc.entries, desc.entryCount, descriptor.entries)) {
        return {};
    }

    return interop::GPUBindGroup::Create<GPUBindGroup>(env, device_.CreateBindGroup(&desc));
}

interop::Interface<interop::GPUShaderModule> GPUDevice::createShaderModule(
    Napi::Env env,
    interop::GPUShaderModuleDescriptor descriptor) {
    Converter conv(env);

    wgpu::ShaderModuleWGSLDescriptor wgsl_desc{};
    wgpu::ShaderModuleDescriptor sm_desc{};
    if (!conv(wgsl_desc.source, descriptor.code) || !conv(sm_desc.label, descriptor.label)) {
        return {};
    }
    sm_desc.nextInChain = &wgsl_desc;

    return interop::GPUShaderModule::Create<GPUShaderModule>(
        env, device_.CreateShaderModule(&sm_desc), async_);
}

interop::Interface<interop::GPUComputePipeline> GPUDevice::createComputePipeline(
    Napi::Env env,
    interop::GPUComputePipelineDescriptor descriptor) {
    Converter conv(env);

    wgpu::ComputePipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        return {};
    }

    return interop::GPUComputePipeline::Create<GPUComputePipeline>(
        env, device_.CreateComputePipeline(&desc));
}

interop::Interface<interop::GPURenderPipeline> GPUDevice::createRenderPipeline(
    Napi::Env env,
    interop::GPURenderPipelineDescriptor descriptor) {
    Converter conv(env);

    wgpu::RenderPipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        return {};
    }

    return interop::GPURenderPipeline::Create<GPURenderPipeline>(
        env, device_.CreateRenderPipeline(&desc));
}

interop::Promise<interop::Interface<interop::GPUComputePipeline>>
GPUDevice::createComputePipelineAsync(Napi::Env env,
                                      interop::GPUComputePipelineDescriptor descriptor) {
    using Promise = interop::Promise<interop::Interface<interop::GPUComputePipeline>>;

    Converter conv(env);

    wgpu::ComputePipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        Promise promise(env, PROMISE_INFO);
        promise.Reject(Errors::OperationError(env));
        return promise;
    }

    struct Context {
        Napi::Env env;
        Promise promise;
        AsyncTask task;
    };
    auto ctx = new Context{env, Promise(env, PROMISE_INFO), AsyncTask(async_)};
    auto promise = ctx->promise;

    device_.CreateComputePipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPUComputePipeline pipeline, char const* message,
           void* userdata) {
            auto c = std::unique_ptr<Context>(static_cast<Context*>(userdata));

            switch (status) {
                case WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success:
                    c->promise.Resolve(
                        interop::GPUComputePipeline::Create<GPUComputePipeline>(c->env, pipeline));
                    break;
                default:
                    c->promise.Reject(Errors::OperationError(c->env));
                    break;
            }
        },
        ctx);

    return promise;
}

interop::Promise<interop::Interface<interop::GPURenderPipeline>>
GPUDevice::createRenderPipelineAsync(Napi::Env env,
                                     interop::GPURenderPipelineDescriptor descriptor) {
    using Promise = interop::Promise<interop::Interface<interop::GPURenderPipeline>>;

    Converter conv(env);

    wgpu::RenderPipelineDescriptor desc{};
    if (!conv(desc, descriptor)) {
        Promise promise(env, PROMISE_INFO);
        promise.Reject(Errors::OperationError(env));
        return promise;
    }

    struct Context {
        Napi::Env env;
        Promise promise;
        AsyncTask task;
    };
    auto ctx = new Context{env, Promise(env, PROMISE_INFO), AsyncTask(async_)};
    auto promise = ctx->promise;

    device_.CreateRenderPipelineAsync(
        &desc,
        [](WGPUCreatePipelineAsyncStatus status, WGPURenderPipeline pipeline, char const* message,
           void* userdata) {
            auto c = std::unique_ptr<Context>(static_cast<Context*>(userdata));

            switch (status) {
                case WGPUCreatePipelineAsyncStatus::WGPUCreatePipelineAsyncStatus_Success:
                    c->promise.Resolve(
                        interop::GPURenderPipeline::Create<GPURenderPipeline>(c->env, pipeline));
                    break;
                default:
                    c->promise.Reject(Errors::OperationError(c->env));
                    break;
            }
        },
        ctx);

    return promise;
}

interop::Interface<interop::GPUCommandEncoder> GPUDevice::createCommandEncoder(
    Napi::Env env,
    interop::GPUCommandEncoderDescriptor descriptor) {
    wgpu::CommandEncoderDescriptor desc{};
    return interop::GPUCommandEncoder::Create<GPUCommandEncoder>(
        env, device_.CreateCommandEncoder(&desc));
}

interop::Interface<interop::GPURenderBundleEncoder> GPUDevice::createRenderBundleEncoder(
    Napi::Env env,
    interop::GPURenderBundleEncoderDescriptor descriptor) {
    Converter conv(env);

    wgpu::RenderBundleEncoderDescriptor desc{};
    if (!conv(desc.label, descriptor.label) ||
        !conv(desc.colorFormats, desc.colorFormatsCount, descriptor.colorFormats) ||
        !conv(desc.depthStencilFormat, descriptor.depthStencilFormat) ||
        !conv(desc.sampleCount, descriptor.sampleCount) ||
        !conv(desc.depthReadOnly, descriptor.depthReadOnly) ||
        !conv(desc.stencilReadOnly, descriptor.stencilReadOnly)) {
        return {};
    }

    return interop::GPURenderBundleEncoder::Create<GPURenderBundleEncoder>(
        env, device_.CreateRenderBundleEncoder(&desc));
}

interop::Interface<interop::GPUQuerySet> GPUDevice::createQuerySet(
    Napi::Env env,
    interop::GPUQuerySetDescriptor descriptor) {
    Converter conv(env);

    wgpu::QuerySetDescriptor desc{};
    if (!conv(desc.label, descriptor.label) || !conv(desc.type, descriptor.type) ||
        !conv(desc.count, descriptor.count)) {
        return {};
    }

    return interop::GPUQuerySet::Create<GPUQuerySet>(env, device_.CreateQuerySet(&desc));
}

interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> GPUDevice::getLost(Napi::Env env) {
    return lost_promise_;
}

void GPUDevice::pushErrorScope(Napi::Env env, interop::GPUErrorFilter filter) {
    wgpu::ErrorFilter f;
    switch (filter) {
        case interop::GPUErrorFilter::kOutOfMemory:
            f = wgpu::ErrorFilter::OutOfMemory;
            break;
        case interop::GPUErrorFilter::kValidation:
            f = wgpu::ErrorFilter::Validation;
            break;
        default:
            Napi::Error::New(env, "unhandled GPUErrorFilter value").ThrowAsJavaScriptException();
            return;
    }
    device_.PushErrorScope(f);
}

interop::Promise<std::optional<interop::GPUError>> GPUDevice::popErrorScope(Napi::Env env) {
    using Promise = interop::Promise<std::optional<interop::GPUError>>;
    struct Context {
        Napi::Env env;
        Promise promise;
        AsyncTask task;
    };
    auto* ctx = new Context{env, Promise(env, PROMISE_INFO), AsyncTask(async_)};
    auto promise = ctx->promise;

    device_.PopErrorScope(
        [](WGPUErrorType type, char const* message, void* userdata) {
            auto c = std::unique_ptr<Context>(static_cast<Context*>(userdata));
            auto env = c->env;
            switch (type) {
                case WGPUErrorType::WGPUErrorType_NoError:
                    c->promise.Resolve({});
                    break;
                case WGPUErrorType::WGPUErrorType_OutOfMemory:
                    c->promise.Resolve(interop::GPUOutOfMemoryError::Create<OOMError>(env));
                    break;
                case WGPUErrorType::WGPUErrorType_Validation:
                    c->promise.Resolve(
                        interop::GPUValidationError::Create<ValidationError>(env, message));
                    break;
                case WGPUErrorType::WGPUErrorType_Unknown:
                case WGPUErrorType::WGPUErrorType_DeviceLost:
                    c->promise.Reject(Errors::OperationError(env, message));
                    break;
                default:
                    c->promise.Reject("unhandled error type");
                    break;
            }
        },
        ctx);

    return promise;
}

std::variant<std::string, interop::UndefinedType> GPUDevice::getLabel(Napi::Env) {
    UNIMPLEMENTED();
}

void GPUDevice::setLabel(Napi::Env, std::variant<std::string, interop::UndefinedType> value) {
    UNIMPLEMENTED();
}

interop::Interface<interop::EventHandler> GPUDevice::getOnuncapturederror(Napi::Env) {
    // TODO(dawn:1348): Implement support for the "unhandlederror" event.
    UNIMPLEMENTED();
}

void GPUDevice::setOnuncapturederror(Napi::Env, interop::Interface<interop::EventHandler> value) {
    // TODO(dawn:1348): Implement support for the "unhandlederror" event.
    UNIMPLEMENTED();
}

void GPUDevice::addEventListener(
    Napi::Env,
    std::string type,
    std::optional<interop::Interface<interop::EventListener>> callback,
    std::optional<std::variant<interop::AddEventListenerOptions, bool>> options) {
    // TODO(dawn:1348): Implement support for the "unhandlederror" event.
    UNIMPLEMENTED();
}

void GPUDevice::removeEventListener(
    Napi::Env,
    std::string type,
    std::optional<interop::Interface<interop::EventListener>> callback,
    std::optional<std::variant<interop::EventListenerOptions, bool>> options) {
    // TODO(dawn:1348): Implement support for the "unhandlederror" event.
    UNIMPLEMENTED();
}

bool GPUDevice::dispatchEvent(Napi::Env, interop::Interface<interop::Event> event) {
    // TODO(dawn:1348): Implement support for the "unhandlederror" event.
    UNIMPLEMENTED();
}

}  // namespace wgpu::binding
