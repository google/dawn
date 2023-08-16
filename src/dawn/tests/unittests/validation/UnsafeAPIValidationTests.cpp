// Copyright 2020 The Dawn Authors
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

#include <vector>

#include "dawn/tests/MockCallback.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/ComboRenderBundleEncoderDescriptor.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

using testing::HasSubstr;

class UnsafeAPIValidationTest : public ValidationTest {
  protected:
    // UnsafeAPIValidationTest create the device with the AllowUnsafeAPIs toggle explicitly
    // disabled, which overrides the inheritance.
    WGPUDevice CreateTestDevice(native::Adapter dawnAdapter,
                                wgpu::DeviceDescriptor descriptor) override {
        // Disable the AllowUnsafeAPIs toggles in device toggles descriptor to override the
        // inheritance and create a device disallowing unsafe apis.
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        descriptor.nextInChain = &deviceTogglesDesc;
        const char* toggle = "allow_unsafe_apis";
        deviceTogglesDesc.disabledToggles = &toggle;
        deviceTogglesDesc.disabledToggleCount = 1;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Check chromium_disable_uniformity_analysis is an unsafe API.
TEST_F(UnsafeAPIValidationTest, chromium_disable_uniformity_analysis) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_disable_uniformity_analysis;

        @compute @workgroup_size(8) fn uniformity_error(
            @builtin(local_invocation_id) local_invocation_id : vec3u
        ) {
            if (local_invocation_id.x == 0u) {
                workgroupBarrier();
            }
        }
    )"));
}

}  // anonymous namespace
}  // namespace dawn
