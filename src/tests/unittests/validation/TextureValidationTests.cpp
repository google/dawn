// Copyright 2018 The Dawn Authors
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

#include "common/Constants.h"
#include "utils/DawnHelpers.h"

namespace {

class TextureValidationTest : public ValidationTest {
};

constexpr uint32_t kWidth = 32;
constexpr uint32_t kHeight = 32;
constexpr uint32_t kDefaultArraySize = 1;
constexpr uint32_t kDefaultMipLevels = 1;
constexpr uint32_t kDefaultSampleCount = 1;

constexpr dawn::TextureFormat kDefaultTextureFormat = dawn::TextureFormat::R8G8B8A8Unorm;

dawn::TextureDescriptor CreateDefaultTextureDescriptor() {
    dawn::TextureDescriptor descriptor;
    descriptor.nextInChain = nullptr;
    descriptor.size.width = kWidth;
    descriptor.size.height = kHeight;
    descriptor.size.depth = 1;
    descriptor.arraySize = kDefaultArraySize;
    descriptor.levelCount = kDefaultMipLevels;
    descriptor.sampleCount = kDefaultSampleCount;
    descriptor.dimension = dawn::TextureDimension::e2D;
    descriptor.format = kDefaultTextureFormat;
    descriptor.usage = dawn::TextureUsageBit::OutputAttachment | dawn::TextureUsageBit::Sampled;
    return descriptor;
}

// Test the validation of sample count
TEST_F(TextureValidationTest, SampleCount) {
    dawn::TextureDescriptor defaultDescriptor = CreateDefaultTextureDescriptor();

    // sampleCount == 1 is allowed.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 1;

        device.CreateTexture(&descriptor);
    }

    // It is an error to create a texture with an invalid sampleCount.
    {
        dawn::TextureDescriptor descriptor = defaultDescriptor;
        descriptor.sampleCount = 3;

        ASSERT_DEVICE_ERROR(device.CreateTexture(&descriptor));
    }
}
}