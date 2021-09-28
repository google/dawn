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

#include "src/dawn_node/binding/GPUCommandBuffer.h"

#include "src/dawn_node/utils/Debug.h"

namespace wgpu { namespace binding {

    ////////////////////////////////////////////////////////////////////////////////
    // wgpu::bindings::GPUCommandBuffer
    ////////////////////////////////////////////////////////////////////////////////

    GPUCommandBuffer::GPUCommandBuffer(wgpu::CommandBuffer cmd_buf) : cmd_buf_(std::move(cmd_buf)) {
    }

    interop::Promise<double> GPUCommandBuffer::getExecutionTime(Napi::Env) {
        UNIMPLEMENTED();
    };

    std::optional<std::string> GPUCommandBuffer::getLabel(Napi::Env) {
        UNIMPLEMENTED();
    }

    void GPUCommandBuffer::setLabel(Napi::Env, std::optional<std::string> value) {
        UNIMPLEMENTED();
    }

}}  // namespace wgpu::binding
