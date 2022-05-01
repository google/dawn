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

#ifndef SRC_DAWN_NATIVE_VULKAN_CACHEKEYVK_H_
#define SRC_DAWN_NATIVE_VULKAN_CACHEKEYVK_H_

#include <map>

#include "dawn/common/Assert.h"
#include "dawn/common/vulkan_platform.h"
#include "dawn/native/CacheKey.h"

#include "icd/generated/vk_typemap_helper.h"

namespace dawn::native::vulkan {

namespace detail {

template <typename... VK_STRUCT_TYPES>
void ValidatePnextImpl(const VkBaseOutStructure* root) {
    const VkBaseOutStructure* next = reinterpret_cast<const VkBaseOutStructure*>(root->pNext);
    while (next != nullptr) {
        // Assert that the type of each pNext struct is exactly one of the specified
        // templates.
        ASSERT(((LvlTypeMap<VK_STRUCT_TYPES>::kSType == next->sType ? 1 : 0) + ... + 0) == 1);
        next = reinterpret_cast<const VkBaseOutStructure*>(next->pNext);
    }
}

template <typename VK_STRUCT_TYPE>
void SerializePnextImpl(CacheKey* key, const VkBaseOutStructure* root) {
    const VkBaseOutStructure* next = reinterpret_cast<const VkBaseOutStructure*>(root->pNext);
    const VK_STRUCT_TYPE* found = nullptr;
    while (next != nullptr) {
        if (LvlTypeMap<VK_STRUCT_TYPE>::kSType == next->sType) {
            if (found == nullptr) {
                found = reinterpret_cast<const VK_STRUCT_TYPE*>(next);
            } else {
                // Fail an assert here since that means that the chain had more than one of
                // the same typed chained object.
                ASSERT(false);
            }
        }
        next = reinterpret_cast<const VkBaseOutStructure*>(next->pNext);
    }
    if (found != nullptr) {
        key->Record(found);
    }
}

template <typename VK_STRUCT_TYPE,
          typename... VK_STRUCT_TYPES,
          typename = std::enable_if_t<(sizeof...(VK_STRUCT_TYPES) > 0)>>
void SerializePnextImpl(CacheKey* key, const VkBaseOutStructure* root) {
    SerializePnextImpl<VK_STRUCT_TYPE>(key, root);
    SerializePnextImpl<VK_STRUCT_TYPES...>(key, root);
}

template <typename VK_STRUCT_TYPE>
const VkBaseOutStructure* ToVkBaseOutStructure(const VK_STRUCT_TYPE* t) {
    // Checks to ensure proper type safety.
    static_assert(offsetof(VK_STRUCT_TYPE, sType) == offsetof(VkBaseOutStructure, sType) &&
                      offsetof(VK_STRUCT_TYPE, pNext) == offsetof(VkBaseOutStructure, pNext),
                  "Argument type is not a proper Vulkan structure type");
    return reinterpret_cast<const VkBaseOutStructure*>(t);
}

}  // namespace detail

template <typename... VK_STRUCT_TYPES,
          typename VK_STRUCT_TYPE,
          typename = std::enable_if_t<(sizeof...(VK_STRUCT_TYPES) > 0)>>
void SerializePnext(CacheKey* key, const VK_STRUCT_TYPE* t) {
    const VkBaseOutStructure* root = detail::ToVkBaseOutStructure(t);
    detail::ValidatePnextImpl<VK_STRUCT_TYPES...>(root);
    detail::SerializePnextImpl<VK_STRUCT_TYPES...>(key, root);
}

// Empty template specialization so that we can put this in to ensure failures occur if new
// extensions are added without updating serialization.
template <typename VK_STRUCT_TYPE>
void SerializePnext(CacheKey* key, const VK_STRUCT_TYPE* t) {
    const VkBaseOutStructure* root = detail::ToVkBaseOutStructure(t);
    detail::ValidatePnextImpl<>(root);
}

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_CACHEKEYVK_H_
