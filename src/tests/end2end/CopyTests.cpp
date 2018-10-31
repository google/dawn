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

#include "tests/DawnTest.h"

#include <array>
#include "common/Constants.h"
#include "common/Math.h"
#include "utils/DawnHelpers.h"

class CopyTests : public DawnTest {
    protected:
        static constexpr unsigned int kBytesPerTexel = 4;

        struct TextureSpec {
            uint32_t width;
            uint32_t height;
            uint32_t x;
            uint32_t y;
            uint32_t copyWidth;
            uint32_t copyHeight;
            uint32_t level;
            uint32_t arrayLayer = 1u;
        };

        struct BufferSpec {
            uint32_t size;
            uint32_t offset;
            uint32_t rowPitch;
        };

        BufferSpec MinimumBufferSpec(uint32_t width, uint32_t height) {
            uint32_t rowPitch = Align(width * kBytesPerTexel, kTextureRowPitchAlignment);
            return { rowPitch * (height - 1) + width * kBytesPerTexel, 0, rowPitch };
        }

        static void PackTextureData(const RGBA8* srcData, uint32_t width, uint32_t height, uint32_t srcTexelsPerRow, RGBA8* dstData, uint32_t dstTexelsPerRow) {
            for (unsigned int y = 0; y < height; ++y) {
                for (unsigned int x = 0; x < width; ++x) {
                    unsigned int src = x + y * srcTexelsPerRow;
                    unsigned int dst = x + y * dstTexelsPerRow;
                    dstData[dst] = srcData[src];
                }
            }
        }
};

class CopyTests_T2B : public CopyTests {
    protected:
        static void FillTextureData(uint32_t width, uint32_t height, uint32_t texelsPerRow, RGBA8* data, uint32_t layer) {
            for (unsigned int y = 0; y < height; ++y) {
                for (unsigned int x = 0; x < width; ++x) {
                    unsigned int i = x + y * texelsPerRow;
                    data[i] = RGBA8(
                        static_cast<uint8_t>((x + layer * x)% 256),
                        static_cast<uint8_t>((y + layer * y)% 256),
                        static_cast<uint8_t>(x / 256),
                        static_cast<uint8_t>(y / 256));
                }
            }
        }

