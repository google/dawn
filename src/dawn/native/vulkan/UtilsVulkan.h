// Copyright 2019 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_VULKAN_UTILSVULKAN_H_
#define SRC_DAWN_NATIVE_VULKAN_UTILSVULKAN_H_

#include <string>
#include <vector>

#include "dawn/common/vulkan_platform.h"
#include "dawn/native/Commands.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {
struct ProgrammableStage;
union OverrideScalar;
}  // namespace dawn::native

namespace dawn::native::vulkan {

class Device;

// A Helper type used to build a pNext chain of extension structs.
// Usage is:
//   1) Create instance, passing the address of the first struct in the chain. This requires
//      pNext to be nullptr. If you already have a chain you need to pass a pointer to the tail
//      of it.
//
//   2) Call Add(&vk_struct) every time a new struct needs to be appended to the chain.
//
//   3) Alternatively, call Add(&vk_struct, VK_STRUCTURE_TYPE_XXX) to initialize the struct
//      with a given VkStructureType value while appending it to the chain.
//
// Examples:
//     VkPhysicalFeatures2 features2 = {
//       .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
//       .pNext = nullptr,
//     };
//
//     PNextChainBuilder featuresChain(&features2);
//
//     featuresChain.Add(&featuresExtensions.subgroupSizeControl,
//                       VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SUBGROUP_SIZE_CONTROL_FEATURES_EXT);
//
struct PNextChainBuilder {
    // Constructor takes the address of a Vulkan structure instance, and
    // walks its pNext chain to record the current location of its tail.
    //
    // NOTE: Some VK_STRUCT_TYPEs define their pNext field as a const void*
    // which is why the VkBaseOutStructure* casts below are necessary.
    template <typename VK_STRUCT_TYPE>
    explicit PNextChainBuilder(VK_STRUCT_TYPE* head)
        : mCurrent(reinterpret_cast<VkBaseOutStructure*>(head)) {
        while (mCurrent->pNext != nullptr) {
            mCurrent = mCurrent->pNext;
        }
    }

    // Add one item to the chain. |vk_struct| must be a Vulkan structure
    // that is already initialized.
    template <typename VK_STRUCT_TYPE>
    void Add(VK_STRUCT_TYPE* vkStruct) {
        // Checks to ensure proper type safety.
        static_assert(offsetof(VK_STRUCT_TYPE, sType) == offsetof(VkBaseOutStructure, sType) &&
                          offsetof(VK_STRUCT_TYPE, pNext) == offsetof(VkBaseOutStructure, pNext),
                      "Argument type is not a proper Vulkan structure type");
        vkStruct->pNext = nullptr;

        mCurrent->pNext = reinterpret_cast<VkBaseOutStructure*>(vkStruct);
        mCurrent = mCurrent->pNext;
    }

    // A variant of Add() above that also initializes the |sType| field in |vk_struct|.
    template <typename VK_STRUCT_TYPE>
    void Add(VK_STRUCT_TYPE* vkStruct, VkStructureType sType) {
        vkStruct->sType = sType;
        Add(vkStruct);
    }

  private:
    VkBaseOutStructure* mCurrent;
};

VkCompareOp ToVulkanCompareOp(wgpu::CompareFunction op);

VkImageAspectFlags VulkanAspectMask(const Aspect& aspects);

Extent3D ComputeTextureCopyExtent(const TextureCopy& textureCopy, const Extent3D& copySize);

VkBufferImageCopy ComputeBufferImageCopyRegion(const BufferCopy& bufferCopy,
                                               const TextureCopy& textureCopy,
                                               const Extent3D& copySize);
VkBufferImageCopy ComputeBufferImageCopyRegion(const TextureDataLayout& dataLayout,
                                               const TextureCopy& textureCopy,
                                               const Extent3D& copySize);

// Gets the associated VkObjectType for any non-dispatchable handle
template <class HandleType>
VkObjectType GetVkObjectType(HandleType handle);

void SetDebugNameInternal(Device* device,
                          VkObjectType objectType,
                          uint64_t objectHandle,
                          const char* prefix,
                          std::string label);

// The majority of Vulkan handles are "non-dispatchable". Dawn wraps these by overriding
// VK_DEFINE_NON_DISPATCHABLE_HANDLE to add some capabilities like making null comparisons
// easier. In those cases we can make setting the debug name a bit easier by getting the
// object type automatically and handling the indirection to the native handle.
template <typename Tag, typename HandleType>
void SetDebugName(Device* device,
                  detail::VkHandle<Tag, HandleType> objectHandle,
                  const char* prefix,
                  std::string label = "") {
    uint64_t handle;
    if constexpr (std::is_same_v<HandleType, uint64_t>) {
        handle = objectHandle.GetHandle();
    } else {
        handle = reinterpret_cast<uint64_t>(objectHandle.GetHandle());
    }
    SetDebugNameInternal(device, GetVkObjectType(objectHandle), handle, prefix, label);
}

// Handles like VkQueue and VKDevice require a special path because they are dispatchable, so
// they require an explicit VkObjectType and cast to a uint64_t directly rather than by getting
// the non-dispatchable wrapper's underlying handle.
template <typename HandleType>
void SetDebugName(Device* device,
                  VkObjectType objectType,
                  HandleType objectHandle,
                  const char* prefix,
                  std::string label = "") {
    SetDebugNameInternal(device, objectType, reinterpret_cast<uint64_t>(objectHandle), prefix,
                         label);
}

std::string GetNextDeviceDebugPrefix();
std::string GetDeviceDebugPrefixFromDebugName(const char* debugName);

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_UTILSVULKAN_H_
