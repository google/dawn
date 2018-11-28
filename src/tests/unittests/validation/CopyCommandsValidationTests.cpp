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

#include "common/Constants.h"
#include "common/Math.h"
#include "tests/unittests/validation/ValidationTest.h"
#include "utils/DawnHelpers.h"

class CopyCommandTest : public ValidationTest {
    protected:
        dawn::Buffer CreateBuffer(uint32_t size, dawn::BufferUsageBit usage) {
            dawn::BufferDescriptor descriptor;
            descriptor.size = size;
            descriptor.usage = usage;

            return device.CreateBuffer(&descriptor);
        }

        dawn::Texture Create2DTexture(uint32_t width, uint32_t height, uint32_t levels, uint32_t arrayLayer,
                                         dawn::TextureFormat format, dawn::TextureUsageBit usage) {
            dawn::TextureDescriptor descriptor;
            descriptor.dimension = dawn::TextureDimension::e2D;
            descriptor.size.width = width;
            descriptor.size.height = height;
            descriptor.size.depth = 1;
            descriptor.arrayLayer = arrayLayer;
            descriptor.format = format;
            descriptor.levelCount = levels;
            descriptor.usage = usage;
            dawn::Texture tex = device.CreateTexture(&descriptor);
            return tex;
        }

        uint32_t BufferSizeForTextureCopy(uint32_t width, uint32_t height, uint32_t depth) {
            uint32_t rowPitch = Align(width * 4, kTextureRowPitchAlignment);
            return (rowPitch * (height - 1) + width) * depth;
        }

        void TestB2TCopy(utils::Expectation expectation,
                         dawn::Buffer srcBuffer,
                         uint32_t srcOffset,
                         uint32_t srcRowPitch,
                         uint32_t srcImageHeight,
                         dawn::Texture destTexture,
                         uint32_t destLevel,
                         uint32_t destSlice,
                         dawn::Origin3D destOrigin,
                         dawn::TextureAspect destAspect,
                         dawn::Extent3D extent3D) {
            dawn::BufferCopyView bufferCopyView =
                utils::CreateBufferCopyView(srcBuffer, srcOffset, srcRowPitch, srcImageHeight);
            dawn::TextureCopyView textureCopyView = utils::CreateTextureCopyView(
                destTexture, destLevel, destSlice, destOrigin, destAspect);

            if (expectation == utils::Expectation::Success) {
                dawn::CommandBuffer commands =
                    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
                        .CopyBufferToTexture(&bufferCopyView, &textureCopyView, &extent3D)
                        .GetResult();
            } else {
                dawn::CommandBuffer commands =
                    AssertWillBeError(device.CreateCommandBufferBuilder())
                        .CopyBufferToTexture(&bufferCopyView, &textureCopyView, &extent3D)
                        .GetResult();
            }
        }

        void TestT2BCopy(utils::Expectation expectation,
                         dawn::Texture srcTexture,
                         uint32_t srcLevel,
                         uint32_t srcSlice,
                         dawn::Origin3D srcOrigin,
                         dawn::TextureAspect srcAspect,
                         dawn::Buffer destBuffer,
                         uint32_t destOffset,
                         uint32_t destRowPitch,
                         uint32_t destImageHeight,
                         dawn::Extent3D extent3D) {
            dawn::BufferCopyView bufferCopyView =
                utils::CreateBufferCopyView(destBuffer, destOffset, destRowPitch, destImageHeight);
            dawn::TextureCopyView textureCopyView = utils::CreateTextureCopyView(
                srcTexture, srcLevel, srcSlice, srcOrigin, srcAspect);

            if (expectation == utils::Expectation::Success) {
                dawn::CommandBuffer commands =
                    AssertWillBeSuccess(device.CreateCommandBufferBuilder())
                        .CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &extent3D)
                        .GetResult();
            } else {
                dawn::CommandBuffer commands =
                    AssertWillBeError(device.CreateCommandBufferBuilder())
                        .CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &extent3D)
                        .GetResult();
            }
        }
};

class CopyCommandTest_B2B : public CopyCommandTest {
};

// TODO(cwallez@chromium.org): Test that copies are forbidden inside renderpasses

