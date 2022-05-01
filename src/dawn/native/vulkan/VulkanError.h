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

#ifndef SRC_DAWN_NATIVE_VULKAN_VULKANERROR_H_
#define SRC_DAWN_NATIVE_VULKAN_VULKANERROR_H_

#include "dawn/native/ErrorInjector.h"
#include "dawn/native/vulkan/VulkanFunctions.h"

constexpr VkResult VK_FAKE_ERROR_FOR_TESTING = VK_RESULT_MAX_ENUM;
constexpr VkResult VK_FAKE_DEVICE_OOM_FOR_TESTING = static_cast<VkResult>(VK_RESULT_MAX_ENUM - 1);

namespace dawn::native::vulkan {

// Returns a string version of the result.
const char* VkResultAsString(::VkResult result);

MaybeError CheckVkSuccessImpl(VkResult result, const char* context);
MaybeError CheckVkOOMThenSuccessImpl(VkResult result, const char* context);

// Returns a success only if result if VK_SUCCESS, an error with the context and stringified
// result value instead. Can be used like this:
//
//   DAWN_TRY(CheckVkSuccess(vkDoSomething, "doing something"));
#define CheckVkSuccess(resultIn, contextIn)                            \
    ::dawn::native::vulkan::CheckVkSuccessImpl(                        \
        ::dawn::native::vulkan::VkResult::WrapUnsafe(                  \
            INJECT_ERROR_OR_RUN(resultIn, VK_FAKE_ERROR_FOR_TESTING)), \
        contextIn)

#define CheckVkOOMThenSuccess(resultIn, contextIn)                                 \
    ::dawn::native::vulkan::CheckVkOOMThenSuccessImpl(                             \
        ::dawn::native::vulkan::VkResult::WrapUnsafe(INJECT_ERROR_OR_RUN(          \
            resultIn, VK_FAKE_DEVICE_OOM_FOR_TESTING, VK_FAKE_ERROR_FOR_TESTING)), \
        contextIn)

}  // namespace dawn::native::vulkan

#endif  // SRC_DAWN_NATIVE_VULKAN_VULKANERROR_H_
