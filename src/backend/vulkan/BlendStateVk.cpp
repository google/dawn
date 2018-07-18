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

#include "backend/vulkan/BlendStateVk.h"

#include "common/Assert.h"

namespace backend { namespace vulkan {

    namespace {
        VkBlendFactor VulkanBlendFactor(dawn::BlendFactor factor) {
            switch (factor) {
                case dawn::BlendFactor::Zero:
                    return VK_BLEND_FACTOR_ZERO;
                case dawn::BlendFactor::One:
                    return VK_BLEND_FACTOR_ONE;
                case dawn::BlendFactor::SrcColor:
                    return VK_BLEND_FACTOR_SRC_COLOR;
                case dawn::BlendFactor::OneMinusSrcColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case dawn::BlendFactor::SrcAlpha:
                    return VK_BLEND_FACTOR_SRC_ALPHA;
                case dawn::BlendFactor::OneMinusSrcAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case dawn::BlendFactor::DstColor:
                    return VK_BLEND_FACTOR_DST_COLOR;
                case dawn::BlendFactor::OneMinusDstColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case dawn::BlendFactor::DstAlpha:
                    return VK_BLEND_FACTOR_DST_ALPHA;
                case dawn::BlendFactor::OneMinusDstAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                case dawn::BlendFactor::SrcAlphaSaturated:
                    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
                case dawn::BlendFactor::BlendColor:
                    return VK_BLEND_FACTOR_CONSTANT_COLOR;
                case dawn::BlendFactor::OneMinusBlendColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
                default:
                    UNREACHABLE();
            }
        }

        VkBlendOp VulkanBlendOperation(dawn::BlendOperation operation) {
            switch (operation) {
                case dawn::BlendOperation::Add:
                    return VK_BLEND_OP_ADD;
                case dawn::BlendOperation::Subtract:
                    return VK_BLEND_OP_SUBTRACT;
                case dawn::BlendOperation::ReverseSubtract:
                    return VK_BLEND_OP_REVERSE_SUBTRACT;
                case dawn::BlendOperation::Min:
                    return VK_BLEND_OP_MIN;
                case dawn::BlendOperation::Max:
                    return VK_BLEND_OP_MAX;
                default:
                    UNREACHABLE();
            }
        }

        VkColorComponentFlagBits VulkanColorWriteMask(dawn::ColorWriteMask mask) {
            // Vulkan and NXT color write masks match, static assert it and return the mask
            static_assert(static_cast<VkColorComponentFlagBits>(dawn::ColorWriteMask::Red) ==
                              VK_COLOR_COMPONENT_R_BIT,
                          "");
            static_assert(static_cast<VkColorComponentFlagBits>(dawn::ColorWriteMask::Green) ==
                              VK_COLOR_COMPONENT_G_BIT,
                          "");
            static_assert(static_cast<VkColorComponentFlagBits>(dawn::ColorWriteMask::Blue) ==
                              VK_COLOR_COMPONENT_B_BIT,
                          "");
            static_assert(static_cast<VkColorComponentFlagBits>(dawn::ColorWriteMask::Alpha) ==
                              VK_COLOR_COMPONENT_A_BIT,
                          "");

            return static_cast<VkColorComponentFlagBits>(mask);
        }
    }  // anonymous namespace

    BlendState::BlendState(BlendStateBuilder* builder) : BlendStateBase(builder) {
        // Fill the "color blend attachment info" that will be copied in an array and chained in
        // the pipeline create info.
        const auto& info = GetBlendInfo();

        mState.blendEnable = info.blendEnabled ? VK_TRUE : VK_FALSE;
        mState.srcColorBlendFactor = VulkanBlendFactor(info.colorBlend.srcFactor);
        mState.dstColorBlendFactor = VulkanBlendFactor(info.colorBlend.dstFactor);
        mState.colorBlendOp = VulkanBlendOperation(info.colorBlend.operation);
        mState.srcAlphaBlendFactor = VulkanBlendFactor(info.alphaBlend.srcFactor);
        mState.dstAlphaBlendFactor = VulkanBlendFactor(info.alphaBlend.dstFactor);
        mState.alphaBlendOp = VulkanBlendOperation(info.alphaBlend.operation);
        mState.colorWriteMask = VulkanColorWriteMask(info.colorWriteMask);
    }

    const VkPipelineColorBlendAttachmentState& BlendState::GetState() const {
        return mState;
    }

}}  // namespace backend::vulkan
