// Copyright 2020 The Dawn Authors
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

#include "common/Assert.h"
#include "common/Constants.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/WGPUHelpers.h"

class StorageTextureTests : public DawnTest {
  public:
    // TODO(jiawei.shao@intel.com): support all formats that can be used in storage textures.
    static std::vector<uint32_t> GetExpectedData() {
        constexpr size_t kDataCount = kWidth * kHeight;
        std::vector<uint32_t> outputData(kDataCount);
        for (size_t i = 0; i < kDataCount; ++i) {
            outputData[i] = static_cast<uint32_t>(i + 1u);
        }
        return outputData;
    }

    wgpu::Texture CreateTexture(wgpu::TextureFormat format,
                                wgpu::TextureUsage usage,
                                uint32_t width = kWidth,
                                uint32_t height = kHeight) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {width, height, 1};
        descriptor.format = format;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }

    wgpu::Buffer CreateEmptyBufferForTextureCopy(uint32_t texelSize) {
        ASSERT(kWidth * texelSize <= kTextureBytesPerRowAlignment);
        const size_t uploadBufferSize =
            kTextureBytesPerRowAlignment * (kHeight - 1) + kWidth * texelSize;
        wgpu::BufferDescriptor descriptor;
        descriptor.size = uploadBufferSize;
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        return device.CreateBuffer(&descriptor);
    }

    // TODO(jiawei.shao@intel.com): support all formats that can be used in storage textures.
    wgpu::Texture CreateTextureWithTestData(const std::vector<uint32_t>& initialTextureData,
                                            uint32_t texelSize) {
        ASSERT(kWidth * texelSize <= kTextureBytesPerRowAlignment);
        const size_t uploadBufferSize =
            kTextureBytesPerRowAlignment * (kHeight - 1) + kWidth * texelSize;
        std::vector<uint32_t> uploadBufferData(uploadBufferSize / texelSize);

        const size_t texelCountPerRow = kTextureBytesPerRowAlignment / texelSize;
        for (size_t y = 0; y < kHeight; ++y) {
            for (size_t x = 0; x < kWidth; ++x) {
                uint32_t data = initialTextureData[kWidth * y + x];

                size_t indexInUploadBuffer = y * texelCountPerRow + x;
                uploadBufferData[indexInUploadBuffer] = data;
            }
        }
        wgpu::Buffer uploadBuffer =
            utils::CreateBufferFromData(device, uploadBufferData.data(), uploadBufferSize,
                                        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

        wgpu::Texture outputTexture =
            CreateTexture(wgpu::TextureFormat::R32Uint,
                          wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopyDst);

        wgpu::BufferCopyView bufferCopyView =
            utils::CreateBufferCopyView(uploadBuffer, 0, kTextureBytesPerRowAlignment, 0);
        wgpu::TextureCopyView textureCopyView;
        textureCopyView.texture = outputTexture;
        wgpu::Extent3D copyExtent = {kWidth, kHeight, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copyExtent);
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        return outputTexture;
    }

    wgpu::ComputePipeline CreateComputePipeline(const char* computeShader) {
        wgpu::ShaderModule csModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Compute, computeShader);
        wgpu::ComputePipelineDescriptor computeDescriptor;
        computeDescriptor.layout = nullptr;
        computeDescriptor.computeStage.module = csModule;
        computeDescriptor.computeStage.entryPoint = "main";
        return device.CreateComputePipeline(&computeDescriptor);
    }

    wgpu::RenderPipeline CreateRenderPipeline(const char* vertexShader,
                                              const char* fragmentShader) {
        wgpu::ShaderModule vsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Vertex, vertexShader);
        wgpu::ShaderModule fsModule =
            utils::CreateShaderModule(device, utils::SingleShaderStage::Fragment, fragmentShader);

        utils::ComboRenderPipelineDescriptor desc(device);
        desc.vertexStage.module = vsModule;
        desc.cFragmentStage.module = fsModule;
        desc.cColorStates[0].format = kOutputAttachmentFormat;
        desc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        return device.CreateRenderPipeline(&desc);
    }

    void CheckDrawsGreen(const char* vertexShader,
                         const char* fragmentShader,
                         wgpu::Texture readonlyStorageTexture) {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(vertexShader, fragmentShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, readonlyStorageTexture.CreateView()}});

        // Clear the output attachment to red at the beginning of the render pass.
        wgpu::Texture outputTexture =
            CreateTexture(kOutputAttachmentFormat,
                          wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc, 1, 1);
        utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDescriptor.cColorAttachments[0].clearColor = {1.f, 0.f, 0.f, 1.f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.Draw(1);
        renderPassEncoder.EndPass();

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the output texture are all as expected (green).
        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, outputTexture, 0, 0);
    }

    void CheckResultInStorageBuffer(wgpu::Texture readonlyStorageTexture,
                                    const std::string& computeShader) {
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader.c_str());

        // Clear the content of the result buffer into 0.
        constexpr uint32_t kInitialValue = 0;
        wgpu::Buffer resultBuffer =
            utils::CreateBufferFromData(device, &kInitialValue, sizeof(kInitialValue),
                                        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, readonlyStorageTexture.CreateView()}, {1, resultBuffer}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computeEncoder = encoder.BeginComputePass();
        computeEncoder.SetBindGroup(0, bindGroup);
        computeEncoder.SetPipeline(pipeline);
        computeEncoder.Dispatch(1);
        computeEncoder.EndPass();

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the result buffer are what we expect.
        constexpr uint32_t kExpectedValue = 1u;
        EXPECT_BUFFER_U32_RANGE_EQ(&kExpectedValue, resultBuffer, 0, 1u);
    }

    void WriteIntoStorageTextureInRenderPass(wgpu::Texture writeonlyStorageTexture,
                                             const char* kVertexShader,
                                             const char* kFragmentShader) {
        // Create a render pipeline that writes the expected pixel values into the storage texture
        // without fragment shader outputs.
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(kVertexShader, kFragmentShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, writeonlyStorageTexture.CreateView()}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        // TODO(jiawei.shao@intel.com): remove the output attachment when Dawn supports beginning a
        // render pass with no attachments.
        wgpu::Texture dummyOutputTexture =
            CreateTexture(kOutputAttachmentFormat,
                          wgpu::TextureUsage::OutputAttachment | wgpu::TextureUsage::CopySrc, 1, 1);
        utils::ComboRenderPassDescriptor renderPassDescriptor({dummyOutputTexture.CreateView()});
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.Draw(1);
        renderPassEncoder.EndPass();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void WriteIntoStorageTextureInComputePass(wgpu::Texture writeonlyStorageTexture,
                                              const char* computeShader) {
        // Create a compute pipeline that writes the expected pixel values into the storage texture.
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, writeonlyStorageTexture.CreateView()}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup);
        computePassEncoder.SetPipeline(pipeline);
        computePassEncoder.Dispatch(1);
        computePassEncoder.EndPass();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void CheckOutputStorageTexture(wgpu::Texture writeonlyStorageTexture,
                                   uint32_t texelSize,
                                   const std::vector<uint32_t>& expectedData) {
        // Copy the content from the write-only storage texture to the result buffer.
        wgpu::Buffer resultBuffer = CreateEmptyBufferForTextureCopy(texelSize);
        wgpu::BufferCopyView bufferCopyView =
            utils::CreateBufferCopyView(resultBuffer, 0, kTextureBytesPerRowAlignment, 0);
        wgpu::TextureCopyView textureCopyView;
        textureCopyView.texture = writeonlyStorageTexture;
        wgpu::Extent3D copyExtent = {kWidth, kHeight, 1};

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &copyExtent);

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the result buffer are what we expect.
        for (size_t y = 0; y < kHeight; ++y) {
            const size_t resultBufferOffset = kTextureBytesPerRowAlignment * y;
            EXPECT_BUFFER_U32_RANGE_EQ(expectedData.data() + kWidth * y, resultBuffer,
                                       resultBufferOffset, kWidth);
        }
    }

    static constexpr size_t kWidth = 4u;
    static constexpr size_t kHeight = 4u;
    static constexpr wgpu::TextureFormat kOutputAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

    const char* kSimpleVertexShader = R"(
        #version 450
        void main() {
            gl_Position = vec4(0.f, 0.f, 0.f, 1.f);
            gl_PointSize = 1.0f;
        })";

    const char* kCommonReadOnlyTestCode_uimage2D = R"(
        bool doTest() {
            for (uint y = 0; y < 4; ++y) {
                for (uint x = 0; x < 4; ++x) {
                    uvec4 expected = uvec4(1u + x + y * 4u, 0, 0, 1u);
                    uvec4 pixel = imageLoad(srcImage, ivec2(x, y));
                    if (pixel != expected) {
                        return false;
                    }
                }
            }
            return true;
        })";

    const char* kCommonWriteOnlyTestCode_uimage2D = R"(
        #version 450
        layout(set = 0, binding = 0, r32ui) uniform writeonly uimage2D dstImage;
        void main() {
            for (uint y = 0; y < 4; ++y) {
                for (uint x = 0; x < 4; ++x) {
                    uvec4 pixel = uvec4(1u + x + y * 4u, 0, 0, 1u);
                    imageStore(dstImage, ivec2(x, y), pixel);
                }
            }
        })";
};

