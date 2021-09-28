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

#include "src/dawn_node/binding/GPUAdapter.h"

#include "src/dawn_node/binding/GPUDevice.h"
#include "src/dawn_node/binding/GPUSupportedLimits.h"

namespace wgpu { namespace binding {

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPUAdapter
    // TODO(crbug.com/dawn/1133): This is a stub implementation. Properly implement.
    ////////////////////////////////////////////////////////////////////////////////
    GPUAdapter::GPUAdapter(dawn_native::Adapter a) : adapter_(a) {
    }

    std::string GPUAdapter::getName(Napi::Env) {
        return "dawn-adapter";
    }

    interop::Interface<interop::GPUSupportedFeatures> GPUAdapter::getFeatures(Napi::Env env) {
        class Features : public interop::GPUSupportedFeatures {};
        return interop::GPUSupportedFeatures::Create<Features>(env);
    }

    interop::Interface<interop::GPUSupportedLimits> GPUAdapter::getLimits(Napi::Env env) {
        return interop::GPUSupportedLimits::Create<GPUSupportedLimits>(env);
    }

    bool GPUAdapter::getIsFallbackAdapter(Napi::Env) {
        UNIMPLEMENTED();
    }

    interop::Promise<interop::Interface<interop::GPUDevice>> GPUAdapter::requestDevice(
        Napi::Env env,
        std::optional<interop::GPUDeviceDescriptor> descriptor) {
        dawn_native::DeviceDescriptor desc{};  // TODO(crbug.com/dawn/1133): Fill in.
        interop::Promise<interop::Interface<interop::GPUDevice>> promise(env);

        auto wgpu_device = adapter_.CreateDevice(&desc);
        if (wgpu_device) {
            promise.Resolve(interop::GPUDevice::Create<GPUDevice>(env, env, wgpu_device));
        } else {
            Napi::Error::New(env, "failed to create device").ThrowAsJavaScriptException();
        }
        return promise;
    }

}}  // namespace wgpu::binding
