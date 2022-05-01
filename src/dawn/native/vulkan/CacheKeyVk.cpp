// Copyright 2022 The Dawn Authors
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

#include <cstring>

#include "dawn/native/vulkan/CacheKeyVk.h"
#include "dawn/native/vulkan/RenderPassCache.h"

namespace dawn::native {

template <>
void CacheKeySerializer<VkDescriptorSetLayoutBinding>::Serialize(
    CacheKey* key,
    const VkDescriptorSetLayoutBinding& t) {
    key->Record(t.binding, t.descriptorType, t.descriptorCount, t.stageFlags);
}

template <>
void CacheKeySerializer<VkDescriptorSetLayoutCreateInfo>::Serialize(
    CacheKey* key,
    const VkDescriptorSetLayoutCreateInfo& t) {
    key->Record(t.flags).RecordIterable(t.pBindings, t.bindingCount);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPushConstantRange>::Serialize(CacheKey* key,
                                                        const VkPushConstantRange& t) {
    key->Record(t.stageFlags, t.offset, t.size);
}

template <>
void CacheKeySerializer<VkPipelineLayoutCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineLayoutCreateInfo& t) {
    // The set layouts are not serialized here because they are pointers to backend objects.
    // They need to be cross-referenced with the frontend objects and serialized from there.
    key->Record(t.flags).RecordIterable(t.pPushConstantRanges, t.pushConstantRangeCount);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>::Serialize(
    CacheKey* key,
    const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& t) {
    key->Record(t.requiredSubgroupSize);
}

template <>
void CacheKeySerializer<VkSpecializationMapEntry>::Serialize(CacheKey* key,
                                                             const VkSpecializationMapEntry& t) {
    key->Record(t.constantID, t.offset, t.size);
}

template <>
void CacheKeySerializer<VkSpecializationInfo>::Serialize(CacheKey* key,
                                                         const VkSpecializationInfo& t) {
    key->RecordIterable(t.pMapEntries, t.mapEntryCount)
        .RecordIterable(static_cast<const uint8_t*>(t.pData), t.dataSize);
}

template <>
void CacheKeySerializer<VkPipelineShaderStageCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineShaderStageCreateInfo& t) {
    // The shader module is not serialized here because it is a pointer to a backend object.
    key->Record(t.flags, t.stage)
        .RecordIterable(t.pName, strlen(t.pName))
        .Record(t.pSpecializationInfo);
    vulkan::SerializePnext<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>(key, &t);
}

template <>
void CacheKeySerializer<VkComputePipelineCreateInfo>::Serialize(
    CacheKey* key,
    const VkComputePipelineCreateInfo& t) {
    // The pipeline layout is not serialized here because it is a pointer to a backend object.
    // It needs to be cross-referenced with the frontend objects and serialized from there. The
    // base pipeline information is also currently not recorded since we do not use them in our
    // backend implementation. If we decide to use them later on, they also need to be
    // cross-referenced from the frontend.
    key->Record(t.flags, t.stage);
}

template <>
void CacheKeySerializer<VkVertexInputBindingDescription>::Serialize(
    CacheKey* key,
    const VkVertexInputBindingDescription& t) {
    key->Record(t.binding, t.stride, t.inputRate);
}

template <>
void CacheKeySerializer<VkVertexInputAttributeDescription>::Serialize(
    CacheKey* key,
    const VkVertexInputAttributeDescription& t) {
    key->Record(t.location, t.binding, t.format, t.offset);
}

template <>
void CacheKeySerializer<VkPipelineVertexInputStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineVertexInputStateCreateInfo& t) {
    key->Record(t.flags)
        .RecordIterable(t.pVertexBindingDescriptions, t.vertexBindingDescriptionCount)
        .RecordIterable(t.pVertexAttributeDescriptions, t.vertexAttributeDescriptionCount);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPipelineInputAssemblyStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineInputAssemblyStateCreateInfo& t) {
    key->Record(t.flags, t.topology, t.primitiveRestartEnable);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPipelineTessellationStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineTessellationStateCreateInfo& t) {
    key->Record(t.flags, t.patchControlPoints);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkViewport>::Serialize(CacheKey* key, const VkViewport& t) {
    key->Record(t.x, t.y, t.width, t.height, t.minDepth, t.maxDepth);
}

template <>
void CacheKeySerializer<VkOffset2D>::Serialize(CacheKey* key, const VkOffset2D& t) {
    key->Record(t.x, t.y);
}

template <>
void CacheKeySerializer<VkExtent2D>::Serialize(CacheKey* key, const VkExtent2D& t) {
    key->Record(t.width, t.height);
}

template <>
void CacheKeySerializer<VkRect2D>::Serialize(CacheKey* key, const VkRect2D& t) {
    key->Record(t.offset, t.extent);
}

template <>
void CacheKeySerializer<VkPipelineViewportStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineViewportStateCreateInfo& t) {
    key->Record(t.flags)
        .RecordIterable(t.pViewports, t.viewportCount)
        .RecordIterable(t.pScissors, t.scissorCount);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPipelineRasterizationStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineRasterizationStateCreateInfo& t) {
    key->Record(t.flags, t.depthClampEnable, t.rasterizerDiscardEnable, t.polygonMode, t.cullMode,
                t.frontFace, t.depthBiasEnable, t.depthBiasConstantFactor, t.depthBiasClamp,
                t.depthBiasSlopeFactor, t.lineWidth);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPipelineMultisampleStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineMultisampleStateCreateInfo& t) {
    key->Record(t.flags, t.rasterizationSamples, t.sampleShadingEnable, t.minSampleShading,
                t.pSampleMask, t.alphaToCoverageEnable, t.alphaToOneEnable);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkStencilOpState>::Serialize(CacheKey* key, const VkStencilOpState& t) {
    key->Record(t.failOp, t.passOp, t.depthFailOp, t.compareOp, t.compareMask, t.writeMask,
                t.reference);
}

template <>
void CacheKeySerializer<VkPipelineDepthStencilStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineDepthStencilStateCreateInfo& t) {
    key->Record(t.flags, t.depthTestEnable, t.depthWriteEnable, t.depthCompareOp,
                t.depthBoundsTestEnable, t.stencilTestEnable, t.front, t.back, t.minDepthBounds,
                t.maxDepthBounds);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPipelineColorBlendAttachmentState>::Serialize(
    CacheKey* key,
    const VkPipelineColorBlendAttachmentState& t) {
    key->Record(t.blendEnable, t.srcColorBlendFactor, t.dstColorBlendFactor, t.colorBlendOp,
                t.srcAlphaBlendFactor, t.dstAlphaBlendFactor, t.alphaBlendOp, t.colorWriteMask);
}

template <>
void CacheKeySerializer<VkPipelineColorBlendStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineColorBlendStateCreateInfo& t) {
    key->Record(t.flags, t.logicOpEnable, t.logicOp)
        .RecordIterable(t.pAttachments, t.attachmentCount)
        .Record(t.blendConstants);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<VkPipelineDynamicStateCreateInfo>::Serialize(
    CacheKey* key,
    const VkPipelineDynamicStateCreateInfo& t) {
    key->Record(t.flags).RecordIterable(t.pDynamicStates, t.dynamicStateCount);
    vulkan::SerializePnext<>(key, &t);
}

template <>
void CacheKeySerializer<vulkan::RenderPassCacheQuery>::Serialize(
    CacheKey* key,
    const vulkan::RenderPassCacheQuery& t) {
    key->Record(t.colorMask.to_ulong(), t.resolveTargetMask.to_ulong(), t.sampleCount);

    // Manually iterate the color attachment indices and their corresponding format/load/store
    // ops because the data is sparse and may be uninitialized. Since we record the colorMask
    // member above, recording sparse data should be fine here.
    for (ColorAttachmentIndex i : IterateBitSet(t.colorMask)) {
        key->Record(t.colorFormats[i], t.colorLoadOp[i], t.colorStoreOp[i]);
    }

    // Serialize the depth-stencil toggle bit, and the parameters if applicable.
    key->Record(t.hasDepthStencil);
    if (t.hasDepthStencil) {
        key->Record(t.depthStencilFormat, t.depthLoadOp, t.depthStoreOp, t.stencilLoadOp,
                    t.stencilStoreOp, t.readOnlyDepthStencil);
    }
}

template <>
void CacheKeySerializer<VkGraphicsPipelineCreateInfo>::Serialize(
    CacheKey* key,
    const VkGraphicsPipelineCreateInfo& t) {
    // The pipeline layout and render pass are not serialized here because they are pointers to
    // backend objects. They need to be cross-referenced with the frontend objects and
    // serialized from there. The base pipeline information is also currently not recorded since
    // we do not use them in our backend implementation. If we decide to use them later on, they
    // also need to be cross-referenced from the frontend.
    key->Record(t.flags)
        .RecordIterable(t.pStages, t.stageCount)
        .Record(t.pVertexInputState, t.pInputAssemblyState, t.pTessellationState, t.pViewportState,
                t.pRasterizationState, t.pMultisampleState, t.pDepthStencilState,
                t.pColorBlendState, t.pDynamicState, t.subpass);
    vulkan::SerializePnext<>(key, &t);
}

}  // namespace dawn::native
