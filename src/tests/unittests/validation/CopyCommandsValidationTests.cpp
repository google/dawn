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

#include "common/Constants.h"
#include "common/Math.h"
#include "tests/unittests/validation/ValidationTest.h"

class CopyCommandTest : public ValidationTest {
    protected:
        nxt::Buffer CreateFrozenBuffer(uint32_t size, nxt::BufferUsageBit usage) {
            nxt::Buffer buf = AssertWillBeSuccess(device.CreateBufferBuilder())
                .SetSize(size)
                .SetAllowedUsage(usage)
                .GetResult();
            buf.FreezeUsage(usage);
            return buf;
        }

        nxt::Texture CreateFrozen2DTexture(uint32_t width, uint32_t height, uint32_t levels,
                                         nxt::TextureFormat format, nxt::TextureUsageBit usage) {
            nxt::Texture tex = AssertWillBeSuccess(device.CreateTextureBuilder())
                .SetDimension(nxt::TextureDimension::e2D)
                .SetExtent(width, height, 1)
                .SetFormat(format)
                .SetMipLevels(levels)
                .SetAllowedUsage(usage)
                .GetResult();
            tex.FreezeUsage(usage);
            return tex;
        }

        uint32_t BufferSizeForTextureCopy(uint32_t width, uint32_t height, uint32_t depth) {
            uint32_t rowPitch = Align(width * 4, kTextureRowPitchAlignment);
            return (rowPitch * (height - 1) + width) * depth;
        }
};

class CopyCommandTest_B2B : public CopyCommandTest {
};

// TODO(cwallez@chromium.org): Test that copies are forbidden inside renderpasses

// Test a successfull B2B copy
TEST_F(CopyCommandTest_B2B, Success) {
    nxt::Buffer source = CreateFrozenBuffer(16, nxt::BufferUsageBit::TransferSrc);
    nxt::Buffer destination = CreateFrozenBuffer(16, nxt::BufferUsageBit::TransferDst);

    // Copy different copies, including some that touch the OOB condition
    {
        nxt::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, destination, 0, 16)
            .CopyBufferToBuffer(source, 8, destination, 0, 8)
            .CopyBufferToBuffer(source, 0, destination, 8, 8)
            .GetResult();
    }

    // Empty copies are valid
    {
        nxt::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, destination, 0, 0)
            .CopyBufferToBuffer(source, 0, destination, 16, 0)
            .CopyBufferToBuffer(source, 16, destination, 0, 0)
            .GetResult();
    }
}

// Test B2B copies with OOB
TEST_F(CopyCommandTest_B2B, OutOfBounds) {
    nxt::Buffer source = CreateFrozenBuffer(16, nxt::BufferUsageBit::TransferSrc);
    nxt::Buffer destination = CreateFrozenBuffer(16, nxt::BufferUsageBit::TransferDst);

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

// Test B2B copies with incorrect buffer usage
TEST_F(CopyCommandTest_B2B, BadUsage) {
    nxt::Buffer source = CreateFrozenBuffer(16, nxt::BufferUsageBit::TransferSrc);
    nxt::Buffer destination = CreateFrozenBuffer(16, nxt::BufferUsageBit::TransferDst);
    nxt::Buffer vertex = CreateFrozenBuffer(16, nxt::BufferUsageBit::Vertex);

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

class CopyCommandTest_B2T : public CopyCommandTest {
};

// Test a successfull B2T copy
TEST_F(CopyCommandTest_B2T, Success) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Buffer source = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferSrc);
    nxt::Texture destination = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                     nxt::TextureUsageBit::TransferDst);

    // Different copies, including some that touch the OOB condition
    {
        nxt::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            // Copy 4x4 block in corner of first mip.
            .CopyBufferToTexture(source, 0, 256, destination, 0, 0, 0, 4, 4, 1, 0)
            // Copy 4x4 block in opposite corner of first mip.
            .CopyBufferToTexture(source, 0, 256, destination, 12, 12, 0, 4, 4, 1, 0)
            // Copy 4x4 block in the 4x4 mip.
            .CopyBufferToTexture(source, 0, 256, destination, 0, 0, 0, 4, 4, 1, 2)
            // Copy with a buffer offset
            .CopyBufferToTexture(source, bufferSize - 4, 256, destination, 0, 0, 0, 1, 1, 1, 4)
            .GetResult();
    }

    // Empty copies are valid
    {
        nxt::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            // An empty copy
            .CopyBufferToTexture(source, 0, 0, destination, 0, 0, 0, 0, 0, 1, 0)
            // An empty copy touching the end of the buffer
            .CopyBufferToTexture(source, bufferSize, 0, destination, 0, 0, 0, 0, 0, 1, 0)
            // An empty copy touching the side of the texture
            .CopyBufferToTexture(source, 0, 0, destination, 16, 16, 0, 0, 0, 1, 0)
            .GetResult();
    }
}

