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

#ifndef SRC_DAWN_NODE_BINDING_GPUSHADERMODULE_H_
#define SRC_DAWN_NODE_BINDING_GPUSHADERMODULE_H_

#include <memory>
#include <string>

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUShaderModule is an implementation of interop::GPUShaderModule that wraps a
// wgpu::ShaderModule.
class GPUShaderModule final : public interop::GPUShaderModule {
  public:
    GPUShaderModule(wgpu::ShaderModule shader, std::shared_ptr<AsyncRunner> async);

    // Implicit cast operator to Dawn GPU object
    inline operator const wgpu::ShaderModule&() const { return shader_; }

    // interop::GPUShaderModule interface compliance
    interop::Promise<interop::Interface<interop::GPUCompilationInfo>> compilationInfo(
        Napi::Env) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::ShaderModule shader_;
    std::shared_ptr<AsyncRunner> async_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUSHADERMODULE_H_
