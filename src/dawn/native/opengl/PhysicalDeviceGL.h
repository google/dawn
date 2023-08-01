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

#ifndef SRC_DAWN_NATIVE_OPENGL_PHYSICALDEVICEGL_H_
#define SRC_DAWN_NATIVE_OPENGL_PHYSICALDEVICEGL_H_

#include "dawn/native/PhysicalDevice.h"
#include "dawn/native/opengl/EGLFunctions.h"
#include "dawn/native/opengl/OpenGLFunctions.h"

namespace dawn::native::opengl {

class PhysicalDevice : public PhysicalDeviceBase {
  public:
    static ResultOrError<Ref<PhysicalDevice>> Create(InstanceBase* instance,
                                                     wgpu::BackendType backendType,
                                                     void* (*getProc)(const char*),
                                                     EGLDisplay display);

    ~PhysicalDevice() override = default;

    // PhysicalDeviceBase Implementation
    bool SupportsExternalImages() const override;
    bool SupportsFeatureLevel(FeatureLevel featureLevel) const override;

  private:
    PhysicalDevice(InstanceBase* instance, wgpu::BackendType backendType, EGLDisplay display);
    MaybeError InitializeGLFunctions(void* (*getProc)(const char*));

    MaybeError InitializeImpl() override;
    void InitializeSupportedFeaturesImpl() override;
    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override;

    MaybeError ValidateFeatureSupportedWithTogglesImpl(wgpu::FeatureName feature,
                                                       const TogglesState& toggles) const override;

    void SetupBackendAdapterToggles(TogglesState* adapterToggles) const override;
    void SetupBackendDeviceToggles(TogglesState* deviceToggles) const override;
    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(AdapterBase* adapter,
                                                    const DeviceDescriptor* descriptor,
                                                    const TogglesState& deviceToggles) override;

    OpenGLFunctions mFunctions;
    EGLDisplay mDisplay;
    EGLFunctions mEGLFunctions;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_PHYSICALDEVICEGL_H_
