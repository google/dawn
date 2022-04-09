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

#include "dawn/native/vulkan/CacheKeyVk.h"

#include <cstring>

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
        vulkan::SerializePnext<>(key, reinterpret_cast<const VkBaseOutStructure*>(&t));
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
        vulkan::SerializePnext<>(key, reinterpret_cast<const VkBaseOutStructure*>(&t));
    }

    template <>
    void CacheKeySerializer<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>::Serialize(
        CacheKey* key,
        const VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT& t) {
        key->Record(t.requiredSubgroupSize);
    }

    template <>
    void CacheKeySerializer<VkSpecializationMapEntry>::Serialize(
        CacheKey* key,
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
        vulkan::SerializePnext<VkPipelineShaderStageRequiredSubgroupSizeCreateInfoEXT>(
            key, reinterpret_cast<const VkBaseOutStructure*>(&t));
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

}  // namespace dawn::native
