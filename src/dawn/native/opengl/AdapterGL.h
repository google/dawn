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

#ifndef SRC_DAWN_NATIVE_OPENGL_ADAPTERGL_H_
#define SRC_DAWN_NATIVE_OPENGL_ADAPTERGL_H_

#include "dawn/native/Adapter.h"
#include "dawn/native/opengl/EGLFunctions.h"
#include "dawn/native/opengl/OpenGLFunctions.h"

namespace dawn::native::opengl {

class Adapter : public AdapterBase {
  public:
    Adapter(InstanceBase* instance, wgpu::BackendType backendType);

    MaybeError InitializeGLFunctions(void* (*getProc)(const char*));

    ~Adapter() override = default;

    // AdapterBase Implementation
    bool SupportsExternalImages() const override;

  private:
    MaybeError InitializeImpl() override;
    void InitializeSupportedFeaturesImpl() override;
    MaybeError InitializeSupportedLimitsImpl(CombinedLimits* limits) override;

    MaybeError ValidateFeatureSupportedWithDeviceTogglesImpl(
        wgpu::FeatureName feature,
        const TogglesState& deviceTogglesState) override;

    void SetupBackendDeviceToggles(TogglesState* deviceToggles) const override;
    ResultOrError<Ref<DeviceBase>> CreateDeviceImpl(const DeviceDescriptor* descriptor,
                                                    const TogglesState& deviceToggles) override;

    OpenGLFunctions mFunctions;
    EGLFunctions mEGLFunctions;
};

}  // namespace dawn::native::opengl

#endif  // SRC_DAWN_NATIVE_OPENGL_ADAPTERGL_H_
