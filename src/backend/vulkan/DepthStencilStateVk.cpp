// Copyright 2018 The NXT Authors
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

#include "backend/vulkan/DepthStencilStateVk.h"

#include "common/Assert.h"

namespace backend { namespace vulkan {

    namespace {
        VkCompareOp VulkanCompareOp(nxt::CompareFunction op) {
            switch (op) {
                case nxt::CompareFunction::Always:
                    return VK_COMPARE_OP_ALWAYS;
                case nxt::CompareFunction::Equal:
                    return VK_COMPARE_OP_EQUAL;
                case nxt::CompareFunction::Greater:
                    return VK_COMPARE_OP_GREATER;
                case nxt::CompareFunction::GreaterEqual:
                    return VK_COMPARE_OP_GREATER_OR_EQUAL;
                case nxt::CompareFunction::Less:
                    return VK_COMPARE_OP_LESS;
                case nxt::CompareFunction::LessEqual:
                    return VK_COMPARE_OP_LESS_OR_EQUAL;
                case nxt::CompareFunction::Never:
                    return VK_COMPARE_OP_NEVER;
                case nxt::CompareFunction::NotEqual:
                    return VK_COMPARE_OP_NOT_EQUAL;
                default:
                    UNREACHABLE();
            }
        }

        VkStencilOp VulkanStencilOp(nxt::StencilOperation op) {
            switch (op) {
                case nxt::StencilOperation::Keep:
                    return VK_STENCIL_OP_KEEP;
                case nxt::StencilOperation::Zero:
                    return VK_STENCIL_OP_ZERO;
                case nxt::StencilOperation::Replace:
                    return VK_STENCIL_OP_REPLACE;
                case nxt::StencilOperation::IncrementClamp:
                    return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
                case nxt::StencilOperation::DecrementClamp:
                    return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
                case nxt::StencilOperation::Invert:
                    return VK_STENCIL_OP_INVERT;
                case nxt::StencilOperation::IncrementWrap:
                    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
                case nxt::StencilOperation::DecrementWrap:
                    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
                default:
                    UNREACHABLE();
            }
        }

    }  // anonymous namespace

    DepthStencilState::DepthStencilState(DepthStencilStateBuilder* builder)
        : DepthStencilStateBase(builder) {
        mCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        mCreateInfo.pNext = nullptr;
        mCreateInfo.flags = 0;

        const auto& depth = GetDepth();
        mCreateInfo.depthTestEnable = VK_TRUE;
        mCreateInfo.depthWriteEnable = depth.depthWriteEnabled ? VK_TRUE : VK_FALSE;
        mCreateInfo.depthCompareOp = VulkanCompareOp(depth.compareFunction);
        mCreateInfo.depthBoundsTestEnable = false;
        mCreateInfo.minDepthBounds = 0.0f;
        mCreateInfo.maxDepthBounds = 0.0f;

        const auto& stencil = GetStencil();
        mCreateInfo.stencilTestEnable = StencilTestEnabled() ? VK_TRUE : VK_FALSE;

        mCreateInfo.front.failOp = VulkanStencilOp(stencil.front.stencilFail);
        mCreateInfo.front.passOp = VulkanStencilOp(stencil.front.depthStencilPass);
        mCreateInfo.front.depthFailOp = VulkanStencilOp(stencil.front.depthFail);
        mCreateInfo.front.compareOp = VulkanCompareOp(stencil.front.compareFunction);

        mCreateInfo.back.failOp = VulkanStencilOp(stencil.back.stencilFail);
        mCreateInfo.back.passOp = VulkanStencilOp(stencil.back.depthStencilPass);
        mCreateInfo.back.depthFailOp = VulkanStencilOp(stencil.back.depthFail);
        mCreateInfo.back.compareOp = VulkanCompareOp(stencil.back.compareFunction);

        // NXT doesn't have separate front and back stencil masks.
        mCreateInfo.front.compareMask = stencil.readMask;
        mCreateInfo.back.compareMask = stencil.readMask;
        mCreateInfo.front.writeMask = stencil.writeMask;
        mCreateInfo.back.writeMask = stencil.writeMask;

        // The stencil reference is always dynamic
        mCreateInfo.front.reference = 0;
        mCreateInfo.back.reference = 0;
    }

    const VkPipelineDepthStencilStateCreateInfo* DepthStencilState::GetCreateInfo() const {
        return &mCreateInfo;
    }

}}  // namespace backend::vulkan
