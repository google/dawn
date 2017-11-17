// Copyright 2017 The NXT Authors
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

#include "backend/vulkan/VulkanFunctions.h"

#include "common/DynamicLib.h"

namespace backend {
namespace vulkan {

    #define GET_GLOBAL_PROC(name) \
        name = reinterpret_cast<decltype(name)>(GetInstanceProcAddr(nullptr, "vk" #name)); \
        if (name == nullptr) { \
            return false; \
        }

    bool VulkanFunctions::LoadGlobalProcs(const DynamicLib& vulkanLib) {
        if (!vulkanLib.GetProc(&GetInstanceProcAddr, "vkGetInstanceProcAddr")) {
            return false;
        }

        GET_GLOBAL_PROC(CreateInstance);
        GET_GLOBAL_PROC(DestroyInstance);
        GET_GLOBAL_PROC(EnumerateInstanceExtensionProperties);
        GET_GLOBAL_PROC(EnumerateInstanceLayerProperties);

        return true;
    }

}
}