// Test that using read-only storage texture and write-only storage texture in BindGroupLayout is
// valid on all backends. This test is a regression test for chromium:1061156 and passes by not
// asserting or crashing.
TEST_P(StorageTextureTests, BindGroupLayoutWithStorageTextureBindingType) {
    // wgpu::BindingType::ReadonlyStorageTexture is a valid binding type to create a bind group
    // layout.
    {
        wgpu::BindGroupLayoutEntry entry = {0, wgpu::ShaderStage::Compute,
                                            wgpu::BindingType::ReadonlyStorageTexture};
        entry.storageTextureFormat = wgpu::TextureFormat::R32Float;
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        device.CreateBindGroupLayout(&descriptor);
    }

    // wgpu::BindingType::WriteonlyStorageTexture is a valid binding type to create a bind group
    // layout.
    {
        wgpu::BindGroupLayoutEntry entry = {0, wgpu::ShaderStage::Compute,
                                            wgpu::BindingType::WriteonlyStorageTexture};
        entry.storageTextureFormat = wgpu::TextureFormat::R32Float;
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        device.CreateBindGroupLayout(&descriptor);
    }
}

// Test that read-only storage textures are supported in compute shader.
TEST_P(StorageTextureTests, ReadonlyStorageTextureInComputeShader) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsSpvcParserBeingUsed());

    // Prepare the read-only storage texture and fill it with the expected data.
    // TODO(jiawei.shao@intel.com): test more texture formats.
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    const std::vector<uint32_t> kInitialTextureData = GetExpectedData();
    wgpu::Texture readonlyStorageTexture =
        CreateTextureWithTestData(kInitialTextureData, kTexelSizeR32Uint);

    // Create a compute shader that reads the pixels from the read-only storage texture and writes 1
    // to DstBuffer if they all have to expected value.
    const std::string kComputeShader = std::string(R"(
        #version 450
        layout (set = 0, binding = 0, r32ui) uniform readonly uimage2D srcImage;
        layout (set = 0, binding = 1, std430) buffer DstBuffer {
            uint result;
        } dstBuffer;)") + kCommonReadOnlyTestCode_uimage2D +
                                       R"(
        void main() {
            if (doTest()) {
                dstBuffer.result = 1;
            } else {
                dstBuffer.result = 0;
            }
        })";

    CheckResultInStorageBuffer(readonlyStorageTexture, kComputeShader);
}