// Test a successfull B2B copy
TEST_F(CopyCommandTest_B2B, Success) {
    dawn::Buffer source = CreateBuffer(16, dawn::BufferUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(16, dawn::BufferUsageBit::TransferDst);

    // Copy different copies, including some that touch the OOB condition
    {
        dawn::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, destination, 0, 16)
            .CopyBufferToBuffer(source, 8, destination, 0, 8)
            .CopyBufferToBuffer(source, 0, destination, 8, 8)
            .GetResult();
    }

    // Empty copies are valid
    {
        dawn::CommandBuffer commands = AssertWillBeSuccess(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, destination, 0, 0)
            .CopyBufferToBuffer(source, 0, destination, 16, 0)
            .CopyBufferToBuffer(source, 16, destination, 0, 0)
            .GetResult();
    }
}

// Test B2B copies with OOB
TEST_F(CopyCommandTest_B2B, OutOfBounds) {
    dawn::Buffer source = CreateBuffer(16, dawn::BufferUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(16, dawn::BufferUsageBit::TransferDst);

    // OOB on the source
    {
        dawn::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 8, destination, 0, 12)
            .GetResult();
    }

    // OOB on the destination
    {
        dawn::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, destination, 8, 12)
            .GetResult();
    }
}

// Test B2B copies with incorrect buffer usage
TEST_F(CopyCommandTest_B2B, BadUsage) {
    dawn::Buffer source = CreateBuffer(16, dawn::BufferUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(16, dawn::BufferUsageBit::TransferDst);
    dawn::Buffer vertex = CreateBuffer(16, dawn::BufferUsageBit::Vertex);

    // Source with incorrect usage
    {
        dawn::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(vertex, 0, destination, 0, 16)
            .GetResult();
    }

    // Destination with incorrect usage
    {
        dawn::CommandBuffer commands = AssertWillBeError(device.CreateCommandBufferBuilder())
            .CopyBufferToBuffer(source, 0, vertex, 0, 16)
            .GetResult();
    }
}

class CopyCommandTest_B2T : public CopyCommandTest {
};

// Test a successfull B2T copy
TEST_F(CopyCommandTest_B2T, Success) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Buffer source = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferSrc);
    dawn::Texture destination = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                     dawn::TextureUsageBit::TransferDst);

    // Different copies, including some that touch the OOB condition
    {
        // Copy 4x4 block in corner of first mip.
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 0, destination, 0, 0, {0, 0, 0},
                    dawn::TextureAspect::Color, {4, 4, 1});
        // Copy 4x4 block in opposite corner of first mip.
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 0, destination, 0, 0, {12, 12, 0},
                    dawn::TextureAspect::Color, {4, 4, 1});
        // Copy 4x4 block in the 4x4 mip.
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 0, destination, 2, 0, {0, 0, 0},
                    dawn::TextureAspect::Color, {4, 4, 1});
        // Copy with a buffer offset
        TestB2TCopy(utils::Expectation::Success, source, bufferSize - 4, 256, 0, destination, 0, 0,
                    {0, 0, 0}, dawn::TextureAspect::Color, {1, 1, 1});
    }

    // Copies with a 256-byte aligned row pitch but unaligned texture region
    {
        // Unaligned region
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 0, destination, 0, 0, {0, 0, 0},
                    dawn::TextureAspect::Color, {3, 4, 1});
        // Unaligned region with texture offset
        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 0, destination, 0, 0, {5, 7, 0},
                    dawn::TextureAspect::Color, {2, 3, 1});
        // Unaligned region, with buffer offset
        TestB2TCopy(utils::Expectation::Success, source, 31 * 4, 256, 0, destination, 0, 0, {0, 0, 0},
                    dawn::TextureAspect::Color, {3, 3, 1});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestB2TCopy(utils::Expectation::Success, source, 0, 0, 0, destination, 0, 0, {0, 0, 0},
                    dawn::TextureAspect::Color, {0, 0, 1});
        // An empty copy touching the end of the buffer
        TestB2TCopy(utils::Expectation::Success, source, bufferSize, 0, 0, destination, 0, 0, {0, 0,
                    0}, dawn::TextureAspect::Color, {0, 0, 1});
        // An empty copy touching the side of the texture
        TestB2TCopy(utils::Expectation::Success, source, 0, 0, 0, destination, 0, 0, {16, 16, 0},
                    dawn::TextureAspect::Color, {0, 0, 1});
    }
}

