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

#ifndef SRC_DAWN_NODE_BINDING_GPU_H_
#define SRC_DAWN_NODE_BINDING_GPU_H_

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"

#include "src/dawn/node/binding/Flags.h"
#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {
// GPU is an implementation of interop::GPU that wraps a dawn::native::Instance.
class GPU final : public interop::GPU {
  public:
    GPU(Flags flags);

    // interop::GPU interface compliance
    interop::Promise<std::optional<interop::Interface<interop::GPUAdapter>>> requestAdapter(
        Napi::Env env,
        interop::GPURequestAdapterOptions options) override;
    interop::GPUTextureFormat getPreferredCanvasFormat(Napi::Env) override;

  private:
    const Flags flags_;
    dawn::native::Instance instance_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPU_H_