        void DoTest(const TextureSpec& textureSpec, const BufferSpec& bufferSpec) {
            // Create a texture that is `width` x `height` with (`level` + 1) mip levels.
            dawn::TextureDescriptor descriptor;
            descriptor.dimension = dawn::TextureDimension::e2D;
            descriptor.size.width = textureSpec.width;
            descriptor.size.height = textureSpec.height;
            descriptor.size.depth = 1;
            descriptor.arrayLayer = textureSpec.arrayLayer;
            descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
            descriptor.levelCount = textureSpec.level + 1;
            descriptor.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::TransferSrc;
            dawn::Texture texture = device.CreateTexture(&descriptor);

            uint32_t width = textureSpec.width >> textureSpec.level;
            uint32_t height = textureSpec.height >> textureSpec.level;
            uint32_t rowPitch = Align(kBytesPerTexel * width, kTextureRowPitchAlignment);
            uint32_t texelsPerRow = rowPitch / kBytesPerTexel;
            uint32_t texelCountPerLayer = texelsPerRow * (height - 1) + width;

            dawn::CommandBufferBuilder cmdBuilder = device.CreateCommandBufferBuilder();

            std::vector<std::vector<RGBA8>> textureArrayData(textureSpec.arrayLayer);
            for (uint32_t slice = 0; slice < textureSpec.arrayLayer; ++slice) {
                textureArrayData[slice].resize(texelCountPerLayer);
                FillTextureData(width, height, rowPitch / kBytesPerTexel, textureArrayData[slice].data(), slice);

                // Create an upload buffer and use it to populate the current slice of the texture in `level` mip level
                dawn::Buffer uploadBuffer = utils::CreateBufferFromData(device, textureArrayData[slice].data(),
                    static_cast<uint32_t>(sizeof(RGBA8) * textureArrayData[slice].size()), dawn::BufferUsageBit::TransferSrc);

                cmdBuilder.CopyBufferToTexture(uploadBuffer, 0, rowPitch, texture, 0, 0, 0, width, height, 1, textureSpec.level, slice);
            }

            // Create a buffer of size `size * textureSpec.arrayLayer` and populate it with empty data (0,0,0,0)
            // Note: Prepopulating the buffer with empty data ensures that there is not random data in the expectation
            // and helps ensure that the padding due to the row pitch is not modified by the copy
            dawn::BufferDescriptor bufDescriptor;
            bufDescriptor.size = bufferSpec.size * textureSpec.arrayLayer;
            bufDescriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
            dawn::Buffer buffer = device.CreateBuffer(&bufDescriptor);
            std::vector<RGBA8> emptyData(bufferSpec.size / kBytesPerTexel * textureSpec.arrayLayer);
            buffer.SetSubData(0, static_cast<uint32_t>(emptyData.size() * sizeof(RGBA8)), reinterpret_cast<const uint8_t*>(emptyData.data()));

            uint32_t bufferOffset = bufferSpec.offset;
            for (uint32_t slice = 0; slice < textureSpec.arrayLayer; ++slice) {
                // Copy the region [(`x`, `y`), (`x + copyWidth, `y + copyWidth`)] from the `level` mip into the buffer at `offset + bufferSpec.size * slice` and `rowPitch`
                cmdBuilder.CopyTextureToBuffer(texture, textureSpec.x, textureSpec.y, 0, textureSpec.copyWidth, textureSpec.copyHeight, 1, textureSpec.level, slice, buffer, bufferOffset, bufferSpec.rowPitch);
                bufferOffset += bufferSpec.size;
            }

            dawn::CommandBuffer commands = cmdBuilder.GetResult();
            queue.Submit(1, &commands);

            bufferOffset = bufferSpec.offset;
            std::vector<RGBA8> expected(bufferSpec.rowPitch / kBytesPerTexel * (textureSpec.copyHeight - 1) + textureSpec.copyWidth);
            for (uint32_t slice = 0; slice < textureSpec.arrayLayer; ++slice) {
                // Pack the data used to create the upload buffer in the specified copy region to have the same format as the expected buffer data.
                std::fill(expected.begin(), expected.end(), RGBA8());
                PackTextureData(
                    &textureArrayData[slice][textureSpec.x + textureSpec.y * (rowPitch / kBytesPerTexel)],
                    textureSpec.copyWidth,
                    textureSpec.copyHeight,
                    rowPitch / kBytesPerTexel,
                    expected.data(),
                    bufferSpec.rowPitch / kBytesPerTexel);

                EXPECT_BUFFER_U32_RANGE_EQ(reinterpret_cast<const uint32_t*>(expected.data()), buffer, bufferOffset, static_cast<uint32_t>(expected.size())) <<
                    "Texture to Buffer copy failed copying region [(" << textureSpec.x << ", " << textureSpec.y << "), (" << textureSpec.x + textureSpec.copyWidth << ", " << textureSpec.y + textureSpec.copyHeight <<
                    ")) from " << textureSpec.width << " x " << textureSpec.height << " texture at mip level " << textureSpec.level << " layer " << slice <<
                    " to " << bufDescriptor.size << "-byte buffer with offset " << bufferOffset << " and row pitch " << bufferSpec.rowPitch << std::endl;

                bufferOffset += bufferSpec.size;
            }
        }


};

class CopyTests_B2T : public CopyTests {
protected:
    static void FillBufferData(RGBA8* data, size_t count) {
        for (size_t i = 0; i < count; ++i) {
            data[i] = RGBA8(
                static_cast<uint8_t>(i % 256),
                static_cast<uint8_t>((i / 256) % 256),
                static_cast<uint8_t>((i / 256 / 256) % 256),
                255);
        }
    }