// Test OOB conditions on the buffer
TEST_F(CopyCommandTest_B2T, OutOfBoundsOnBuffer) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Buffer source = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferSrc);
    dawn::Texture destination = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                     dawn::TextureUsageBit::TransferDst);

    // OOB on the buffer because we copy too many pixels
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {4, 5, 1});

    // OOB on the buffer because of the offset
    TestB2TCopy(utils::Expectation::Failure, source, 4, 256, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {4, 4, 1});

    // OOB on the buffer because (row pitch * (height - 1) + width) * depth overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 512, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {4, 3, 1});

    // Not OOB on the buffer although row pitch * height overflows
    // but (row pitch * (height - 1) + width) * depth does not overlow
    {
        uint32_t sourceBufferSize = BufferSizeForTextureCopy(7, 3, 1);
        ASSERT_TRUE(256 * 3 > sourceBufferSize) << "row pitch * height should overflow buffer";
        dawn::Buffer sourceBuffer = CreateBuffer(sourceBufferSize, dawn::BufferUsageBit::TransferSrc);

        TestB2TCopy(utils::Expectation::Success, source, 0, 256, 0, destination, 0, 0, {0, 0, 0},
                    dawn::TextureAspect::Color, {7, 3, 1});
    }
}

// Test OOB conditions on the texture
TEST_F(CopyCommandTest_B2T, OutOfBoundsOnTexture) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Buffer source = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferSrc);
    dawn::Texture destination = Create2DTexture(16, 16, 5, 2, dawn::TextureFormat::R8G8B8A8Unorm,
                                                     dawn::TextureUsageBit::TransferDst);

    // OOB on the texture because x + width overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 0, 0, {13, 12, 0},
                dawn::TextureAspect::Color, {4, 4, 1});

    // OOB on the texture because y + width overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 0, 0, {12, 13, 0},
                dawn::TextureAspect::Color, {4, 4, 1});

    // OOB on the texture because we overflow a non-zero mip
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 2, 0, {1, 0, 0},
                dawn::TextureAspect::Color, {4, 4, 1});

    // OOB on the texture even on an empty copy when we copy to a non-existent mip.
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 5, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {0, 0, 1});

    // OOB on the texture because slice overflows
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 0, 2, {0, 0, 0},
                dawn::TextureAspect::Color, {0, 0, 1});
}

// Test that we force Z=0 and Depth=1 on copies to 2D textures
TEST_F(CopyCommandTest_B2T, ZDepthConstraintFor2DTextures) {
    dawn::Buffer source = CreateBuffer(16 * 4, dawn::BufferUsageBit::TransferSrc);
    dawn::Texture destination = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                     dawn::TextureUsageBit::TransferDst);

    // Z=1 on an empty copy still errors
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 0, 0, {0, 0, 1},
                dawn::TextureAspect::Color, {0, 0, 1});

    // Depth=0 on an empty copy still errors
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {0, 0, 0});
}

// Test B2T copies with incorrect buffer usage
TEST_F(CopyCommandTest_B2T, IncorrectUsage) {
    dawn::Buffer source = CreateBuffer(16 * 4, dawn::BufferUsageBit::TransferSrc);
    dawn::Buffer vertex = CreateBuffer(16 * 4, dawn::BufferUsageBit::Vertex);
    dawn::Texture destination = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                     dawn::TextureUsageBit::TransferDst);
    dawn::Texture sampled = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                 dawn::TextureUsageBit::Sampled);

    // Incorrect source usage
    TestB2TCopy(utils::Expectation::Failure, vertex, 0, 256, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {4, 4, 1});

    // Incorrect destination usage
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, sampled, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {4, 4, 1});
}

