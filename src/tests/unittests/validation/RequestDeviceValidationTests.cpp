// Copyright 2021 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

class RequestDeviceValidationTest : public ValidationTest {
  protected:
    void SetUp() {
        DAWN_SKIP_TEST_IF(UsesWire());
        ValidationTest::SetUp();
    }

    static void ExpectRequestDeviceSuccess(WGPURequestDeviceStatus status,
                                           WGPUDevice cDevice,
                                           const char* message,
                                           void* userdata) {
        wgpu::Device device = wgpu::Device::Acquire(cDevice);
        EXPECT_EQ(status, WGPURequestDeviceStatus_Success);
        EXPECT_NE(device, nullptr);
        EXPECT_STREQ(message, nullptr);
    }

    static void ExpectRequestDeviceError(WGPURequestDeviceStatus status,
                                         WGPUDevice cDevice,
                                         const char* message,
                                         void* userdata) {
        wgpu::Device device = wgpu::Device::Acquire(cDevice);
        EXPECT_EQ(status, WGPURequestDeviceStatus_Error);
        EXPECT_EQ(device, nullptr);
        EXPECT_STRNE(message, nullptr);
    }
};

// Test that requesting a device without specifying limits is valid.
TEST_F(RequestDeviceValidationTest, NoRequiredLimits) {
    dawn_native::DeviceDescriptor descriptor;
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceSuccess, nullptr);
}

// Test that requesting a device with the default limits is valid.
TEST_F(RequestDeviceValidationTest, DefaultLimits) {
    wgpu::Limits limits = {};
    dawn_native::DeviceDescriptor descriptor;
    descriptor.requiredLimits = reinterpret_cast<const WGPULimits*>(&limits);
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceSuccess, nullptr);
}

// Test that requesting a device where a required limit is above the maximum value.
TEST_F(RequestDeviceValidationTest, HigherIsBetter) {
    wgpu::Limits limits = {};
    dawn_native::DeviceDescriptor descriptor;
    descriptor.requiredLimits = reinterpret_cast<const WGPULimits*>(&limits);

    wgpu::Limits supportedLimits;
    EXPECT_TRUE(adapter.GetLimits(reinterpret_cast<WGPULimits*>(&supportedLimits)));

    // Test below the max.
    limits.maxBindGroups = supportedLimits.maxBindGroups - 1;
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceSuccess, nullptr);

    // Test the max.
    limits.maxBindGroups = supportedLimits.maxBindGroups;
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceSuccess, nullptr);

    // Test above the max.
    limits.maxBindGroups = supportedLimits.maxBindGroups + 1;
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceError, nullptr);
}

// Test that requesting a device where a required limit is below the minimum value.
TEST_F(RequestDeviceValidationTest, LowerIsBetter) {
    wgpu::Limits limits = {};
    dawn_native::DeviceDescriptor descriptor;
    descriptor.requiredLimits = reinterpret_cast<const WGPULimits*>(&limits);

    wgpu::Limits supportedLimits;
    EXPECT_TRUE(adapter.GetLimits(reinterpret_cast<WGPULimits*>(&supportedLimits)));

    // Test below the min.
    limits.minUniformBufferOffsetAlignment = supportedLimits.minUniformBufferOffsetAlignment / 2;
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceError, nullptr);

    // Test the min.
    limits.minUniformBufferOffsetAlignment = supportedLimits.minUniformBufferOffsetAlignment;
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceSuccess, nullptr);

    // Test above the min.
    limits.minUniformBufferOffsetAlignment = supportedLimits.minUniformBufferOffsetAlignment * 2;
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceSuccess, nullptr);
}

// Test that it is an error to request limits with an invalid chained struct
TEST_F(RequestDeviceValidationTest, InvalidChainedStruct) {
    wgpu::PrimitiveDepthClampingState depthClamp = {};
    wgpu::Limits limits = {};
    limits.nextInChain = &depthClamp;

    dawn_native::DeviceDescriptor descriptor;
    descriptor.requiredLimits = reinterpret_cast<const WGPULimits*>(&limits);
    adapter.RequestDevice(&descriptor, ExpectRequestDeviceError, nullptr);
}
