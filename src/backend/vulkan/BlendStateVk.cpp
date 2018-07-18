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
        VkBlendFactor VulkanBlendFactor(nxt::BlendFactor factor) {
            switch (factor) {
                case nxt::BlendFactor::Zero:
                    return VK_BLEND_FACTOR_ZERO;
                case nxt::BlendFactor::One:
                    return VK_BLEND_FACTOR_ONE;
                case nxt::BlendFactor::SrcColor:
                    return VK_BLEND_FACTOR_SRC_COLOR;
                case nxt::BlendFactor::OneMinusSrcColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
                case nxt::BlendFactor::SrcAlpha:
                    return VK_BLEND_FACTOR_SRC_ALPHA;
                case nxt::BlendFactor::OneMinusSrcAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
                case nxt::BlendFactor::DstColor:
                    return VK_BLEND_FACTOR_DST_COLOR;
                case nxt::BlendFactor::OneMinusDstColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
                case nxt::BlendFactor::DstAlpha:
                    return VK_BLEND_FACTOR_DST_ALPHA;
                case nxt::BlendFactor::OneMinusDstAlpha:
                    return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
                case nxt::BlendFactor::SrcAlphaSaturated:
                    return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
                case nxt::BlendFactor::BlendColor:
                    return VK_BLEND_FACTOR_CONSTANT_COLOR;
                case nxt::BlendFactor::OneMinusBlendColor:
                    return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
                default:
                    UNREACHABLE();
            }
        }

        VkBlendOp VulkanBlendOperation(nxt::BlendOperation operation) {
            switch (operation) {
                case nxt::BlendOperation::Add:
                    return VK_BLEND_OP_ADD;
                case nxt::BlendOperation::Subtract:
                    return VK_BLEND_OP_SUBTRACT;
                case nxt::BlendOperation::ReverseSubtract:
                    return VK_BLEND_OP_REVERSE_SUBTRACT;
                case nxt::BlendOperation::Min:
                    return VK_BLEND_OP_MIN;
                case nxt::BlendOperation::Max:
                    return VK_BLEND_OP_MAX;
                default:
                    UNREACHABLE();
            }
        }

        VkColorComponentFlagBits VulkanColorWriteMask(nxt::ColorWriteMask mask) {
            // Vulkan and NXT color write masks match, static assert it and return the mask
            static_assert(static_cast<VkColorComponentFlagBits>(nxt::ColorWriteMask::Red) ==
                              VK_COLOR_COMPONENT_R_BIT,
                          "");
            static_assert(static_cast<VkColorComponentFlagBits>(nxt::ColorWriteMask::Green) ==
                              VK_COLOR_COMPONENT_G_BIT,
                          "");
            static_assert(static_cast<VkColorComponentFlagBits>(nxt::ColorWriteMask::Blue) ==
                              VK_COLOR_COMPONENT_B_BIT,
                          "");
            static_assert(static_cast<VkColorComponentFlagBits>(nxt::ColorWriteMask::Alpha) ==
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
