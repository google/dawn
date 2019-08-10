#ifndef DAWNNATIVE_VULKAN_EXTERNALHANDLE_H_
#define DAWNNATIVE_VULKAN_EXTERNALHANDLE_H_

namespace dawn_native { namespace vulkan {

#ifdef DAWN_PLATFORM_LINUX
    // File descriptor
    using ExternalMemoryHandle = int;
    // File descriptor
    using ExternalSemaphoreHandle = int;
#else
    // Generic types so that the Null service can compile, not used for real handles
    using ExternalMemoryHandle = void*;
    using ExternalSemaphoreHandle = void*;
#endif

}}  // namespace dawn_native::vulkan

#endif  // DAWNNATIVE_VULKAN_EXTERNALHANDLE_H_
