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

#ifndef SRC_DAWN_NATIVE_VULKAN_EXTERNALHANDLE_H_
#define SRC_DAWN_NATIVE_VULKAN_EXTERNALHANDLE_H_

#include "dawn/common/vulkan_platform.h"

namespace dawn::native::vulkan {

#if DAWN_PLATFORM_IS(ANDROID)
// AHardwareBuffer
using ExternalMemoryHandle = struct AHardwareBuffer*;
// File descriptor
using ExternalSemaphoreHandle = int;
const ExternalSemaphoreHandle kNullExternalSemaphoreHandle = -1;
#elif DAWN_PLATFORM_IS(LINUX)
// File descriptor
using ExternalMemoryHandle = int;
// File descriptor
using ExternalSemaphoreHandle = int;
const ExternalSemaphoreHandle kNullExternalSemaphoreHandle = -1;
#elif DAWN_PLATFORM_IS(FUCHSIA)
// Really a Zircon vmo handle.
using ExternalMemoryHandle = zx_handle_t;
// Really a Zircon event handle.
using ExternalSemaphoreHandle = zx_handle_t;
const ExternalSemaphoreHandle kNullExternalSemaphoreHandle = ZX_HANDLE_INVALID;
#else
// Generic types so that the Null service can compile, not used for real handles
using ExternalMemoryHandle = void*;
using ExternalSemaphoreHandle = void*;
const ExternalSemaphoreHandle kNullExternalSemaphoreHandle = nullptr;
#endif

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_EXTERNALHANDLE_H_
