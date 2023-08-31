// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_WHITE_BOX_SHAREDTEXTUREMEMORYTESTS_H_
#define SRC_DAWN_TESTS_WHITE_BOX_SHAREDTEXTUREMEMORYTESTS_H_

#include <gtest/gtest.h>
#include <string>
#include <utility>
#include <vector>

#include "dawn/tests/DawnTest.h"

namespace dawn {

class SharedTextureMemoryTestBackend {
  public:
    // The name used in gtest parameterization. Names of backends must be unique.
    virtual std::string Name() const = 0;

    // The required features for testing this backend.
    virtual std::vector<wgpu::FeatureName> RequiredFeatures() const = 0;

    // Create one basic shared texture memory. It should support most operations.
    virtual wgpu::SharedTextureMemory CreateSharedTextureMemory(wgpu::Device& device) = 0;

    // Create a variety of valid SharedTextureMemory for testing, one on each device.
    // Backends should return all interesting types of shared texture memory here, including
    // different sizes, formats, memory types, etc.
    // The inner vector is a vector of the same memory imported to each device.
    virtual std::vector<std::vector<wgpu::SharedTextureMemory>>
    CreatePerDeviceSharedTextureMemories(const std::vector<wgpu::Device>& devices) = 0;

    // Import `fence` which may have been created on some other device, onto `importingDevice`.
    virtual wgpu::SharedFence ImportFenceTo(const wgpu::Device& importingDevice,
                                            const wgpu::SharedFence& fence) = 0;

    // Shorthand version of `CreatePerDeviceSharedTextureMemories` that creates memories on a single
    // device.
    std::vector<wgpu::SharedTextureMemory> CreateSharedTextureMemories(wgpu::Device& device);

    // Wrapper around CreatePerDeviceSharedTextureMemories that filters the memories by
    // usage to ensure they have `requiredUsage`.
    std::vector<std::vector<wgpu::SharedTextureMemory>>
    CreatePerDeviceSharedTextureMemoriesFilterByUsage(const std::vector<wgpu::Device>& devices,
                                                      wgpu::TextureUsage requiredUsage);

    // Return true if the test should always use the same device.
    // Some interop paths require the same underyling backend device.
    virtual bool UseSameDevice() const { return false; }

    // Whether or not the backing supports concurrent reads. This is
    // a property of the underlying API (keyed mutex, vk binary semaphore),
    // so it is concurrent reads across disjoint Dawn devices - not concurrent
    // reads on the same Dawn device.
    virtual bool SupportsConcurrentRead() const { return true; }
};

inline std::ostream& operator<<(std::ostream& o, SharedTextureMemoryTestBackend* backend) {
    o << backend->Name();
    return o;
}

using Backend = SharedTextureMemoryTestBackend*;
DAWN_TEST_PARAM_STRUCT(SharedTextureMemoryTestParams, Backend);

class SharedTextureMemoryNoFeatureTests : public DawnTestWithParams<SharedTextureMemoryTestParams> {
  protected:
    void SetUp() override;
};

class SharedTextureMemoryTests : public DawnTestWithParams<SharedTextureMemoryTestParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override;

    void SetUp() override;

    wgpu::Device CreateDevice();

    void UseInRenderPass(wgpu::Device& deviceObj, wgpu::Texture& texture);
    void UseInCopy(wgpu::Device& deviceObj, wgpu::Texture& texture);

    wgpu::CommandBuffer MakeFourColorsClearCommandBuffer(wgpu::Device& deviceObj,
                                                         wgpu::Texture& texture);
    std::pair<wgpu::CommandBuffer, wgpu::Texture> MakeCheckBySamplingCommandBuffer(
        wgpu::Device& deviceObj,
        wgpu::Texture& texture);
    void CheckFourColors(wgpu::Device& deviceObj,
                         wgpu::TextureFormat format,
                         wgpu::Texture& colorTarget);
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_WHITE_BOX_SHAREDTEXTUREMEMORYTESTS_H_