    void DoTest(const TextureSpec& textureSpec, const BufferSpec& bufferSpec) {
        // Create a buffer of size `size` and populate it with data
        dawn::BufferDescriptor bufDescriptor;
        bufDescriptor.size = bufferSpec.size;
        bufDescriptor.usage = dawn::BufferUsageBit::TransferSrc | dawn::BufferUsageBit::TransferDst;
        dawn::Buffer buffer = device.CreateBuffer(&bufDescriptor);

        std::vector<RGBA8> bufferData(bufferSpec.size / kBytesPerTexel);
        FillBufferData(bufferData.data(), bufferData.size());
        buffer.SetSubData(0, static_cast<uint32_t>(bufferData.size() * sizeof(RGBA8)), reinterpret_cast<const uint8_t*>(bufferData.data()));

        // Create a texture that is `width` x `height` with (`level` + 1) mip levels.
        dawn::TextureDescriptor descriptor;
        descriptor.dimension = dawn::TextureDimension::e2D;
        descriptor.size.width = textureSpec.width;
        descriptor.size.height = textureSpec.height;
        descriptor.size.depth = 1;
        descriptor.arrayLayer = 1;
        descriptor.format = dawn::TextureFormat::R8G8B8A8Unorm;
        descriptor.levelCount = textureSpec.level + 1;
        descriptor.usage = dawn::TextureUsageBit::TransferDst | dawn::TextureUsageBit::TransferSrc;
        dawn::Texture texture = device.CreateTexture(&descriptor);

        dawn::CommandBufferBuilder cmdBuilder = device.CreateCommandBufferBuilder();

        // Create an upload buffer filled with empty data and use it to populate the `level` mip of the texture
        // Note: Prepopulating the texture with empty data ensures that there is not random data in the expectation
        // and helps ensure that the padding due to the row pitch is not modified by the copy
        {
            uint32_t width = textureSpec.width >> textureSpec.level;
            uint32_t height = textureSpec.height >> textureSpec.level;
            uint32_t rowPitch = Align(kBytesPerTexel * width, kTextureRowPitchAlignment);
            uint32_t texelsPerRow = rowPitch / kBytesPerTexel;
            uint32_t texelCount = texelsPerRow * (height - 1) + width;

            std::vector<RGBA8> emptyData(texelCount);
            dawn::Buffer uploadBuffer = utils::CreateBufferFromData(device, emptyData.data(), static_cast<uint32_t>(sizeof(RGBA8) * emptyData.size()), dawn::BufferUsageBit::TransferSrc);

            cmdBuilder.CopyBufferToTexture(uploadBuffer, 0, rowPitch, texture, 0, 0, 0, width, height, 1, textureSpec.level, 0);
        }

        // Copy to the region [(`x`, `y`), (`x + copyWidth, `y + copyWidth`)] at the `level` mip from the buffer at the specified `offset` and `rowPitch`
        cmdBuilder.CopyBufferToTexture(buffer, bufferSpec.offset, bufferSpec.rowPitch, texture, textureSpec.x, textureSpec.y, 0, textureSpec.copyWidth, textureSpec.copyHeight, 1, textureSpec.level, 0);

        dawn::CommandBuffer commands = cmdBuilder.GetResult();
        queue.Submit(1, &commands);

        // Pack the data used to create the buffer in the specified copy region to have the same format as the expected texture data.
        uint32_t rowPitch = Align(kBytesPerTexel * textureSpec.copyWidth, kTextureRowPitchAlignment);
        std::vector<RGBA8> expected(rowPitch / kBytesPerTexel * (textureSpec.copyHeight - 1) + textureSpec.copyWidth);
        PackTextureData(&bufferData[bufferSpec.offset / kBytesPerTexel], textureSpec.copyWidth, textureSpec.copyHeight, bufferSpec.rowPitch / kBytesPerTexel, expected.data(), textureSpec.copyWidth);

        EXPECT_TEXTURE_RGBA8_EQ(expected.data(), texture, textureSpec.x, textureSpec.y, textureSpec.copyWidth, textureSpec.copyHeight, textureSpec.level) <<
            "Buffer to Texture copy failed copying "
            << bufferSpec.size << "-byte buffer with offset " << bufferSpec.offset << " and row pitch " << bufferSpec.rowPitch << " to [("
            << textureSpec.x << ", " << textureSpec.y << "), (" << textureSpec.x + textureSpec.copyWidth << ", " << textureSpec.y + textureSpec.copyHeight <<
            ")) region of " << textureSpec.width << " x " << textureSpec.height << " texture at mip level " << textureSpec.level << std::endl;
    }


};

// Test that copying an entire texture with 256-byte aligned dimensions works
TEST_P(CopyTests_T2B, FullTextureAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, MinimumBufferSpec(kWidth, kHeight));
}

// Test that copying an entire texture without 256-byte aligned dimensions works
TEST_P(CopyTests_T2B, FullTextureUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, MinimumBufferSpec(kWidth, kHeight));
}

