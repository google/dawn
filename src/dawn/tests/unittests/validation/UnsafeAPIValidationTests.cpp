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

namespace {
using testing::HasSubstr;
}  // anonymous namespace

class UnsafeAPIValidationTest : public ValidationTest {
  protected:
    WGPUDevice CreateTestDevice(dawn::native::Adapter dawnAdapter) override {
        wgpu::DeviceDescriptor descriptor;
        wgpu::DawnTogglesDescriptor deviceTogglesDesc;
        descriptor.nextInChain = &deviceTogglesDesc;
        const char* toggle = "disallow_unsafe_apis";
        deviceTogglesDesc.enabledToggles = &toggle;
        deviceTogglesDesc.enabledTogglesCount = 1;
        return dawnAdapter.CreateDevice(&descriptor);
    }
};

// Check chromium_disable_uniformity_analysis is an unsafe API.
TEST_F(UnsafeAPIValidationTest, chromium_disable_uniformity_analysis) {
    ASSERT_DEVICE_ERROR(utils::CreateShaderModule(device, R"(
        enable chromium_disable_uniformity_analysis;

        @compute @workgroup_size(8) fn uniformity_error(
            @builtin(local_invocation_id) local_invocation_id : vec3<u32>
        ) {
            if (local_invocation_id.x == 0u) {
                workgroupBarrier();
            }
        }
    )"));
}