TEST_F(CopyCommandTest_B2T, IncorrectRowPitch) {
    uint32_t bufferSize = BufferSizeForTextureCopy(128, 16, 1);
    dawn::Buffer source = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferSrc);
    dawn::Texture destination = Create2DTexture(128, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
        dawn::TextureUsageBit::TransferDst);

    // Default row pitch is not 256-byte aligned
    TestB2TCopy(utils::Expectation::Failure, source, 0, 0, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {3, 4, 1});

    // Row pitch is not 256-byte aligned
    TestB2TCopy(utils::Expectation::Failure, source, 0, 128, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {4, 4, 1});

    // Row pitch is less than width * bytesPerPixel
    TestB2TCopy(utils::Expectation::Failure, source, 0, 256, 0, destination, 0, 0, {0, 0, 0},
                dawn::TextureAspect::Color, {65, 1, 1});
}

// Test B2T copies with incorrect buffer offset usage
TEST_F(CopyCommandTest_B2T, IncorrectBufferOffset) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Buffer source = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferSrc);
    dawn::Texture destination = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                     dawn::TextureUsageBit::TransferDst);

    // Correct usage
    TestB2TCopy(utils::Expectation::Success, source, bufferSize - 4, 256, 0, destination, 0, 0, {0,
                0, 0}, dawn::TextureAspect::Color, {1, 1, 1});

    // Incorrect usages
    {
        TestB2TCopy(utils::Expectation::Failure, source, bufferSize - 5, 256, 0, destination, 0, 0,
                    {0, 0, 0}, dawn::TextureAspect::Color, {1, 1, 1});
        TestB2TCopy(utils::Expectation::Failure, source, bufferSize - 6, 256, 0, destination, 0, 0,
                    {0, 0, 0}, dawn::TextureAspect::Color, {1, 1, 1});
        TestB2TCopy(utils::Expectation::Failure, source, bufferSize - 7, 256, 0, destination, 0, 0,
                    {0, 0, 0}, dawn::TextureAspect::Color, {1, 1, 1});
    }
}

class CopyCommandTest_T2B : public CopyCommandTest {
};

// Test a successfull T2B copy
TEST_F(CopyCommandTest_T2B, Success) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Texture source = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                dawn::TextureUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferDst);

    // Different copies, including some that touch the OOB condition
    {
        // Copy from 4x4 block in corner of first mip.
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destination, 0, 256, 0, {4, 4, 1});
        // Copy from 4x4 block in opposite corner of first mip.
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {12, 12, 0},
                    dawn::TextureAspect::Color, destination, 0, 256, 0, {4, 4, 1});
        // Copy from 4x4 block in the 4x4 mip.
        TestT2BCopy(utils::Expectation::Success, source, 2, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destination, 0, 256, 0, {4, 4, 1});
        // Copy with a buffer offset
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destination, bufferSize - 4, 256, 0, {1, 1, 1});
    }

    // Copies with a 256-byte aligned row pitch but unaligned texture region
    {
        // Unaligned region
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destination, 0, 256, 0, {3, 4, 1});
        // Unaligned region with texture offset
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {5, 7, 0}, dawn::TextureAspect::Color,
                    destination, 0, 256, 0, {2, 3, 1});
        // Unaligned region, with buffer offset
        TestT2BCopy(utils::Expectation::Success, source, 2, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destination, 31 * 4, 256, 0, {3, 3, 1});
    }

    // Empty copies are valid
    {
        // An empty copy
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destination, 0, 0, 0, {0, 0, 1});
        // An empty copy touching the end of the buffer
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destination, bufferSize, 0, 0, {0, 0, 1});
        // An empty copy touching the side of the texture
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {16, 16, 0},
                    dawn::TextureAspect::Color, destination, 0, 0, 0, {0, 0, 1});
    }
}

// Test OOB conditions on the texture
TEST_F(CopyCommandTest_T2B, OutOfBoundsOnTexture) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Texture source = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                dawn::TextureUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferDst);

    // OOB on the texture because x + width overflows
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {13, 12, 0}, dawn::TextureAspect::Color,
                destination, 0, 256, 0, {4, 4, 1});

    // OOB on the texture because y + width overflows
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {12, 13, 0}, dawn::TextureAspect::Color,
                destination, 0, 256, 0, {4, 4, 1});

    // OOB on the texture because we overflow a non-zero mip
    TestT2BCopy(utils::Expectation::Failure, source, 2, 0, {1, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 256, 0, {4, 4, 1});

    // OOB on the texture even on an empty copy when we copy from a non-existent mip.
    TestT2BCopy(utils::Expectation::Failure, source, 5, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 0, 0, {0, 0, 1});
}

