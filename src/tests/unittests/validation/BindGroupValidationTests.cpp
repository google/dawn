// Copyright 2017 The NXT Authors
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

class BindGroupValidationTest : public ValidationTest {
};

TEST_F(BindGroupValidationTest, BufferViewOffset) {
    auto layout = device.CreateBindGroupLayoutBuilder()
        .SetBindingsType(nxt::ShaderStageBit::Vertex, nxt::BindingType::UniformBuffer, 0, 1)
        .GetResult();

    auto buffer = device.CreateBufferBuilder()
        .SetAllowedUsage(nxt::BufferUsageBit::Uniform)
        .SetInitialUsage(nxt::BufferUsageBit::Uniform)
        .SetSize(512)
        .GetResult();
    
    // Check that offset 0 is valid
    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(0, 512)
            .GetResult();

        auto bindGroup = AssertWillBeSuccess(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetUsage(nxt::BindGroupUsage::Frozen)
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
            .SetUsage(nxt::BindGroupUsage::Frozen)
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
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(64, 256)
            .GetResult();

        auto bindGroup = AssertWillBeError(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(128, 256)
            .GetResult();

        auto bindGroup = AssertWillBeError(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }

    {
        auto bufferView = buffer.CreateBufferViewBuilder()
            .SetExtent(255, 256)
            .GetResult();

        auto bindGroup = AssertWillBeError(device.CreateBindGroupBuilder())
            .SetLayout(layout)
            .SetUsage(nxt::BindGroupUsage::Frozen)
            .SetBufferViews(0, 1, &bufferView)
            .GetResult();
    }
}