// Test that read-only storage textures are supported in vertex shader.
TEST_P(StorageTextureTests, ReadonlyStorageTextureInVertexShader) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsSpvcParserBeingUsed());

    // Prepare the read-only storage texture and fill it with the expected data.
    // TODO(jiawei.shao@intel.com): test more texture formats
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    const std::vector<uint32_t> kInitialTextureData = GetExpectedData();
    wgpu::Texture readonlyStorageTexture =
        CreateTextureWithTestData(kInitialTextureData, kTexelSizeR32Uint);

    // Create a rendering pipeline that reads the pixels from the read-only storage texture and uses
    // green as the output color, otherwise uses red instead.
    const std::string kVertexShader = std::string(R"(
            #version 450
            layout(set = 0, binding = 0, r32ui) uniform readonly uimage2D srcImage;
            layout(location = 0) out vec4 o_color;)") +
                                      kCommonReadOnlyTestCode_uimage2D + R"(
            void main() {
                gl_Position = vec4(0.f, 0.f, 0.f, 1.f);
                if (doTest()) {
                    o_color = vec4(0.f, 1.f, 0.f, 1.f);
                } else {
                    o_color = vec4(1.f, 0.f, 0.f, 1.f);
                }
                gl_PointSize = 1.0f;
            })";
    const char* kFragmentShader = R"(
            #version 450
            layout(location = 0) in vec4 o_color;
            layout(location = 0) out vec4 fragColor;
            void main() {
                fragColor = o_color;
            })";
    CheckDrawsGreen(kVertexShader.c_str(), kFragmentShader, readonlyStorageTexture);
}

