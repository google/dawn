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

#ifndef DAWN_NODE_BINDING_GPU_H_
#define DAWN_NODE_BINDING_GPU_H_

#include "dawn/webgpu_cpp.h"
#include "dawn_native/DawnNative.h"
#include "napi.h"
#include "src/dawn_node/binding/Flags.h"
#include "src/dawn_node/interop/WebGPU.h"

namespace wgpu { namespace binding {
    // GPU is an implementation of interop::GPU that wraps a dawn_native::Instance.
    class GPU final : public interop::GPU {
      public:
        GPU(Flags flags);

        // interop::GPU interface compliance
        interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>> requestAdapter(
            Napi::Env env,
            interop::GPURequestAdapterOptions options) override;

      private:
        const Flags flags_;
        dawn_native::Instance instance_;
    };

}}  // namespace wgpu::binding

#endif  // DAWN_NODE_BINDING_GPU_H_