// Test OOB conditions on the buffer
TEST_F(CopyCommandTest_B2T, OutOfBoundsOnBuffer) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Buffer source = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferSrc);
    nxt::Texture destination = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                     nxt::TextureUsageBit::TransferDst);

    // OOB on the buffer because we copy too many pixels
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 256, destination, 0, 0, 0, 4, 5, 1, 0)
            .GetResult();
    }

    // OOB on the buffer because of the offset
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 1, 256, destination, 0, 0, 0, 4, 4, 1, 0)
            .GetResult();
    }
}

// Test OOB conditions on the texture
TEST_F(CopyCommandTest_B2T, OutOfBoundsOnTexture) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Buffer source = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferSrc);
    nxt::Texture destination = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                     nxt::TextureUsageBit::TransferDst);

    // OOB on the texture because x + width overflows
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 256, destination, 13, 12, 0, 4, 4, 1, 0)
            .GetResult();
    }

    // OOB on the texture because y + width overflows
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 256, destination, 12, 13, 0, 4, 4, 1, 0)
            .GetResult();
    }

    // OOB on the texture because we overflow a non-zero mip
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 256, destination, 1, 0, 0, 4, 4, 1, 2)
            .GetResult();
    }

    // OOB on the texture even on an empty copy when we copy to a non-existent mip.
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 0, destination, 0, 0, 0, 0, 0, 1, 5)
            .GetResult();
    }
}

// Test that we force Z=0 and Depth=1 on copies to 2D textures
TEST_F(CopyCommandTest_B2T, ZDepthConstraintFor2DTextures) {
    nxt::Buffer source = CreateFrozenBuffer(16 * 4, nxt::BufferUsageBit::TransferSrc);
    nxt::Texture destination = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                     nxt::TextureUsageBit::TransferDst);

    // Z=1 on an empty copy still errors
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 0, destination, 0, 0, 1, 0, 0, 1, 0)
            .GetResult();
    }

    // Depth=0 on an empty copy still errors
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 0, destination, 0, 0, 0, 0, 0, 0, 0)
            .GetResult();
    }
}

// Test B2T copies with incorrect buffer usage
TEST_F(CopyCommandTest_B2T, IncorrectUsage) {
    nxt::Buffer source = CreateFrozenBuffer(16 * 4, nxt::BufferUsageBit::TransferSrc);
    nxt::Buffer vertex = CreateFrozenBuffer(16 * 4, nxt::BufferUsageBit::Vertex);
    nxt::Texture destination = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                     nxt::TextureUsageBit::TransferDst);
    nxt::Texture sampled = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                 nxt::TextureUsageBit::Sampled);

    // Incorrect source usage
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(vertex, 0, 256, destination, 0, 0, 0, 4, 4, 1, 0)
            .GetResult();
    }

    // Incorrect destination usage
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToTexture(source, 0, 256, sampled, 0, 0, 0, 4, 4, 1, 0)
            .GetResult();
    }
}

class CopyCommandTest_T2B : public CopyCommandTest {
};

