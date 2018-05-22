// Copyright 2017 The NXT Authors
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

#include <nxt/nxtcpp.h>

#include <initializer_list>

namespace utils {

    void FillShaderModuleBuilder(const nxt::ShaderModuleBuilder& builder,
                                 nxt::ShaderStage stage,
                                 const char* source);
    nxt::ShaderModule CreateShaderModule(const nxt::Device& device,
                                         nxt::ShaderStage stage,
                                         const char* source);
    nxt::Buffer CreateFrozenBufferFromData(const nxt::Device& device,
                                           const void* data,
                                           uint32_t size,
                                           nxt::BufferUsageBit usage);

    template <typename T>
    nxt::Buffer CreateFrozenBufferFromData(const nxt::Device& device,
                                           nxt::BufferUsageBit usage,
                                           std::initializer_list<T> data) {
        return CreateFrozenBufferFromData(device, data.begin(), uint32_t(sizeof(T) * data.size()),
                                          usage);
    }

    struct BasicRenderPass {
        uint32_t width;
        uint32_t height;
        nxt::Texture color;
        nxt::TextureFormat colorFormat;
        nxt::RenderPassDescriptor renderPassInfo;
    };
    BasicRenderPass CreateBasicRenderPass(const nxt::Device& device,
                                          uint32_t width,
                                          uint32_t height);

    nxt::SamplerDescriptor GetDefaultSamplerDescriptor();

}  // namespace utils
