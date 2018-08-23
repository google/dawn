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

#if !defined(DAWN_ENABLE_BACKEND_VULKAN)
#    error "vulkan_platform.h included without the Vulkan backend enabled"
#endif

#include "common/Platform.h"

#include <cstddef>
#include <cstdint>

// vulkan.h defines non-dispatchable handles to opaque pointers on 64bit architectures and uint64_t
// on 32bit architectures. This causes a problem in 32bit where the handles cannot be used to
// distinguish between overloads of the same function.
// Change the definition of non-dispatchable handles to be opaque structures containing a uint64_t
// and overload the comparison operators between themselves and VK_NULL_HANDLE (which will be
// redefined to be nullptr). This keeps the type-safety of having the handles be different types
// (like vulkan.h on 64 bit) but makes sure the types are different on 32 bit architectures.

// Force the handle type to have the same alignment as what would have been the Vulkan
// non-dispatchable handle type.
#if defined(DAWN_PLATFORM_64_BIT)
// In 64 bit handles are pointers to some structure, we just declare one inline here.
using NativeVulkanHandleType = struct VkSomeHandle*;
#elif defined(DAWN_PLATFORM_32_BIT)
using NativeVulkanHandleType = uint64_t;
#    define ALIGNAS_VULKAN_HANDLE alignas(alignof(uint64_t))
#else
#    error "Unsupported platform"
#endif

// Simple handle types that supports "nullptr_t" as a 0 value.
template <typename Tag, typename HandleType>
class alignas(alignof(NativeVulkanHandleType)) VkNonDispatchableHandle {
  public:
    // Default constructor and assigning of VK_NULL_HANDLE
    VkNonDispatchableHandle() = default;
    VkNonDispatchableHandle(std::nullptr_t) : mHandle(0) {
    }

    // Use default copy constructor/assignment
    VkNonDispatchableHandle(const VkNonDispatchableHandle<Tag, HandleType>& other) = default;
    VkNonDispatchableHandle& operator=(const VkNonDispatchableHandle<Tag, HandleType>&) = default;

    // Comparisons between handles
    bool operator==(VkNonDispatchableHandle<Tag, HandleType> other) {
        return mHandle == other.mHandle;
    }
    bool operator!=(VkNonDispatchableHandle<Tag, HandleType> other) {
        return mHandle != other.mHandle;
    }

    // Comparisons between handles and VK_NULL_HANDLE
    bool operator==(std::nullptr_t) {
        return mHandle == 0;
    }
    bool operator!=(std::nullptr_t) {
        return mHandle != 0;
    }

    // The regular Vulkan handle type depends on the pointer width but is always 64 bits wide.
    //  - On 64bit it is an opaque pointer type, probably to help with type safety
    //  - On 32bit it is a uint64_t because pointers aren't wide enough (and non dispatchable
    //    handles can be optimized to not be pointer but contain GPU virtual addresses or the
    //    data in a packed form).
    // Because of this we need two types of conversions from our handle type: to uint64_t and to
    // the "native" Vulkan type that may not be an uint64_t

    static VkNonDispatchableHandle<Tag, HandleType> CreateFromU64(uint64_t handle) {
        return {handle};
    }

    uint64_t GetU64() const {
        return mHandle;
    }

#if defined(DAWN_PLATFORM_64_BIT)
    static VkNonDispatchableHandle<Tag, HandleType> CreateFromHandle(HandleType handle) {
        return CreateFromU64(static_cast<uint64_t>(reinterpret_cast<intptr_t>(handle)));
    }

    HandleType GetHandle() const {
        return mHandle;
    }
#elif defined(DAWN_PLATFORM_32_BIT)
    static VkNonDispatchableHandle<Tag, HandleType> CreateFromHandle(HandleType handle) {
        return {handle};
    }

    HandleType GetHandle() const {
        return mHandle;
    }
#else
#    error "Unsupported platform"
#endif

  private:
    VkNonDispatchableHandle(uint64_t handle) : mHandle(handle) {
    }

    uint64_t mHandle = 0;
};

#if defined(DAWN_PLATFORM_64_BIT)
#    define DAWN_DEFINE_NATIVE_NON_DISPATCHABLE_HANDLE(object) \
        using object##Native = struct object##_T*;
#elif defined(DAWN_PLATFORM_32_BIT)
#    define DAWN_DEFINE_NATIVE_NON_DISPATCHABLE_HANDLE(object) using object##Native = uint64_t;
#else
#    error "Unsupported platform"
#endif

#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object)                          \
    struct VkTag##object;                                                  \
    DAWN_DEFINE_NATIVE_NON_DISPATCHABLE_HANDLE(object)                     \
    using object = VkNonDispatchableHandle<VkTag##object, object##Native>; \
    static_assert(sizeof(object) == sizeof(uint64_t), "");                 \
    static_assert(alignof(object) == alignof(uint64_t), "");               \
    static_assert(sizeof(object) == sizeof(object##Native), "");           \
    static_assert(alignof(object) == alignof(object##Native), "");

#    include <vulkan/vulkan.h>

    // VK_NULL_HANDLE is defined to 0 but we don't want our handle type to compare to arbitrary
    // integers. Redefine VK_NULL_HANDLE to nullptr that has its own type.
#    undef VK_NULL_HANDLE
#    define VK_NULL_HANDLE nullptr

// Remove windows.h macros after vulkan_platform's include of windows.h
#if defined(DAWN_PLATFORM_WINDOWS)
#    include "common/windows_with_undefs.h"
#endif

#endif  // COMMON_VULKANPLATFORM_H_