// Test that reading pixels from a 256-byte aligned texture works
TEST_P(CopyTests_T2B, PixelReadAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);
    DoTest({ kWidth, kHeight, 0, 0, 1, 1, 0 },                    pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, 0, 1, 1, 0 },           pixelBuffer);
    DoTest({ kWidth, kHeight, 0, kHeight - 1, 1, 1, 0 },          pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, kHeight - 1, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 3, kHeight / 7, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 7, kHeight / 3, 1, 1, 0 }, pixelBuffer);
}

// Test that copying pixels from a texture that is not 256-byte aligned works
TEST_P(CopyTests_T2B, PixelReadUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);
    DoTest({ kWidth, kHeight, 0, 0, 1, 1, 0 },                    pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, 0, 1, 1, 0 },           pixelBuffer);
    DoTest({ kWidth, kHeight, 0, kHeight - 1, 1, 1, 0 },          pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, kHeight - 1, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 3, kHeight / 7, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 7, kHeight / 3, 1, 1, 0 }, pixelBuffer);
}

// Test that copying regions with 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureRegionAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int w : {64, 128, 256}) {
        for (unsigned int h : { 16, 32, 48 }) {
            DoTest({ kWidth, kHeight, 0, 0, w, h, 0 }, MinimumBufferSpec(w, h));
        }
    }
}

// Test that copying regions without 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureRegionUnaligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int w : {13, 63, 65}) {
        for (unsigned int h : { 17, 19, 63 }) {
            DoTest({ kWidth, kHeight, 0, 0, w, h, 0 }, MinimumBufferSpec(w, h));
        }
    }
}

// Test that copying mips with 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureMipAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int i = 1; i < 4; ++i) {
        DoTest({ kWidth, kHeight, 0, 0, kWidth >> i, kHeight >> i, i }, MinimumBufferSpec(kWidth >> i, kHeight >> i));
    }
}

// Test that copying mips without 256-byte aligned sizes works
TEST_P(CopyTests_T2B, TextureMipUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    for (unsigned int i = 1; i < 4; ++i) {
        DoTest({ kWidth, kHeight, 0, 0, kWidth >> i, kHeight >> i, i }, MinimumBufferSpec(kWidth >> i, kHeight >> i));
    }
}

// Test that copying with a 512-byte aligned buffer offset works
TEST_P(CopyTests_T2B, OffsetBufferAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int i = 0; i < 3; ++i) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        uint32_t offset = 512 * i;
        bufferSpec.size += offset;
        bufferSpec.offset += offset;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying without a 512-byte aligned buffer offset works
TEST_P(CopyTests_T2B, OffsetBufferUnaligned) {
    constexpr uint32_t kWidth = 128;
    constexpr uint32_t kHeight = 128;
    for (uint32_t i = kBytesPerTexel; i < 512; i += kBytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying without a 512-byte aligned buffer offset that is greater than the row pitch works
TEST_P(CopyTests_T2B, OffsetBufferUnalignedSmallRowPitch) {
    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 128;
    for (uint32_t i = 256 + kBytesPerTexel; i < 512; i += kBytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying with a greater row pitch than needed on a 256-byte aligned texture works
TEST_P(CopyTests_T2B, RowPitchAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.rowPitch += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying with a greater row pitch than needed on a texture that is not 256-byte aligned works
TEST_P(CopyTests_T2B, RowPitchUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.rowPitch += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying regions of each texture 2D array layer works
TEST_P(CopyTests_T2B, Texture2DArrayRegion) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0, kLayers }, MinimumBufferSpec(kWidth, kHeight));
}

// Test that copying texture 2D array mips with 256-byte aligned sizes works
TEST_P(CopyTests_T2B, Texture2DArrayMip) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    constexpr uint32_t kLayers = 6u;
    for (unsigned int i = 1; i < 4; ++i) {
        DoTest({ kWidth, kHeight, 0, 0, kWidth >> i, kHeight >> i, i, kLayers }, MinimumBufferSpec(kWidth >> i, kHeight >> i));
    }
}

DAWN_INSTANTIATE_TEST(CopyTests_T2B, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)

// Test that copying an entire texture with 256-byte aligned dimensions works
TEST_P(CopyTests_B2T, FullTextureAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, MinimumBufferSpec(kWidth, kHeight));
}

// Test that copying an entire texture without 256-byte aligned dimensions works
TEST_P(CopyTests_B2T, FullTextureUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, MinimumBufferSpec(kWidth, kHeight));
}