// Test that read-only storage textures are supported in fragment shader.
TEST_P(StorageTextureTests, ReadonlyStorageTextureInFragmentShader) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsSpvcParserBeingUsed());

    // Prepare the read-only storage texture and fill it with the expected data.
    // TODO(jiawei.shao@intel.com): test more texture formats
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    const std::vector<uint32_t> kInitialTextureData = GetExpectedData();
    wgpu::Texture readonlyStorageTexture =
        CreateTextureWithTestData(kInitialTextureData, kTexelSizeR32Uint);

    // Create a rendering pipeline that reads the pixels from the read-only storage texture and uses
    // green as the output color if the pixel value is expected, otherwise uses red instead.
    const char* kVertexShader = kSimpleVertexShader;
    const std::string kFragmentShader = std::string(R"(
            #version 450
            layout(set = 0, binding = 0, r32ui) uniform readonly uimage2D srcImage;
            layout(location = 0) out vec4 o_color;)") +
                                        kCommonReadOnlyTestCode_uimage2D + R"(
            void main() {
                if (doTest()) {
                    o_color = vec4(0.f, 1.f, 0.f, 1.f);
                } else {
                    o_color = vec4(1.f, 0.f, 0.f, 1.f);
                }
            })";
    CheckDrawsGreen(kVertexShader, kFragmentShader.c_str(), readonlyStorageTexture);
}

// Test that write-only storage textures are supported in compute shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInComputeShader) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on D3D12 and OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsSpvcParserBeingUsed());

    // Prepare the write-only storage texture.
    // TODO(jiawei.shao@intel.com): test more texture formats.
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture,
                                         kCommonWriteOnlyTestCode_uimage2D);
    CheckOutputStorageTexture(writeonlyStorageTexture, kTexelSizeR32Uint, GetExpectedData());
}

// Test that write-only storage textures are supported in fragment shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInFragmentShader) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on D3D12 and OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsSpvcParserBeingUsed());

    // Prepare the write-only storage texture.
    // TODO(jiawei.shao@intel.com): test more texture formats.
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

    WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                        kCommonWriteOnlyTestCode_uimage2D);
    CheckOutputStorageTexture(writeonlyStorageTexture, kTexelSizeR32Uint, GetExpectedData());
}

DAWN_INSTANTIATE_TEST(StorageTextureTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      VulkanBackend());

class StorageTextureZeroInitTests : public StorageTextureTests {
  public:
    static std::vector<uint32_t> GetExpectedData() {
        constexpr size_t kDataCount = kWidth * kHeight;
        std::vector<uint32_t> outputData(kDataCount, 0);
        outputData[0] = 1u;
        return outputData;
    }

