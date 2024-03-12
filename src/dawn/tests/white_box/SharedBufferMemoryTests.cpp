// Copyright 2024 The Dawn & Tint Authors
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

#include "dawn/tests/white_box/SharedBufferMemoryTests.h"
#include <gtest/gtest.h>
#include <vector>
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {

void SharedBufferMemoryTests::SetUp() {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    DawnTestWithParams<SharedBufferMemoryTestParams>::SetUp();
}

std::vector<wgpu::FeatureName> SharedBufferMemoryTests::GetRequiredFeatures() {
    auto features = GetParam().mBackend->RequiredFeatures(GetAdapter().Get());
    if (!SupportsFeatures(features)) {
        return {};
    }

    return features;
}

namespace {

using ::testing::HasSubstr;

// Test that it is an error to import shared buffer memory without a chained struct.
TEST_P(SharedBufferMemoryTests, ImportSharedBufferMemoryNoChain) {
    wgpu::SharedBufferMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR_MSG(
        wgpu::SharedBufferMemory memory = device.ImportSharedBufferMemory(&desc),
        HasSubstr("chain"));
}

// Test that it is an error to import shared buffer memory when the device is destroyed.
TEST_P(SharedBufferMemoryTests, ImportSharedBufferMemoryDeviceDestroy) {
    device.Destroy();

    wgpu::SharedBufferMemoryDescriptor desc;
    wgpu::SharedBufferMemory memory = device.ImportSharedBufferMemory(&desc);
}

// Test that SharedBufferMemory::IsDeviceLost() returns the expected value before and
// after destroying the device.
TEST_P(SharedBufferMemoryTests, CheckIsDeviceLostBeforeAndAfterDestroyingDevice) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(device);

    EXPECT_FALSE(memory.IsDeviceLost());
    device.Destroy();
    EXPECT_TRUE(memory.IsDeviceLost());
}

// Test that SharedBufferMemory::IsDeviceLost() returns the expected value before and
// after losing the device.
TEST_P(SharedBufferMemoryTests, CheckIsDeviceLostBeforeAndAfterLosingDevice) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(device);

    EXPECT_FALSE(memory.IsDeviceLost());
    LoseDeviceForTesting(device);
    EXPECT_TRUE(memory.IsDeviceLost());
}

// Test calling GetProperties on SharedBufferMemory after an error.
TEST_P(SharedBufferMemoryTests, GetPropertiesErrorMemory) {
    wgpu::SharedBufferMemoryDescriptor desc;
    ASSERT_DEVICE_ERROR(wgpu::SharedBufferMemory memory = device.ImportSharedBufferMemory(&desc));

    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    EXPECT_EQ(properties.usage, wgpu::BufferUsage::None);
    EXPECT_EQ(properties.size, 0u);
}

// Tests that creating SharedBufferMemory validates buffer size.
TEST_P(SharedBufferMemoryTests, SizeValidation) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(device);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.usage = properties.usage;
    bufferDesc.size = properties.size + 1;
    ASSERT_DEVICE_ERROR_MSG(memory.CreateBuffer(&bufferDesc),
                            HasSubstr("doesn't match descriptor size"));
}

// Tests that creating SharedBufferMemory validates buffer usages.
TEST_P(SharedBufferMemoryTests, UsageValidation) {
    wgpu::SharedBufferMemory memory = GetParam().mBackend->CreateSharedBufferMemory(device);
    wgpu::SharedBufferMemoryProperties properties;
    memory.GetProperties(&properties);

    wgpu::BufferDescriptor bufferDesc = {};
    bufferDesc.size = properties.size;

    for (wgpu::BufferUsage usage :
         {wgpu::BufferUsage::MapRead, wgpu::BufferUsage::MapWrite, wgpu::BufferUsage::CopySrc,
          wgpu::BufferUsage::CopyDst, wgpu::BufferUsage::Index, wgpu::BufferUsage::Vertex,
          wgpu::BufferUsage::Uniform, wgpu::BufferUsage::Storage, wgpu::BufferUsage::Indirect,
          wgpu::BufferUsage::QueryResolve}) {
        bufferDesc.usage = usage;
        if (usage & properties.usage) {
            wgpu::Buffer b = memory.CreateBuffer(&bufferDesc);
            EXPECT_EQ(b.GetUsage(), usage);
        } else {
            ASSERT_DEVICE_ERROR(memory.CreateBuffer(&bufferDesc));
        }
    }
}

GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST(SharedBufferMemoryTests);

}  // anonymous namespace
}  // namespace dawn
