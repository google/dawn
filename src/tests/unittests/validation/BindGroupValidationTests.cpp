// Copyright 2017 The Dawn Authors
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

class BindGroupValidationTest : public ValidationTest {
};

// Tests constraints on the buffer view offset for bind groups.
TEST_F(BindGroupValidationTest, BufferViewOffset) {
    auto layout = utils::MakeBindGroupLayout(
        device, {
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });

    dawn::Buffer buffer;
    {
        dawn::BufferDescriptor descriptor;
        descriptor.size = 512;
        descriptor.usage = dawn::BufferUsageBit::Uniform;
        buffer = device.CreateBuffer(&descriptor);
    }

    // Check that offset 0 is valid
    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(0, 512)
            .GetResult();

        auto bindGroup = AssertWillBeSuccess(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    // Check that offset 256 (aligned) is valid
    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(256, 256)
            .GetResult();

        auto bindGroup = AssertWillBeSuccess(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    // Check cases where unaligned buffer view offset is invalid
    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(1, 256)
            .GetResult();

        auto bindGroup = AssertWillBeError(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(64, 256)
            .GetResult();

        auto bindGroup = AssertWillBeError(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(128, 256)
            .GetResult();

        auto bindGroup = AssertWillBeError(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(255, 256)
            .GetResult();

        auto bindGroup = AssertWillBeError(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }
}

// This test verifies that the BindGroupLayout cache is successfully caching/deduplicating objects.
//
// NOTE: This test only works currently because unittests are run without the wire - so the returned
// BindGroupLayout pointers are actually visibly equivalent. With the wire, this would not be true.
TEST_F(BindGroupValidationTest, BindGroupLayoutCache) {
    auto layout1 = utils::MakeBindGroupLayout(
        device, {
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });
    auto layout2 = utils::MakeBindGroupLayout(
        device, {
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });

    // Caching should cause these to be the same.
    ASSERT_EQ(layout1.Get(), layout2.Get());
}

// This test verifies that the BindGroupLayout bindings are correctly validated, even if the
// binding ids are out-of-order.
TEST_F(BindGroupValidationTest, BindGroupBinding) {
    auto layout = utils::MakeBindGroupLayout(
        device, {
                    {1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                    {0, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer},
                });
}

// Tests setting OOB checks for kMaxBindingsPerGroup in bind group layouts.
TEST_F(BindGroupValidationTest, BindGroupLayoutBindingOOB) {
    // Checks that kMaxBindingsPerGroup - 1 is valid.
    utils::MakeBindGroupLayout(device, {
        {kMaxBindingsPerGroup - 1, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}
    });

    // Checks that kMaxBindingsPerGroup is OOB
    ASSERT_DEVICE_ERROR(utils::MakeBindGroupLayout(device, {
        {kMaxBindingsPerGroup, dawn::ShaderStageBit::Vertex, dawn::BindingType::UniformBuffer}
    }));
}
