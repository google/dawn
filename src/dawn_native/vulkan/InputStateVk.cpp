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

#include "dawn_native/vulkan/InputStateVk.h"

#include "common/BitSetIterator.h"

namespace dawn_native { namespace vulkan {

    namespace {

        VkVertexInputRate VulkanInputRate(dawn::InputStepMode stepMode) {
            switch (stepMode) {
                case dawn::InputStepMode::Vertex:
                    return VK_VERTEX_INPUT_RATE_VERTEX;
                case dawn::InputStepMode::Instance:
                    return VK_VERTEX_INPUT_RATE_INSTANCE;
                default:
                    UNREACHABLE();
            }
        }

        VkFormat VulkanVertexFormat(dawn::VertexFormat format) {
            switch (format) {
                case dawn::VertexFormat::UChar2:
                    return VK_FORMAT_R8G8_UINT;
                case dawn::VertexFormat::UChar4:
                    return VK_FORMAT_R8G8B8A8_UINT;
                case dawn::VertexFormat::Char2:
                    return VK_FORMAT_R8G8_SINT;
                case dawn::VertexFormat::Char4:
                    return VK_FORMAT_R8G8B8A8_SINT;
                case dawn::VertexFormat::UChar2Norm:
                    return VK_FORMAT_R8G8_UNORM;
                case dawn::VertexFormat::UChar4Norm:
                    return VK_FORMAT_R8G8B8A8_UNORM;
                case dawn::VertexFormat::Char2Norm:
                    return VK_FORMAT_R8G8_SNORM;
                case dawn::VertexFormat::Char4Norm:
                    return VK_FORMAT_R8G8B8A8_SNORM;
                case dawn::VertexFormat::UShort2:
                    return VK_FORMAT_R16G16_UINT;
                case dawn::VertexFormat::UShort4:
                    return VK_FORMAT_R16G16B16A16_UINT;
                case dawn::VertexFormat::Short2:
                    return VK_FORMAT_R16G16_SINT;
                case dawn::VertexFormat::Short4:
                    return VK_FORMAT_R16G16B16A16_SINT;
                case dawn::VertexFormat::UShort2Norm:
                    return VK_FORMAT_R16G16_UNORM;
                case dawn::VertexFormat::UShort4Norm:
                    return VK_FORMAT_R16G16B16A16_UNORM;
                case dawn::VertexFormat::Short2Norm:
                    return VK_FORMAT_R16G16_SNORM;
                case dawn::VertexFormat::Short4Norm:
                    return VK_FORMAT_R16G16B16A16_SNORM;
                case dawn::VertexFormat::Half2:
                    return VK_FORMAT_R16G16_SFLOAT;
                case dawn::VertexFormat::Half4:
                    return VK_FORMAT_R16G16B16A16_SFLOAT;
                case dawn::VertexFormat::Float:
                    return VK_FORMAT_R32_SFLOAT;
                case dawn::VertexFormat::Float2:
                    return VK_FORMAT_R32G32_SFLOAT;
                case dawn::VertexFormat::Float3:
                    return VK_FORMAT_R32G32B32_SFLOAT;
                case dawn::VertexFormat::Float4:
                    return VK_FORMAT_R32G32B32A32_SFLOAT;
                case dawn::VertexFormat::UInt:
                    return VK_FORMAT_R32_UINT;
                case dawn::VertexFormat::UInt2:
                    return VK_FORMAT_R32G32_UINT;
                case dawn::VertexFormat::UInt3:
                    return VK_FORMAT_R32G32B32_UINT;
                case dawn::VertexFormat::UInt4:
                    return VK_FORMAT_R32G32B32A32_UINT;
                case dawn::VertexFormat::Int:
                    return VK_FORMAT_R32_SINT;
                case dawn::VertexFormat::Int2:
                    return VK_FORMAT_R32G32_SINT;
                case dawn::VertexFormat::Int3:
                    return VK_FORMAT_R32G32B32_SINT;
                case dawn::VertexFormat::Int4:
                    return VK_FORMAT_R32G32B32A32_SINT;
                default:
                    UNREACHABLE();
            }
        }

    }  // anonymous namespace

    InputState::InputState(InputStateBuilder* builder) : InputStateBase(builder) {
        // Fill in the "binding info" that will be chained in the create info
        uint32_t bindingCount = 0;
        for (uint32_t i : IterateBitSet(GetInputsSetMask())) {
            const auto& bindingInfo = GetInput(i);

            auto& bindingDesc = mBindings[bindingCount];
            bindingDesc.binding = i;
            bindingDesc.stride = bindingInfo.stride;
            bindingDesc.inputRate = VulkanInputRate(bindingInfo.stepMode);

            bindingCount++;
        }

        // Fill in the "attribute info" that will be chained in the create info
        uint32_t attributeCount = 0;
        for (uint32_t i : IterateBitSet(GetAttributesSetMask())) {
            const auto& attributeInfo = GetAttribute(i);

            auto& attributeDesc = mAttributes[attributeCount];
            attributeDesc.location = i;
            attributeDesc.binding = attributeInfo.inputSlot;
            attributeDesc.format = VulkanVertexFormat(attributeInfo.format);
            attributeDesc.offset = attributeInfo.offset;

            attributeCount++;
        }

        // Build the create info
        mCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        mCreateInfo.pNext = nullptr;
        mCreateInfo.flags = 0;
        mCreateInfo.vertexBindingDescriptionCount = bindingCount;
        mCreateInfo.pVertexBindingDescriptions = mBindings.data();
        mCreateInfo.vertexAttributeDescriptionCount = attributeCount;
        mCreateInfo.pVertexAttributeDescriptions = mAttributes.data();
    }

    const VkPipelineVertexInputStateCreateInfo* InputState::GetCreateInfo() const {
        return &mCreateInfo;
    }

}}  // namespace dawn_native::vulkan