// Test OOB conditions on the buffer
TEST_F(CopyCommandTest_T2B, OutOfBoundsOnBuffer) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Texture source = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                dawn::TextureUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferDst);

    // OOB on the buffer because we copy too many pixels
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 256, 0, {4, 5, 1});

    // OOB on the buffer because of the offset
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 4, 256, 0, {4, 4, 1});

    // OOB on the buffer because (row pitch * (height - 1) + width) * depth overflows
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 512, 0, {4, 3, 1});

    // Not OOB on the buffer although row pitch * height overflows
    // but (row pitch * (height - 1) + width) * depth does not overlow
    {
        uint32_t destinationBufferSize = BufferSizeForTextureCopy(7, 3, 1);
        ASSERT_TRUE(256 * 3 > destinationBufferSize) << "row pitch * height should overflow buffer";
        dawn::Buffer destinationBuffer = CreateBuffer(destinationBufferSize, dawn::BufferUsageBit::TransferDst);
        TestT2BCopy(utils::Expectation::Success, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                    destinationBuffer, 0, 256, 0, {7, 3, 1});
    }
}

// Test that we force Z=0 and Depth=1 on copies from to 2D textures
TEST_F(CopyCommandTest_T2B, ZDepthConstraintFor2DTextures) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Texture source = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                dawn::TextureUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferDst);

    // Z=1 on an empty copy still errors
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 1}, dawn::TextureAspect::Color,
                destination, 0, 0, 0, {0, 0, 1});

    // Depth=0 on an empty copy still errors
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 0, 0, {0, 0, 0});
}

// Test T2B copies with incorrect buffer usage
TEST_F(CopyCommandTest_T2B, IncorrectUsage) {
    uint32_t bufferSize = BufferSizeForTextureCopy(4, 4, 1);
    dawn::Texture source = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                dawn::TextureUsageBit::TransferSrc);
    dawn::Texture sampled = Create2DTexture(16, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                                 dawn::TextureUsageBit::Sampled);
    dawn::Buffer destination = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferDst);
    dawn::Buffer vertex = CreateBuffer(bufferSize, dawn::BufferUsageBit::Vertex);

    // Incorrect source usage
    TestT2BCopy(utils::Expectation::Failure, sampled, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 256, 0, {4, 4, 1});

    // Incorrect destination usage
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                vertex, 0, 256, 0, {4, 4, 1});
}

TEST_F(CopyCommandTest_T2B, IncorrectRowPitch) {
    uint32_t bufferSize = BufferSizeForTextureCopy(128, 16, 1);
    dawn::Texture source = Create2DTexture(128, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
        dawn::TextureUsageBit::TransferDst);
    dawn::Buffer destination = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferSrc);

    // Default row pitch is not 256-byte aligned
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 256, 0, {3, 4, 1});

    // Row pitch is not 256-byte aligned
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 257, 0, {4, 4, 1});

    // Row pitch is less than width * bytesPerPixel
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, 0, 256, 0, {65, 1, 1});
}

// Test T2B copies with incorrect buffer offset usage
TEST_F(CopyCommandTest_T2B, IncorrectBufferOffset) {
    uint32_t bufferSize = BufferSizeForTextureCopy(128, 16, 1);
    dawn::Texture source = Create2DTexture(128, 16, 5, 1, dawn::TextureFormat::R8G8B8A8Unorm,
                                           dawn::TextureUsageBit::TransferSrc);
    dawn::Buffer destination = CreateBuffer(bufferSize, dawn::BufferUsageBit::TransferDst);

    // Correct usage
    TestT2BCopy(utils::Expectation::Success, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, bufferSize - 4, 256, 0, {1, 1, 1});

    // Incorrect usages
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, bufferSize - 5, 256, 0, {1, 1, 1});
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, bufferSize - 6, 256, 0, {1, 1, 1});
    TestT2BCopy(utils::Expectation::Failure, source, 0, 0, {0, 0, 0}, dawn::TextureAspect::Color,
                destination, bufferSize - 7, 256, 0, {1, 1, 1});
}

