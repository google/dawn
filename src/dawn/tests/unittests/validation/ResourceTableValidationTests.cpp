// Copyright 2025 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <utility>
#include <vector>

#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

class ResourceTableValidationTest : public ValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        return {wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable};
    }
};

class ResourceTableValidationTestDisabled : public ValidationTest {
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override { return {}; }
};

// Test that validates that the feature must be enabled
TEST_F(ResourceTableValidationTestDisabled, FeatureNotEnabled) {
    wgpu::ResourceTableDescriptor descriptor;
    ASSERT_DEVICE_ERROR(device.CreateResourceTable(&descriptor));
}

// Test that setting invalid size is an error
TEST_F(ResourceTableValidationTest, InvalidSize) {
    wgpu::ResourceTableDescriptor descriptor;

    // Size 0 is valid
    descriptor.size = 0u;
    device.CreateResourceTable(&descriptor);

    // Size of 1 is valid
    descriptor.size = 1u;
    device.CreateResourceTable(&descriptor);

    // Size of maxResourceTableSize is valid
    descriptor.size = kMaxResourceTableSize;
    device.CreateResourceTable(&descriptor);

    // Size > limits is invalid
    descriptor.size = kMaxResourceTableSize + 1u;
    ASSERT_DEVICE_ERROR(device.CreateResourceTable(&descriptor));
}

// Test the Destroy call on a ResourceTable
TEST_F(ResourceTableValidationTest, Destroy) {
    wgpu::ResourceTableDescriptor descriptor;
    descriptor.size = 1u;
    wgpu::ResourceTable resourceTable = device.CreateResourceTable(&descriptor);

    // Calling destroy is valid
    resourceTable.Destroy();

    // Calling it multiple times is valid
    resourceTable.Destroy();
}

}  // namespace
}  // namespace dawn