// Test that reading pixels from a 256-byte aligned texture works
TEST_P(CopyTests_B2T, PixelReadAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);
    DoTest({ kWidth, kHeight, 0, 0, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, 0, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, 0, kHeight - 1, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, kHeight - 1, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 3, kHeight / 7, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 7, kHeight / 3, 1, 1, 0 }, pixelBuffer);
}

// Test that copying pixels from a texture that is not 256-byte aligned works
TEST_P(CopyTests_B2T, PixelReadUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    BufferSpec pixelBuffer = MinimumBufferSpec(1, 1);
    DoTest({ kWidth, kHeight, 0, 0, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, 0, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, 0, kHeight - 1, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth - 1, kHeight - 1, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 3, kHeight / 7, 1, 1, 0 }, pixelBuffer);
    DoTest({ kWidth, kHeight, kWidth / 7, kHeight / 3, 1, 1, 0 }, pixelBuffer);
}

// Test that copying regions with 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureRegionAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int w : {64, 128, 256}) {
        for (unsigned int h : { 16, 32, 48 }) {
            DoTest({ kWidth, kHeight, 0, 0, w, h, 0 }, MinimumBufferSpec(w, h));
        }
    }
}

// Test that copying regions without 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureRegionUnaligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int w : {13, 63, 65}) {
        for (unsigned int h : { 17, 19, 63 }) {
            DoTest({ kWidth, kHeight, 0, 0, w, h, 0 }, MinimumBufferSpec(w, h));
        }
    }
}

// Test that copying mips with 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureMipAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int i = 1; i < 4; ++i) {
        DoTest({ kWidth, kHeight, 0, 0, kWidth >> i, kHeight >> i, i }, MinimumBufferSpec(kWidth >> i, kHeight >> i));
    }
}

// Test that copying mips without 256-byte aligned sizes works
TEST_P(CopyTests_B2T, TextureMipUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    for (unsigned int i = 1; i < 4; ++i) {
        DoTest({ kWidth, kHeight, 0, 0, kWidth >> i, kHeight >> i, i }, MinimumBufferSpec(kWidth >> i, kHeight >> i));
    }
}

// Test that copying with a 512-byte aligned buffer offset works
TEST_P(CopyTests_B2T, OffsetBufferAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (unsigned int i = 0; i < 3; ++i) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        uint32_t offset = 512 * i;
        bufferSpec.size += offset;
        bufferSpec.offset += offset;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying without a 512-byte aligned buffer offset works
TEST_P(CopyTests_B2T, OffsetBufferUnaligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    for (uint32_t i = kBytesPerTexel; i < 512; i += kBytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying without a 512-byte aligned buffer offset that is greater than the row pitch works
TEST_P(CopyTests_B2T, OffsetBufferUnalignedSmallRowPitch) {
    constexpr uint32_t kWidth = 32;
    constexpr uint32_t kHeight = 128;
    for (uint32_t i = 256 + kBytesPerTexel; i < 512; i += kBytesPerTexel * 9) {
        BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
        bufferSpec.size += i;
        bufferSpec.offset += i;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying with a greater row pitch than needed on a 256-byte aligned texture works
TEST_P(CopyTests_B2T, RowPitchAligned) {
    constexpr uint32_t kWidth = 256;
    constexpr uint32_t kHeight = 128;
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.rowPitch += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

// Test that copying with a greater row pitch than needed on a texture that is not 256-byte aligned works
TEST_P(CopyTests_B2T, RowPitchUnaligned) {
    constexpr uint32_t kWidth = 259;
    constexpr uint32_t kHeight = 127;
    BufferSpec bufferSpec = MinimumBufferSpec(kWidth, kHeight);
    for (unsigned int i = 1; i < 4; ++i) {
        bufferSpec.rowPitch += 256;
        bufferSpec.size += 256 * kHeight;
        DoTest({ kWidth, kHeight, 0, 0, kWidth, kHeight, 0 }, bufferSpec);
    }
}

DAWN_INSTANTIATE_TEST(CopyTests_B2T, D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend)
