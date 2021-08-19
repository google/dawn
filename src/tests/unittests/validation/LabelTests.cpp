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
// WITHOUT WvecANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <string>
#include "tests/unittests/validation/ValidationTest.h"

class LabelTest : public ValidationTest {};

TEST_F(LabelTest, Buffer) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::Uniform;

    // The label should be empty if one was not set.
    {
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        buffer.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Buffer buffer = device.CreateBuffer(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(buffer.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}

TEST_F(LabelTest, Texture) {
    DAWN_SKIP_TEST_IF(UsesWire());
    std::string label = "test";
    wgpu::TextureDescriptor descriptor;
    descriptor.size.width = 1;
    descriptor.size.height = 1;
    descriptor.size.depthOrArrayLayers = 1;
    descriptor.mipLevelCount = 1;
    descriptor.sampleCount = 1;
    descriptor.dimension = wgpu::TextureDimension::e2D;
    descriptor.format = wgpu::TextureFormat::RGBA8Uint;
    descriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::TextureBinding;

    // The label should be empty if one was not set.
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(texture.Get());
        ASSERT_TRUE(readbackLabel.empty());
    }

    // Test setting a label through API
    {
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        texture.SetLabel(label.c_str());
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(texture.Get());
        ASSERT_EQ(label, readbackLabel);
    }

    // Test setting a label through the descriptor.
    {
        descriptor.label = label.c_str();
        wgpu::Texture texture = device.CreateTexture(&descriptor);
        std::string readbackLabel = dawn_native::GetObjectLabelForTesting(texture.Get());
        ASSERT_EQ(label, readbackLabel);
    }
}