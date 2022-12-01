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

#ifndef SRC_DAWN_NODE_BINDING_GPUBUFFER_H_
#define SRC_DAWN_NODE_BINDING_GPUBUFFER_H_

#include <memory>
#include <string>
#include <vector>

#include "dawn/native/DawnNative.h"
#include "dawn/webgpu_cpp.h"
#include "src/dawn/node/binding/AsyncRunner.h"
#include "src/dawn/node/interop/Napi.h"
#include "src/dawn/node/interop/WebGPU.h"

namespace wgpu::binding {

// GPUBuffer is an implementation of interop::GPUBuffer that wraps a wgpu::Buffer.
class GPUBuffer final : public interop::GPUBuffer {
  public:
    GPUBuffer(wgpu::Buffer buffer,
              wgpu::BufferDescriptor desc,
              wgpu::Device device,
              std::shared_ptr<AsyncRunner> async);

    // Desc() returns the wgpu::BufferDescriptor used to construct the buffer
    const wgpu::BufferDescriptor& Desc() const { return desc_; }

    // Implicit cast operator to Dawn GPU object
    inline operator const wgpu::Buffer&() const { return buffer_; }

    // interop::GPUBuffer interface compliance
    interop::Promise<void> mapAsync(Napi::Env env,
                                    interop::GPUMapModeFlags mode,
                                    interop::GPUSize64 offset,
                                    std::optional<interop::GPUSize64> size) override;
    interop::ArrayBuffer getMappedRange(Napi::Env env,
                                        interop::GPUSize64 offset,
                                        std::optional<interop::GPUSize64> size) override;
    void unmap(Napi::Env) override;
    void destroy(Napi::Env) override;
    interop::GPUSize64 getSize(Napi::Env) override;
    interop::GPUBufferUsageFlags getUsage(Napi::Env) override;
    interop::GPUBufferMapState getMapState(Napi::Env) override;
    std::string getLabel(Napi::Env) override;
    void setLabel(Napi::Env, std::string value) override;

  private:
    void DetachMappings();

    struct Mapping {
        uint64_t start;
        uint64_t end;
        inline bool Intersects(uint64_t s, uint64_t e) const { return s < end && e > start; }
        Napi::Reference<interop::ArrayBuffer> buffer;
    };

    // https://www.w3.org/TR/webgpu/#buffer-interface
    enum class State {
        Unmapped,
        Mapped,
        MappedAtCreation,
        MappingPending,
        Destroyed,
    };

    wgpu::Buffer buffer_;
    wgpu::BufferDescriptor const desc_;
    wgpu::Device const device_;
    std::shared_ptr<AsyncRunner> async_;
    State state_ = State::Unmapped;
    std::vector<Mapping> mapped_;
};

}  // namespace wgpu::binding

#endif  // SRC_DAWN_NODE_BINDING_GPUBUFFER_H_