    const char* kCommonReadOnlyZeroInitTestCode = R"(
        bool doTest() {
            for (uint y = 0; y < 4; ++y) {
                for (uint x = 0; x < 4; ++x) {
                    uvec4 pixel = imageLoad(srcImage, ivec2(x, y));
                    if (pixel != uvec4(0, 0, 0, 1u)) {
                        return false;
                    }
                }
            }
            return true;
        })";

    const char* kCommonWriteOnlyZeroInitTestCode = R"(
        #version 450
        layout(set = 0, binding = 0, r32ui) uniform writeonly uimage2D dstImage;
        void main() {
            imageStore(dstImage, ivec2(0, 0), uvec4(1u, 0, 0, 1u));
        })";
};

// Verify that the texture is correctly cleared to 0 before its first usage as a read-only storage
// texture in a render pass.
TEST_P(StorageTextureZeroInitTests, ReadonlyStorageTextureClearsToZeroInRenderPass) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsSpvcParserBeingUsed());

    wgpu::Texture readonlyStorageTexture =
        CreateTexture(wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage);

    // Create a rendering pipeline that reads the pixels from the read-only storage texture and uses
    // green as the output color, otherwise uses red instead.
    const char* kVertexShader = kSimpleVertexShader;
    const std::string kFragmentShader = std::string(R"(
            #version 450
            layout(set = 0, binding = 0, r32ui) uniform readonly uimage2D srcImage;
            layout(location = 0) out vec4 o_color;)") +
                                        kCommonReadOnlyZeroInitTestCode +
                                        R"(

            void main() {
                if (doTest()) {
                    o_color = vec4(0.f, 1.f, 0.f, 1.f);
                } else {
                    o_color = vec4(1.f, 0.f, 0.f, 1.f);
                }
            })";
    CheckDrawsGreen(kVertexShader, kFragmentShader.c_str(), readonlyStorageTexture);
}

// Verify that the texture is correctly cleared to 0 before its first usage as a read-only storage
// texture in a compute pass.
TEST_P(StorageTextureZeroInitTests, ReadonlyStorageTextureClearsToZeroInComputePass) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsSpvcParserBeingUsed());

    wgpu::Texture readonlyStorageTexture =
        CreateTexture(wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage);

    // Create a compute shader that reads the pixels from the read-only storage texture and writes 1
    // to DstBuffer if they all have to expected value.
    const std::string kComputeShader = std::string(R"(
        #version 450
        layout (set = 0, binding = 0, r32ui) uniform readonly uimage2D srcImage;
        layout (set = 0, binding = 1, std430) buffer DstBuffer {
            uint result;
        } dstBuffer;)") + kCommonReadOnlyZeroInitTestCode +
                                       R"(

        void main() {
            if (doTest()) {
                dstBuffer.result = 1;
            } else {
                dstBuffer.result = 0;
            }
        })";

    CheckResultInStorageBuffer(readonlyStorageTexture, kComputeShader);
}

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// storage texture in a render pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInRenderPass) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on D3D12 and OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsSpvcParserBeingUsed());

    // Prepare the write-only storage texture.
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

    WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                        kCommonWriteOnlyZeroInitTestCode);
    CheckOutputStorageTexture(writeonlyStorageTexture, kTexelSizeR32Uint, GetExpectedData());
}

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// texture in a compute pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInComputePass) {
    // TODO(jiawei.shao@intel.com): support read-only storage texture on D3D12 and OpenGL.
    DAWN_SKIP_TEST_IF(IsOpenGL());

    // When we run dawn_end2end_tests with "--use-spvc-parser", extracting the binding type of a
    // read-only image will always return shaderc_spvc_binding_type_writeonly_storage_texture.
    // TODO(jiawei.shao@intel.com): enable this test when we specify "--use-spvc-parser" after the
    // bug in spvc parser is fixed.
    DAWN_SKIP_TEST_IF(IsD3D12() && IsSpvcParserBeingUsed());

    // Prepare the write-only storage texture.
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, kCommonWriteOnlyZeroInitTestCode);
    CheckOutputStorageTexture(writeonlyStorageTexture, kTexelSizeR32Uint, GetExpectedData());
}

DAWN_INSTANTIATE_TEST(StorageTextureZeroInitTests,
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"}));
