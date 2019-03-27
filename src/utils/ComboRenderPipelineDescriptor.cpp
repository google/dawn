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

        // Set defaults for the input state descriptors.
        {
            descriptor->inputState = &cInputState;
            cInputState.numInputs = 0;
            cInputState.inputs = nullptr;
            cInputState.numAttributes = 0;
            cInputState.attributes = nullptr;
        }

        // Set defaults for the color state descriptors.
        {
            descriptor->colorStateCount = 1;
            descriptor->colorStates = &cColorStates[0];

            dawn::BlendDescriptor blend;
            blend.operation = dawn::BlendOperation::Add;
            blend.srcFactor = dawn::BlendFactor::One;
            blend.dstFactor = dawn::BlendFactor::Zero;
            dawn::ColorStateDescriptor colorStateDescriptor;
            colorStateDescriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
            colorStateDescriptor.alphaBlend = blend;
            colorStateDescriptor.colorBlend = blend;
            colorStateDescriptor.colorWriteMask = dawn::ColorWriteMask::All;
            for (uint32_t i = 0; i < kMaxColorAttachments; ++i) {
                mColorStates[i] = colorStateDescriptor;
                cColorStates[i] = &mColorStates[i];
            }
        }

        // Set defaults for the depth stencil state descriptors.
        {
            dawn::StencilStateFaceDescriptor stencilFace;
            stencilFace.compare = dawn::CompareFunction::Always;
            stencilFace.failOp = dawn::StencilOperation::Keep;
            stencilFace.depthFailOp = dawn::StencilOperation::Keep;
            stencilFace.passOp = dawn::StencilOperation::Keep;

            cDepthStencilState.format = dawn::TextureFormat::D32FloatS8Uint;
            cDepthStencilState.depthWriteEnabled = false;
            cDepthStencilState.depthCompare = dawn::CompareFunction::Always;
            cDepthStencilState.stencilBack = stencilFace;
            cDepthStencilState.stencilFront = stencilFace;
            cDepthStencilState.stencilReadMask = 0xff;
            cDepthStencilState.stencilWriteMask = 0xff;
            descriptor->depthStencilState = nullptr;
        }

        descriptor->layout = utils::MakeBasicPipelineLayout(device, nullptr);
    }

}  // namespace utils
