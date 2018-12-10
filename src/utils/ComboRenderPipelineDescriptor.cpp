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

#include "utils/ComboRenderPipelineDescriptor.h"

#include "utils/DawnHelpers.h"

namespace utils {

    ComboRenderPipelineDescriptor::ComboRenderPipelineDescriptor(const dawn::Device& device) {
        dawn::RenderPipelineDescriptor* descriptor = this;

        descriptor->indexFormat = dawn::IndexFormat::Uint32;
        descriptor->primitiveTopology = dawn::PrimitiveTopology::TriangleList;
        descriptor->sampleCount = 1;

        // Set defaults for the vertex stage descriptor.
        {
            descriptor->vertexStage = &cVertexStage;
            cVertexStage.entryPoint = "main";
        }

        // Set defaults for the fragment stage desriptor.
        {
            descriptor->fragmentStage = &cFragmentStage;
            cFragmentStage.entryPoint = "main";
        }

        // Set defaults for the attachment states.
        {
            descriptor->attachmentsState = &cAttachmentsState;
            cAttachmentsState.numColorAttachments = 1;
            cAttachmentsState.colorAttachments = cColorAttachments;
            cAttachmentsState.depthStencilAttachment = &cDepthStencilAttachment;
            cAttachmentsState.hasDepthStencilAttachment = false;

            for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
                cColorAttachments[i].format = dawn::TextureFormat::R8G8B8A8Unorm;
            }
        }

        descriptor->inputState = device.CreateInputStateBuilder().GetResult();
        descriptor->depthStencilState = device.CreateDepthStencilStateBuilder().GetResult();
        descriptor->layout = utils::MakeBasicPipelineLayout(device, nullptr);

        descriptor->numBlendStates = 1;
        descriptor->blendStates = cBlendStates;

        for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
            cBlendStates[i] = device.CreateBlendStateBuilder().GetResult();
        }
    }

}  // namespace utils