// Test a successfull T2B copy
TEST_F(CopyCommandTest_T2B, Success) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Texture source = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                nxt::TextureUsageBit::TransferSrc);
    nxt::Buffer destination = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferDst);

    // Different copies, including some that touch the OOB condition
    {
        nxt::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            // Copy from 4x4 block in corner of first mip.
            .CopyTextureToBuffer(source, 0, 0, 0, 4, 4, 1, 0, destination, 0, 256)
            // Copy from 4x4 block in opposite corner of first mip.
            .CopyTextureToBuffer(source, 12, 12, 0, 4, 4, 1, 0, destination, 0, 256)
            // Copy from 4x4 block in the 4x4 mip.
            .CopyTextureToBuffer(source, 0, 0, 0, 4, 4, 1, 2, destination, 0, 256)
            // Copy with a buffer offset
            .CopyTextureToBuffer(source, 0, 0, 0, 1, 1, 1, 4, destination, bufferSize - 4, 256)
            .GetResult();
    }

    // Empty copies are valid
    {
        nxt::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            // An empty copy
            .CopyTextureToBuffer(source, 0, 0, 0, 0, 0, 1, 0, destination, 0, 0)
            // An empty copy touching the end of the buffer
            .CopyTextureToBuffer(source, 0, 0, 0, 0, 0, 1, 0, destination, bufferSize, 0)
            // An empty copy touching the side of the texture
            .CopyTextureToBuffer(source, 16, 16, 0, 0, 0, 1, 0, destination, 0, 0)
            .GetResult();
    }
}

// Test OOB conditions on the texture
TEST_F(CopyCommandTest_T2B, OutOfBoundsOnTexture) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Texture source = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                nxt::TextureUsageBit::TransferSrc);
    nxt::Buffer destination = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferDst);

    // OOB on the texture because x + width overflows
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 13, 12, 0, 4, 4, 1, 0, destination, 0, 256)
            .GetResult();
    }

    // OOB on the texture because y + width overflows
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 12, 13, 0, 4, 4, 1, 0, destination, 0, 256)
            .GetResult();
    }

    // OOB on the texture because we overflow a non-zero mip
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 1, 0, 0, 4, 4, 1, 2, destination, 0, 256)
            .GetResult();
    }

    // OOB on the texture even on an empty copy when we copy from a non-existent mip.
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 0, 0, 0, 0, 0, 1, 5, destination, 0, 0)
            .GetResult();
    }
}

// Test OOB conditions on the buffer
TEST_F(CopyCommandTest_T2B, OutOfBoundsOnBuffer) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Texture source = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                nxt::TextureUsageBit::TransferSrc);
    nxt::Buffer destination = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferDst);

    // OOB on the buffer because we copy too many pixels
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 0, 0, 0, 4, 5, 1, 0, destination, 0, 256)
            .GetResult();
    }

    // OOB on the buffer because of the offset
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 0, 0, 0, 4, 4, 1, 0, destination, 1, 256)
            .GetResult();
    }
}

// Test that we force Z=0 and Depth=1 on copies from to 2D textures
TEST_F(CopyCommandTest_T2B, ZDepthConstraintFor2DTextures) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Texture source = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                nxt::TextureUsageBit::TransferSrc);
    nxt::Buffer destination = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferDst);

    // Z=1 on an empty copy still errors
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 0, 0, 1, 0, 0, 1, 0, destination, 0, 0)
            .GetResult();
    }

    // Depth=0 on an empty copy still errors
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 0, 0, 0, 0, 0, 0, 0, destination, 0, 0)
            .GetResult();
    }
}

// Test T2B copies with incorrect buffer usage
TEST_F(CopyCommandTest_T2B, IncorrectUsage) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    nxt::Texture source = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                nxt::TextureUsageBit::TransferSrc);
    nxt::Texture sampled = CreateFrozen2DTexture(16, 16, 5, nxt::TextureFormat::R8G8B8A8Unorm,
                                                 nxt::TextureUsageBit::Sampled);
    nxt::Buffer destination = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::TransferDst);
    nxt::Buffer vertex = CreateFrozenBuffer(bufferSize, nxt::BufferUsageBit::Vertex);

    // Incorrect source usage
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(sampled, 0, 0, 0, 4, 4, 1, 0, destination, 0, 256)
            .GetResult();
    }

    // Incorrect destination usage
    {
        nxt::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyTextureToBuffer(source, 0, 0, 0, 4, 4, 1, 0, vertex, 0, 256)
            .GetResult();
    }
}
