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

#ifndef DAWN_NODE_BINDING_GPUADAPTER_H_
#define DAWN_NODE_BINDING_GPUADAPTER_H_

#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "napi.h"
#include "src/dawn_node/interop/WebGPU.h"

namespace wgpu { namespace binding {
    class Flags;

    // GPUAdapter is an implementation of interop::GPUAdapter that wraps a dawn_native::Adapter.
    class GPUAdapter final : public interop::GPUAdapter {
      public:
        GPUAdapter(dawn_native::Adapter a, const Flags& flags);

        // interop::GPUAdapter interface compliance
        std::string getName(Napi::Env) override;
        interop::Interface<interop::GPUSupportedFeatures> getFeatures(Napi::Env) override;
        interop::Interface<interop::GPUSupportedLimits> getLimits(Napi::Env) override;
        bool getIsFallbackAdapter(Napi::Env) override;
        interop::Promise<interop::Interface<interop::GPUDevice>> requestDevice(
            Napi::Env env,
            interop::GPUDeviceDescriptor descriptor) override;

      private:
        dawn_native::Adapter adapter_;
        const Flags& flags_;
    };

}}  // namespace wgpu::binding

#endif  // DAWN_NODE_BINDING_GPUADAPTER_H_
