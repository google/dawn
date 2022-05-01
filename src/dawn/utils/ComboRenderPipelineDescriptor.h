// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_
#define SRC_DAWN_UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_

#include <array>

#include "dawn/common/Constants.h"
#include "dawn/webgpu_cpp.h"

namespace utils {

// Primarily used by tests to easily set up the vertex buffer state portion of a RenderPipeline.
class ComboVertexState {
  public:
    ComboVertexState();

    ComboVertexState(const ComboVertexState&) = delete;
    ComboVertexState& operator=(const ComboVertexState&) = delete;
    ComboVertexState(ComboVertexState&&) = delete;
    ComboVertexState& operator=(ComboVertexState&&) = delete;

    uint32_t vertexBufferCount;
    std::array<wgpu::VertexBufferLayout, kMaxVertexBuffers> cVertexBuffers;
    std::array<wgpu::VertexAttribute, kMaxVertexAttributes> cAttributes;
};

class ComboRenderPipelineDescriptor : public wgpu::RenderPipelineDescriptor {
  public:
    ComboRenderPipelineDescriptor();

    ComboRenderPipelineDescriptor(const ComboRenderPipelineDescriptor&) = delete;
    ComboRenderPipelineDescriptor& operator=(const ComboRenderPipelineDescriptor&) = delete;
    ComboRenderPipelineDescriptor(ComboRenderPipelineDescriptor&&) = delete;
    ComboRenderPipelineDescriptor& operator=(ComboRenderPipelineDescriptor&&) = delete;

    wgpu::DepthStencilState* EnableDepthStencil(
        wgpu::TextureFormat format = wgpu::TextureFormat::Depth24PlusStencil8);

    std::array<wgpu::VertexBufferLayout, kMaxVertexBuffers> cBuffers;
    std::array<wgpu::VertexAttribute, kMaxVertexAttributes> cAttributes;
    std::array<wgpu::ColorTargetState, kMaxColorAttachments> cTargets;
    std::array<wgpu::BlendState, kMaxColorAttachments> cBlends;

    wgpu::FragmentState cFragment;
    wgpu::DepthStencilState cDepthStencil;
};

}  // namespace utils

#endif  // SRC_DAWN_UTILS_COMBORENDERPIPELINEDESCRIPTOR_H_
