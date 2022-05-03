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

#ifndef SRC_DAWN_NODE_BINDING_GPUQUEUE_H_
#define SRC_DAWN_NODE_BINDING_GPUQUEUE_H_

#include <memory>
#include <string>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUQueue is an implementation of interop::GPUQueue that wraps a wgpu::Queue.
class GPUQueue final : public interop::GPUQueue {
  public:
    GPUQueue(wgpu::Queue queue, std::shared_ptr<AsyncRunner> async);

    // interop::GPUQueue interface compliance
    void submit(Napi::Env,
                std::vector<interop::Interface<interop::GPUCommandBuffer>> commandBuffers) override;
    interop::Promise<void> onSubmittedWorkDone(Napi::Env) override;
    void writeBuffer(Napi::Env,
                     interop::Interface<interop::GPUBuffer> buffer,
                     interop::GPUSize64 bufferOffset,
                     interop::BufferSource data,
                     interop::GPUSize64 dataOffset,
                     std::optional<interop::GPUSize64> size) override;
    void writeTexture(Napi::Env,
                      interop::GPUImageCopyTexture destination,
                      interop::BufferSource data,
                      interop::GPUImageDataLayout dataLayout,
                      interop::GPUExtent3D size) override;
    void copyExternalImageToTexture(Napi::Env,
                                    interop::GPUImageCopyExternalImage source,
                                    interop::GPUImageCopyTextureTagged destination,
                                    interop::GPUExtent3D copySize) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    wgpu::Queue queue_;
    std::shared_ptr<AsyncRunner> async_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUQUEUE_H_
