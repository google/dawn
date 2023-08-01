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

#ifndef SRC_DAWN_NATIVE_OPENGL_BACKENDGL_H_
#define SRC_DAWN_NATIVE_OPENGL_BACKENDGL_H_

#include <vector>

#include "dawn/common/DynamicLib.h"
#include "dawn/native/BackendConnection.h"

using EGLDisplay = void*;

namespace dawn::native::opengl {

class PhysicalDevice;
class Backend : public BackendConnection {
  public:
    Backend(InstanceBase* instance, wgpu::BackendType backendType);

    std::vector<Ref<PhysicalDeviceBase>> DiscoverPhysicalDevices(
        const RequestAdapterOptions* options) override;
    void ClearPhysicalDevices() override;
    size_t GetPhysicalDeviceCountForTesting() const override;

  private:
    std::vector<Ref<PhysicalDeviceBase>> DiscoverPhysicalDevicesWithProcs(
        void* (*getProc)(const char*),
        EGLDisplay display);

    Ref<PhysicalDevice> mPhysicalDevice = nullptr;
    void* (*mGetProc)(const char*);
    DynamicLib mLibEGL;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_BACKENDGL_H_
