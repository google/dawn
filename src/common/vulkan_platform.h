// Copyright 2017 The Dawn Authors
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

#ifndef COMMON_VULKANPLATFORM_H_
#define COMMON_VULKANPLATFORM_H_

#if !defined(NXT_ENABLE_BACKEND_VULKAN)
#    error "vulkan_platform.h included without the Vulkan backend enabled"
#endif

#include <cstddef>
#include <cstdint>

// vulkan.h defines non-dispatchable handles to opaque pointers on 64bit architectures and uint64_t
// on 32bit architectures. This causes a problem in 32bit where the handles cannot be used to
// distinguish between overloads of the same function.
// Change the definition of non-dispatchable handles to be opaque structures containing a uint64_t
// and overload the comparison operators between themselves and VK_NULL_HANDLE (which will be
// redefined to be nullptr). This keeps the type-safety of having the handles be different types
// (like vulkan.h on 64 bit) but makes sure the types are different on 32 bit architectures.

// Simple handle types that supports "nullptr_t" as a 0 value.
template <typename Tag>
class VkNonDispatchableHandle {
  public:
    // Default constructor and assigning of VK_NULL_HANDLE
    VkNonDispatchableHandle() = default;
    VkNonDispatchableHandle(std::nullptr_t) : mHandle(0) {
    }

    // Use default copy constructor/assignment
    VkNonDispatchableHandle(const VkNonDispatchableHandle<Tag>& other) = default;
    VkNonDispatchableHandle& operator=(const VkNonDispatchableHandle<Tag>&) = default;

    // Comparisons between handles
    bool operator==(VkNonDispatchableHandle<Tag> other) {
        return mHandle == other.mHandle;
    }
    bool operator!=(VkNonDispatchableHandle<Tag> other) {
        return mHandle != other.mHandle;
    }

    // Comparisons between handles and VK_NULL_HANDLE
    bool operator==(std::nullptr_t) {
        return mHandle == 0;
    }
    bool operator!=(std::nullptr_t) {
        return mHandle != 0;
    }

    static VkNonDispatchableHandle<Tag> CreateFromHandle(uint64_t handle) {
        return {handle};
    }

    uint64_t GetHandle() const {
        return mHandle;
    }

  private:
    VkNonDispatchableHandle(uint64_t handle) : mHandle(handle) {
    }

    uint64_t mHandle = 0;
};

#    define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)          \
        struct VkTag##object;                                  \
        using object = VkNonDispatchableHandle<VkTag##object>; \
        static_assert(sizeof(object) == sizeof(uint64_t), ""); \
        static_assert(alignof(object) == alignof(uint64_t), "");

#    include <vulkan/vulkan.h>

    // VK_NULL_HANDLE is defined to 0 but we don't want our handle type to compare to arbitrary
    // integers. Redefine VK_NULL_HANDLE to nullptr that has its own type.
#    undef VK_NULL_HANDLE
#    define VK_NULL_HANDLE nullptr

// Remove windows.h macros after vulkan_platform's include of windows.h
#include "common/Platform.h"
#if defined(DAWN_PLATFORM_WINDOWS)
#    include "common/windows_with_undefs.h"
#endif

#endif  // COMMON_VULKANPLATFORM_H_
