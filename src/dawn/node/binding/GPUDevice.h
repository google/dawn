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

#ifndef SRC_DAWN_NODE_BINDING_GPUDEVICE_H_
#define SRC_DAWN_NODE_BINDING_GPUDEVICE_H_

#include <memory>
#include <string>

#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {
// GPUDevice is an implementation of interop::GPUDevice that wraps a wgpu::Device.
class GPUDevice final : public interop::GPUDevice {
  public:
    GPUDevice(Napi::Env env, wgpu::Device device);
    ~GPUDevice();

    // interop::GPUDevice interface compliance
    interop::Interface<interop::GPUSupportedFeatures> getFeatures(Napi::Env) override;
    interop::Interface<interop::GPUSupportedLimits> getLimits(Napi::Env) override;
    interop::Interface<interop::GPUQueue> getQueue(Napi::Env env) override;
    void destroy(Napi::Env) override;
    interop::Interface<interop::GPUBuffer> createBuffer(
        Napi::Env env,
        interop::GPUBufferDescriptor descriptor) override;
    interop::Interface<interop::GPUTexture> createTexture(
        Napi::Env,
        interop::GPUTextureDescriptor descriptor) override;
    interop::Interface<interop::GPUSampler> createSampler(
        Napi::Env,
        interop::GPUSamplerDescriptor descriptor) override;
    interop::Interface<interop::GPUExternalTexture> importExternalTexture(
        Napi::Env,
        interop::GPUExternalTextureDescriptor descriptor) override;
    interop::Interface<interop::GPUBindGroupLayout> createBindGroupLayout(
        Napi::Env,
        interop::GPUBindGroupLayoutDescriptor descriptor) override;
    interop::Interface<interop::GPUPipelineLayout> createPipelineLayout(
        Napi::Env,
        interop::GPUPipelineLayoutDescriptor descriptor) override;
    interop::Interface<interop::GPUBindGroup> createBindGroup(
        Napi::Env,
        interop::GPUBindGroupDescriptor descriptor) override;
    interop::Interface<interop::GPUShaderModule> createShaderModule(
        Napi::Env,
        interop::GPUShaderModuleDescriptor descriptor) override;
    interop::Interface<interop::GPUComputePipeline> createComputePipeline(
        Napi::Env,
        interop::GPUComputePipelineDescriptor descriptor) override;
    interop::Interface<interop::GPURenderPipeline> createRenderPipeline(
        Napi::Env,
        interop::GPURenderPipelineDescriptor descriptor) override;
    interop::Promise<interop::Interface<interop::GPUComputePipeline>> createComputePipelineAsync(
        Napi::Env env,
        interop::GPUComputePipelineDescriptor descriptor) override;
    interop::Promise<interop::Interface<interop::GPURenderPipeline>> createRenderPipelineAsync(
        Napi::Env env,
        interop::GPURenderPipelineDescriptor descriptor) override;
    interop::Interface<interop::GPUCommandEncoder> createCommandEncoder(
        Napi::Env env,
        interop::GPUCommandEncoderDescriptor descriptor) override;
    interop::Interface<interop::GPURenderBundleEncoder> createRenderBundleEncoder(
        Napi::Env,
        interop::GPURenderBundleEncoderDescriptor descriptor) override;
    interop::Interface<interop::GPUQuerySet> createQuerySet(
        Napi::Env,
        interop::GPUQuerySetDescriptor descriptor) override;
    interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> getLost(
        Napi::Env env) override;
    void pushErrorScope(Napi::Env, interop::GPUErrorFilter filter) override;
    interop::Promise<std::optional<interop::Interface<interop::GPUError>>> popErrorScope(
        Napi::Env env) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;
    interop::Interface<interop::EventHandler> getOnuncapturederror(Napi::Env) override;
    void setOnuncapturederror(Napi::Env, interop::Interface<interop::EventHandler> value) override;
    void addEventListener(
        Napi::Env,
        std::string type,
        std::optional<interop::Interface<interop::EventListener>> callback,
        std::optional<std::variant<interop::AddEventListenerOptions, bool>> options) override;
    void removeEventListener(
        Napi::Env,
        std::string type,
        std::optional<interop::Interface<interop::EventListener>> callback,
        std::optional<std::variant<interop::EventListenerOptions, bool>> options) override;
    bool dispatchEvent(Napi::Env, interop::Interface<interop::Event> event) override;

  private:
    void QueueTick();

    Napi::Env env_;
    wgpu::Device device_;
    std::shared_ptr<AsyncRunner> async_;

    // This promise's JS object lives as long as the device because it is stored in .lost
    // of the wrapper JS object.
    interop::Promise<interop::Interface<interop::GPUDeviceLostInfo>> lost_promise_;

    bool destroyed_ = false;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUDEVICE_H_
