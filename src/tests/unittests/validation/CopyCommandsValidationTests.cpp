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

#include "ValidationTest.h"

class CopyCommandTest_B2B : public ValidationTest {
};

// Test a successfull B2B copy
TEST_F(CopyCommandTest_B2B, Success) {
    nxt::Buffer source = device.CreateBufferBuilder()
        .SetSize(16)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferSrc)
        .GetResult();
    source.FreezeUsage(nxt::BufferUsageBit::TransferSrc);

    nxt::Buffer destination = device.CreateBufferBuilder()
        .SetSize(16)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();
    destination.FreezeUsage(nxt::BufferUsageBit::TransferDst);

    // Copy different copies, including some that touch the OOB condition
    nxt::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
        .CopyBufferToBuffer(source, 0, destination, 0, 16)
        .CopyBufferToBuffer(source, 8, destination, 0, 8)
        .CopyBufferToBuffer(source, 0, destination, 8, 8)
        .GetResult();
}

// Test B2B copies with overflows
TEST_F(CopyCommandTest_B2B, OutOfBounds) {
    nxt::Buffer source = device.CreateBufferBuilder()
        .SetSize(16)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferSrc)
        .GetResult();
    source.FreezeUsage(nxt::BufferUsageBit::TransferSrc);

    nxt::Buffer destination = device.CreateBufferBuilder()
        .SetSize(16)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();
    destination.FreezeUsage(nxt::BufferUsageBit::TransferDst);

    // OOB on the source
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 8, destination, 0, 12)
            .GetResult();
    }

    // OOB on the destination
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, destination, 8, 12)
            .GetResult();
    }
}

// Test B2B copies with overflows
TEST_F(CopyCommandTest_B2B, BadUsage) {
    nxt::Buffer source = device.CreateBufferBuilder()
        .SetSize(16)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferSrc)
        .GetResult();
    source.FreezeUsage(nxt::BufferUsageBit::TransferSrc);

    nxt::Buffer destination = device.CreateBufferBuilder()
        .SetSize(16)
        .SetAllowedUsage(nxt::BufferUsageBit::TransferDst)
        .GetResult();
    destination.FreezeUsage(nxt::BufferUsageBit::TransferDst);

    nxt::Buffer vertex = device.CreateBufferBuilder()
        .SetSize(16)
        .SetAllowedUsage(nxt::BufferUsageBit::Vertex)
        .GetResult();
    vertex.FreezeUsage(nxt::BufferUsageBit::Vertex);

    // Source with incorrect usage
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(vertex, 0, destination, 0, 16)
            .GetResult();
    }

    // Destination with incorrect usage
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, vertex, 0, 16)
            .GetResult();
    }
}

// TODO(cwallez@chromium.org): Test that B2B copies are forbidden inside renderpasses
