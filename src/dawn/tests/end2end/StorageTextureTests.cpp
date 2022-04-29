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

#include <string>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/TestUtils.h"
#include "dawn/utils/TextureUtils.h"
#include "dawn/utils/WGPUHelpers.h"

namespace {
    bool OpenGLESSupportsStorageTexture(wgpu::TextureFormat format) {
        // TODO(crbug.com/dawn/595): 32-bit RG* formats are unsupported on OpenGL ES.
        return format != wgpu::TextureFormat::RG32Float &&
               format != wgpu::TextureFormat::RG32Sint && format != wgpu::TextureFormat::RG32Uint;
    }
}  // namespace

class StorageTextureTests : public DawnTest {
  public:
    static void FillExpectedData(void* pixelValuePtr,
                                 wgpu::TextureFormat format,
                                 uint32_t x,
                                 uint32_t y,
                                 uint32_t depthOrArrayLayer) {
        const uint32_t pixelValue = 1 + x + kWidth * (y + kHeight * depthOrArrayLayer);
        ASSERT(pixelValue <= 255u / 4);

        switch (format) {
            // 32-bit unsigned integer formats
            case wgpu::TextureFormat::R32Uint: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                *valuePtr = pixelValue;
                break;
            }

            case wgpu::TextureFormat::RG32Uint: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                valuePtr[0] = pixelValue;
                valuePtr[1] = pixelValue * 2;
                break;
            }

            case wgpu::TextureFormat::RGBA32Uint: {
                uint32_t* valuePtr = static_cast<uint32_t*>(pixelValuePtr);
                valuePtr[0] = pixelValue;
                valuePtr[1] = pixelValue * 2;
                valuePtr[2] = pixelValue * 3;
                valuePtr[3] = pixelValue * 4;
                break;
            }

            // 32-bit signed integer formats
            case wgpu::TextureFormat::R32Sint: {
                int32_t* valuePtr = static_cast<int32_t*>(pixelValuePtr);
                *valuePtr = static_cast<int32_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RG32Sint: {
                int32_t* valuePtr = static_cast<int32_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int32_t>(pixelValue);
                valuePtr[1] = -static_cast<int32_t>(pixelValue);
                break;
            }

            case wgpu::TextureFormat::RGBA32Sint: {
                int32_t* valuePtr = static_cast<int32_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int32_t>(pixelValue);
                valuePtr[1] = -static_cast<int32_t>(pixelValue);
                valuePtr[2] = static_cast<int32_t>(pixelValue * 2);
                valuePtr[3] = -static_cast<int32_t>(pixelValue * 2);
                break;
            }

            // 32-bit float formats
            case wgpu::TextureFormat::R32Float: {
                float_t* valuePtr = static_cast<float_t*>(pixelValuePtr);
                *valuePtr = static_cast<float_t>(pixelValue * 1.1f);
                break;
            }

            case wgpu::TextureFormat::RG32Float: {
                float_t* valuePtr = static_cast<float_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<float_t>(pixelValue * 1.1f);
                valuePtr[1] = -static_cast<float_t>(pixelValue * 2.2f);
                break;
            }

            case wgpu::TextureFormat::RGBA32Float: {
                float_t* valuePtr = static_cast<float_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<float_t>(pixelValue * 1.1f);
                valuePtr[1] = -static_cast<float_t>(pixelValue * 1.1f);
                valuePtr[2] = static_cast<float_t>(pixelValue * 2.2f);
                valuePtr[3] = -static_cast<float_t>(pixelValue * 2.2f);
                break;
            }

            // 16-bit (unsigned integer, signed integer and float) 4-component formats
            case wgpu::TextureFormat::RGBA16Uint: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<uint16_t>(pixelValue);
                valuePtr[1] = static_cast<uint16_t>(pixelValue * 2);
                valuePtr[2] = static_cast<uint16_t>(pixelValue * 3);
                valuePtr[3] = static_cast<uint16_t>(pixelValue * 4);
                break;
            }
            case wgpu::TextureFormat::RGBA16Sint: {
                int16_t* valuePtr = static_cast<int16_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int16_t>(pixelValue);
                valuePtr[1] = -static_cast<int16_t>(pixelValue);
                valuePtr[2] = static_cast<int16_t>(pixelValue * 2);
                valuePtr[3] = -static_cast<int16_t>(pixelValue * 2);
                break;
            }

            case wgpu::TextureFormat::RGBA16Float: {
                uint16_t* valuePtr = static_cast<uint16_t*>(pixelValuePtr);
                valuePtr[0] = Float32ToFloat16(static_cast<float_t>(pixelValue));
                valuePtr[1] = Float32ToFloat16(-static_cast<float_t>(pixelValue));
                valuePtr[2] = Float32ToFloat16(static_cast<float_t>(pixelValue * 2));
                valuePtr[3] = Float32ToFloat16(-static_cast<float_t>(pixelValue * 2));
                break;
            }

            // 8-bit (normalized/non-normalized signed/unsigned integer) 4-component formats
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8Uint: {
                RGBA8* valuePtr = static_cast<RGBA8*>(pixelValuePtr);
                *valuePtr = RGBA8(pixelValue, pixelValue * 2, pixelValue * 3, pixelValue * 4);
                break;
            }

            case wgpu::TextureFormat::RGBA8Snorm:
            case wgpu::TextureFormat::RGBA8Sint: {
                int8_t* valuePtr = static_cast<int8_t*>(pixelValuePtr);
                valuePtr[0] = static_cast<int8_t>(pixelValue);
                valuePtr[1] = -static_cast<int8_t>(pixelValue);
                valuePtr[2] = static_cast<int8_t>(pixelValue) * 2;
                valuePtr[3] = -static_cast<int8_t>(pixelValue) * 2;
                break;
            }

            default:
                UNREACHABLE();
                break;
        }
    }

    std::string GetImageDeclaration(wgpu::TextureFormat format,
                                    std::string accessQualifier,
                                    wgpu::TextureViewDimension dimension,
                                    uint32_t binding) {
        std::ostringstream ostream;
        ostream << "@group(0) @binding(" << binding << ") "
                << "var storageImage" << binding << " : ";
        switch (dimension) {
            case wgpu::TextureViewDimension::e1D:
                ostream << "texture_storage_1d";
                break;
            case wgpu::TextureViewDimension::e2D:
                ostream << "texture_storage_2d";
                break;
            case wgpu::TextureViewDimension::e2DArray:
                ostream << "texture_storage_2d_array";
                break;
            case wgpu::TextureViewDimension::e3D:
                ostream << "texture_storage_3d";
                break;
            default:
                UNREACHABLE();
                break;
        }
        ostream << "<" << utils::GetWGSLImageFormatQualifier(format) << ", ";
        ostream << accessQualifier << ">;";
        return ostream.str();
    }

    const char* GetExpectedPixelValue(wgpu::TextureFormat format) {
        switch (format) {
            // non-normalized unsigned integer formats
            case wgpu::TextureFormat::R32Uint:
                return "vec4<u32>(u32(value), 0u, 0u, 1u)";

            case wgpu::TextureFormat::RG32Uint:
                return "vec4<u32>(u32(value), u32(value) * 2u, 0u, 1u)";

            case wgpu::TextureFormat::RGBA8Uint:
            case wgpu::TextureFormat::RGBA16Uint:
            case wgpu::TextureFormat::RGBA32Uint:
                return "vec4<u32>(u32(value), u32(value) * 2u, "
                       "u32(value) * 3u, u32(value) * 4u)";

            // non-normalized signed integer formats
            case wgpu::TextureFormat::R32Sint:
                return "vec4<i32>(i32(value), 0, 0, 1)";

            case wgpu::TextureFormat::RG32Sint:
                return "vec4<i32>(i32(value), -i32(value), 0, 1)";

            case wgpu::TextureFormat::RGBA8Sint:
            case wgpu::TextureFormat::RGBA16Sint:
            case wgpu::TextureFormat::RGBA32Sint:
                return "vec4<i32>(i32(value), -i32(value), i32(value) * 2, -i32(value) * 2)";

            // float formats
            case wgpu::TextureFormat::R32Float:
                return "vec4<f32>(f32(value) * 1.1, 0.0, 0.0, 1.0)";

            case wgpu::TextureFormat::RG32Float:
                return "vec4<f32>(f32(value) * 1.1, -f32(value) * 2.2, 0.0, 1.0)";

            case wgpu::TextureFormat::RGBA16Float:
                return "vec4<f32>(f32(value), -f32(value), "
                       "f32(value) * 2.0, -f32(value) * 2.0)";

            case wgpu::TextureFormat::RGBA32Float:
                return "vec4<f32>(f32(value) * 1.1, -f32(value) * 1.1, "
                       "f32(value) * 2.2, -f32(value) * 2.2)";

            // normalized signed/unsigned integer formats
            case wgpu::TextureFormat::RGBA8Unorm:
                return "vec4<f32>(f32(value) / 255.0, f32(value) / 255.0 * 2.0, "
                       "f32(value) / 255.0 * 3.0, f32(value) / 255.0 * 4.0)";

            case wgpu::TextureFormat::RGBA8Snorm:
                return "vec4<f32>(f32(value) / 127.0, -f32(value) / 127.0, "
                       "f32(value) * 2.0 / 127.0, -f32(value) * 2.0 / 127.0)";

            default:
                UNREACHABLE();
                break;
        }
    }

    const char* GetComparisonFunction(wgpu::TextureFormat format) {
        switch (format) {
            // non-normalized unsigned integer formats
            case wgpu::TextureFormat::R32Uint:
            case wgpu::TextureFormat::RG32Uint:
            case wgpu::TextureFormat::RGBA8Uint:
            case wgpu::TextureFormat::RGBA16Uint:
            case wgpu::TextureFormat::RGBA32Uint:
                return R"(
fn IsEqualTo(pixel : vec4<u32>, expected : vec4<u32>) -> bool {
  return all(pixel == expected);
})";

            // non-normalized signed integer formats
            case wgpu::TextureFormat::R32Sint:
            case wgpu::TextureFormat::RG32Sint:
            case wgpu::TextureFormat::RGBA8Sint:
            case wgpu::TextureFormat::RGBA16Sint:
            case wgpu::TextureFormat::RGBA32Sint:
                return R"(
fn IsEqualTo(pixel : vec4<i32>, expected : vec4<i32>) -> bool {
  return all(pixel == expected);
})";

            // float formats
            case wgpu::TextureFormat::R32Float:
            case wgpu::TextureFormat::RG32Float:
            case wgpu::TextureFormat::RGBA16Float:
            case wgpu::TextureFormat::RGBA32Float:
                return R"(
fn IsEqualTo(pixel : vec4<f32>, expected : vec4<f32>) -> bool {
  return all(pixel == expected);
})";

            // normalized signed/unsigned integer formats
            case wgpu::TextureFormat::RGBA8Unorm:
            case wgpu::TextureFormat::RGBA8Snorm:
                // On Windows Intel drivers the tests will fail if tolerance <= 0.00000001f.
                return R"(
fn IsEqualTo(pixel : vec4<f32>, expected : vec4<f32>) -> bool {
  let tolerance : f32 = 0.0000001;
  return all(abs(pixel - expected) < vec4<f32>(tolerance, tolerance, tolerance, tolerance));
})";

            default:
                UNREACHABLE();
                break;
        }

        return "";
    }

    std::string CommonWriteOnlyTestCode(
        const char* stage,
        wgpu::TextureFormat format,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        std::string componentFmt = utils::GetWGSLColorTextureComponentType(format);
        auto texelType = "vec4<" + componentFmt + ">";
        std::string sliceCount;
        std::string textureStore;
        std::string textureSize = "textureDimensions(storageImage0).xy";
        switch (dimension) {
            case wgpu::TextureViewDimension::e1D:
                sliceCount = "1";
                textureStore = "textureStore(storageImage0, x, expected)";
                textureSize = "vec2<i32>(textureDimensions(storageImage0), 1)";
                break;
            case wgpu::TextureViewDimension::e2D:
                sliceCount = "1";
                textureStore = "textureStore(storageImage0, vec2<i32>(x, y), expected)";
                break;
            case wgpu::TextureViewDimension::e2DArray:
                sliceCount = "textureNumLayers(storageImage0)";
                textureStore = "textureStore(storageImage0, vec2<i32>(x, y), slice, expected)";
                break;
            case wgpu::TextureViewDimension::e3D:
                sliceCount = "textureDimensions(storageImage0).z";
                textureStore = "textureStore(storageImage0, vec3<i32>(x, y, slice), expected)";
                break;
            default:
                UNREACHABLE();
                break;
        }
        const char* workgroupSize = !strcmp(stage, "compute") ? " @workgroup_size(1)" : "";
        const bool isFragment = strcmp(stage, "fragment") == 0;

        std::ostringstream ostream;
        ostream << GetImageDeclaration(format, "write", dimension, 0) << "\n";
        ostream << "@stage(" << stage << ")" << workgroupSize << "\n";
        ostream << "fn main() ";
        if (isFragment) {
            ostream << "-> @location(0) vec4<f32> ";
        }
        ostream << "{\n";
        ostream << "  let size : vec2<i32> = " << textureSize << ";\n";
        ostream << "  let sliceCount : i32 = " << sliceCount << ";\n";
        ostream << "  for (var slice : i32 = 0; slice < sliceCount; slice = slice + 1) {\n";
        ostream << "    for (var y : i32 = 0; y < size.y; y = y + 1) {\n";
        ostream << "      for (var x : i32 = 0; x < size.x; x = x + 1) {\n";
        ostream << "        var value : i32 = " << kComputeExpectedValue << ";\n";
        ostream << "        var expected : " << texelType << " = " << GetExpectedPixelValue(format)
                << ";\n";
        ostream << "        " << textureStore << ";\n";
        ostream << "      }\n";
        ostream << "    }\n";
        ostream << "  }\n";
        if (isFragment) {
            ostream << "return vec4<f32>();\n";
        }
        ostream << "}\n";

        return ostream.str();
    }

    static std::vector<uint8_t> GetExpectedData(wgpu::TextureFormat format,
                                                uint32_t sliceCount = 1) {
        const uint32_t texelSizeInBytes = utils::GetTexelBlockSizeInBytes(format);

        std::vector<uint8_t> outputData(texelSizeInBytes * kWidth * kHeight * sliceCount);

        for (uint32_t i = 0; i < outputData.size() / texelSizeInBytes; ++i) {
            uint8_t* pixelValuePtr = &outputData[i * texelSizeInBytes];
            const uint32_t x = i % kWidth;
            const uint32_t y = (i % (kWidth * kHeight)) / kWidth;
            const uint32_t slice = i / (kWidth * kHeight);
            FillExpectedData(pixelValuePtr, format, x, y, slice);
        }

        return outputData;
    }

    wgpu::Texture CreateTexture(wgpu::TextureFormat format,
                                wgpu::TextureUsage usage,
                                const wgpu::Extent3D& size,
                                wgpu::TextureDimension dimension = wgpu::TextureDimension::e2D) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = size;
        descriptor.dimension = dimension;
        descriptor.format = format;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }

    wgpu::Texture CreateTextureWithTestData(
        const std::vector<uint8_t>& initialTextureData,
        wgpu::TextureFormat format,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        uint32_t texelSize = utils::GetTexelBlockSizeInBytes(format);
        ASSERT(kWidth * texelSize <= kTextureBytesPerRowAlignment);

        const uint32_t bytesPerTextureRow = texelSize * kWidth;
        const uint32_t sliceCount =
            static_cast<uint32_t>(initialTextureData.size() / texelSize / (kWidth * kHeight));
        const size_t uploadBufferSize =
            kTextureBytesPerRowAlignment * (kHeight * sliceCount - 1) + kWidth * bytesPerTextureRow;

        std::vector<uint8_t> uploadBufferData(uploadBufferSize);
        for (uint32_t slice = 0; slice < sliceCount; ++slice) {
            const size_t initialDataOffset = bytesPerTextureRow * kHeight * slice;
            for (size_t y = 0; y < kHeight; ++y) {
                for (size_t x = 0; x < bytesPerTextureRow; ++x) {
                    uint8_t data =
                        initialTextureData[initialDataOffset + bytesPerTextureRow * y + x];
                    size_t indexInUploadBuffer =
                        (kHeight * slice + y) * kTextureBytesPerRowAlignment + x;
                    uploadBufferData[indexInUploadBuffer] = data;
                }
            }
        }
        wgpu::Buffer uploadBuffer =
            utils::CreateBufferFromData(device, uploadBufferData.data(), uploadBufferSize,
                                        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

        wgpu::Texture outputTexture = CreateTexture(
            format, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopyDst,
            {kWidth, kHeight, sliceCount}, utils::ViewDimensionToTextureDimension(dimension));

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        const wgpu::Extent3D copyExtent = {kWidth, kHeight, sliceCount};
        wgpu::ImageCopyBuffer imageCopyBuffer =
            utils::CreateImageCopyBuffer(uploadBuffer, 0, kTextureBytesPerRowAlignment, kHeight);
        wgpu::ImageCopyTexture imageCopyTexture;
        imageCopyTexture.texture = outputTexture;
        encoder.CopyBufferToTexture(&imageCopyBuffer, &imageCopyTexture, &copyExtent);

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        return outputTexture;
    }

    wgpu::ComputePipeline CreateComputePipeline(const char* computeShader) {
        wgpu::ShaderModule csModule = utils::CreateShaderModule(device, computeShader);
        wgpu::ComputePipelineDescriptor computeDescriptor;
        computeDescriptor.layout = nullptr;
        computeDescriptor.compute.module = csModule;
        computeDescriptor.compute.entryPoint = "main";
        return device.CreateComputePipeline(&computeDescriptor);
    }

    wgpu::RenderPipeline CreateRenderPipeline(const char* vertexShader,
                                              const char* fragmentShader) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModule(device, vertexShader);
        wgpu::ShaderModule fsModule = utils::CreateShaderModule(device, fragmentShader);

        utils::ComboRenderPipelineDescriptor desc;
        desc.vertex.module = vsModule;
        desc.cFragment.module = fsModule;
        desc.cTargets[0].format = kRenderAttachmentFormat;
        desc.primitive.topology = wgpu::PrimitiveTopology::PointList;
        return device.CreateRenderPipeline(&desc);
    }

    void CheckDrawsGreen(const char* vertexShader,
                         const char* fragmentShader,
                         wgpu::Texture readonlyStorageTexture) {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(vertexShader, fragmentShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, readonlyStorageTexture.CreateView()}});

        // Clear the render attachment to red at the beginning of the render pass.
        wgpu::Texture outputTexture = CreateTexture(
            kRenderAttachmentFormat,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, {1, 1});
        utils::ComboRenderPassDescriptor renderPassDescriptor({outputTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].loadOp = wgpu::LoadOp::Clear;
        renderPassDescriptor.cColorAttachments[0].clearValue = {1.f, 0.f, 0.f, 1.f};
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.Draw(1);
        renderPassEncoder.End();

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the output texture are all as expected (green).
        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, outputTexture, 0, 0)
            << "\nVertex Shader:\n"
            << vertexShader << "\n\nFragment Shader:\n"
            << fragmentShader;
    }

    void CheckResultInStorageBuffer(
        wgpu::Texture readonlyStorageTexture,
        const std::string& computeShader,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader.c_str());

        // Clear the content of the result buffer into 0.
        constexpr uint32_t kInitialValue = 0;
        wgpu::Buffer resultBuffer =
            utils::CreateBufferFromData(device, &kInitialValue, sizeof(kInitialValue),
                                        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = dimension;
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0),
            {{0, readonlyStorageTexture.CreateView(&descriptor)}, {1, resultBuffer}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computeEncoder = encoder.BeginComputePass();
        computeEncoder.SetBindGroup(0, bindGroup);
        computeEncoder.SetPipeline(pipeline);
        computeEncoder.DispatchWorkgroups(1);
        computeEncoder.End();

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the result buffer are what we expect.
        constexpr uint32_t kExpectedValue = 1u;
        EXPECT_BUFFER_U32_RANGE_EQ(&kExpectedValue, resultBuffer, 0, 1u);
    }

    void WriteIntoStorageTextureInRenderPass(wgpu::Texture writeonlyStorageTexture,
                                             const char* vertexShader,
                                             const char* fragmentShader) {
        // Create a render pipeline that writes the expected pixel values into the storage texture
        // without fragment shader outputs.
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(vertexShader, fragmentShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, writeonlyStorageTexture.CreateView()}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        wgpu::Texture placeholderOutputTexture = CreateTexture(
            kRenderAttachmentFormat,
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, {1, 1});
        utils::ComboRenderPassDescriptor renderPassDescriptor(
            {placeholderOutputTexture.CreateView()});
        wgpu::RenderPassEncoder renderPassEncoder = encoder.BeginRenderPass(&renderPassDescriptor);
        renderPassEncoder.SetBindGroup(0, bindGroup);
        renderPassEncoder.SetPipeline(pipeline);
        renderPassEncoder.Draw(1);
        renderPassEncoder.End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void WriteIntoStorageTextureInComputePass(
        wgpu::Texture writeonlyStorageTexture,
        const char* computeShader,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        // Create a compute pipeline that writes the expected pixel values into the storage texture.
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = dimension;
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, writeonlyStorageTexture.CreateView(&descriptor)}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup);
        computePassEncoder.SetPipeline(pipeline);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void ReadWriteIntoStorageTextureInComputePass(
        wgpu::Texture readonlyStorageTexture,
        wgpu::Texture writeonlyStorageTexture,
        const char* computeShader,
        wgpu::TextureViewDimension dimension = wgpu::TextureViewDimension::e2D) {
        // Create a compute pipeline that writes the expected pixel values into the storage texture.
        wgpu::TextureViewDescriptor descriptor;
        descriptor.dimension = dimension;
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader);
        wgpu::BindGroup bindGroup =
            utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                 {{0, writeonlyStorageTexture.CreateView(&descriptor)},
                                  {1, readonlyStorageTexture.CreateView(&descriptor)}});

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::ComputePassEncoder computePassEncoder = encoder.BeginComputePass();
        computePassEncoder.SetBindGroup(0, bindGroup);
        computePassEncoder.SetPipeline(pipeline);
        computePassEncoder.DispatchWorkgroups(1);
        computePassEncoder.End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);
    }

    void CheckOutputStorageTexture(wgpu::Texture writeonlyStorageTexture,
                                   wgpu::TextureFormat format,
                                   const wgpu::Extent3D& size) {
        const std::vector<uint8_t>& expectedData = GetExpectedData(format, size.depthOrArrayLayers);
        CheckOutputStorageTexture(writeonlyStorageTexture, format, size, expectedData);
    }

    void CheckOutputStorageTexture(wgpu::Texture writeonlyStorageTexture,
                                   wgpu::TextureFormat format,
                                   const wgpu::Extent3D& size,
                                   const std::vector<uint8_t>& expectedData) {
        // Copy the content from the write-only storage texture to the result buffer.
        wgpu::BufferDescriptor descriptor;
        descriptor.size =
            utils::RequiredBytesInCopy(kTextureBytesPerRowAlignment, size.height, size, format);
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        wgpu::Buffer resultBuffer = device.CreateBuffer(&descriptor);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::ImageCopyTexture imageCopyTexture =
                utils::CreateImageCopyTexture(writeonlyStorageTexture, 0, {0, 0, 0});
            wgpu::ImageCopyBuffer imageCopyBuffer = utils::CreateImageCopyBuffer(
                resultBuffer, 0, kTextureBytesPerRowAlignment, size.height);
            encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &size);
        }
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the result buffer are what we expect.
        uint32_t texelSize = utils::GetTexelBlockSizeInBytes(format);
        ASSERT(size.width * texelSize <= kTextureBytesPerRowAlignment);

        for (size_t z = 0; z < size.depthOrArrayLayers; ++z) {
            for (size_t y = 0; y < size.height; ++y) {
                const size_t resultBufferOffset =
                    kTextureBytesPerRowAlignment * (size.height * z + y);
                const size_t expectedDataOffset = texelSize * size.width * (size.height * z + y);
                EXPECT_BUFFER_U32_RANGE_EQ(
                    reinterpret_cast<const uint32_t*>(expectedData.data() + expectedDataOffset),
                    resultBuffer, resultBufferOffset, texelSize);
            }
        }
    }

    static constexpr size_t kWidth = 4u;
    static constexpr size_t kHeight = 4u;
    static constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

    const char* kSimpleVertexShader = R"(
;
@stage(vertex) fn main() -> @builtin(position) vec4<f32> {
  return vec4<f32>(0.0, 0.0, 0.0, 1.0);
})";

    const char* kComputeExpectedValue = "1 + x + size.x * (y + size.y * slice)";
};

// Test that write-only storage textures are supported in compute shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInComputeShader) {
    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (IsOpenGLES() && !OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        if (format == wgpu::TextureFormat::RGBA8Snorm && HasToggleEnabled("disable_snorm_read")) {
            continue;
        }

        // TODO(crbug.com/dawn/676): investigate why this test fails with RGBA8Snorm on Linux
        // Intel OpenGL and OpenGLES drivers.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsIntel() &&
            (IsOpenGL() || IsOpenGLES()) && IsLinux()) {
            continue;
        }

        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                          {kWidth, kHeight});

        // Write the expected pixel values into the write-only storage texture.
        const std::string computeShader = CommonWriteOnlyTestCode("compute", format);
        WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format, {kWidth, kHeight});
    }
}

// Test that write-only storage textures are supported in fragment shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInFragmentShader) {
    // TODO(crbug.com/dawn/672): Investigate why this test fails on Linux
    // NVidia OpenGLES drivers.
    DAWN_SUPPRESS_TEST_IF(IsNvidia() && IsLinux() && IsOpenGLES());

    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (IsOpenGLES() && !OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        if (format == wgpu::TextureFormat::RGBA8Snorm && HasToggleEnabled("disable_snorm_read")) {
            continue;
        }

        // TODO(crbug.com/dawn/676): investigate why this test fails with RGBA8Snorm on Linux
        // Intel OpenGL and OpenGLES drivers.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsIntel() &&
            (IsOpenGL() || IsOpenGLES()) && IsLinux()) {
            continue;
        }

        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
                          {kWidth, kHeight});

        // Write the expected pixel values into the write-only storage texture.
        const std::string fragmentShader = CommonWriteOnlyTestCode("fragment", format);
        WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                            fragmentShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format, {kWidth, kHeight});
    }
}

// Verify 2D array and 3D write-only storage textures work correctly.
TEST_P(StorageTextureTests, Writeonly2DArrayOr3DStorageTexture) {
    // TODO(crbug.com/dawn/547): implement 3D storage texture on OpenGL and OpenGLES.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL() || IsOpenGLES());

    constexpr uint32_t kSliceCount = 3u;

    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

    wgpu::TextureViewDimension dimensions[] = {
        wgpu::TextureViewDimension::e2DArray,
        wgpu::TextureViewDimension::e3D,
    };

    // Prepare the write-only storage texture.
    for (wgpu::TextureViewDimension dimension : dimensions) {
        wgpu::Texture writeonlyStorageTexture = CreateTexture(
            kTextureFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
            {kWidth, kHeight, kSliceCount}, utils::ViewDimensionToTextureDimension(dimension));

        // Write the expected pixel values into the write-only storage texture.
        const std::string computeShader =
            CommonWriteOnlyTestCode("compute", kTextureFormat, dimension);
        WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str(),
                                             dimension);

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, kTextureFormat,
                                  {kWidth, kHeight, kSliceCount});
    }
}

// Verify 1D write-only storage textures work correctly.
TEST_P(StorageTextureTests, Writeonly1DStorageTexture) {
    // TODO(crbug.com/dawn/547): implement 1D storage texture on OpenGL and OpenGLES.
    DAWN_TEST_UNSUPPORTED_IF(IsOpenGL() || IsOpenGLES());

    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        kTextureFormat, wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc,
        {kWidth, 1, 1}, wgpu::TextureDimension::e1D);

    // Write the expected pixel values into the write-only storage texture.
    const std::string computeShader =
        CommonWriteOnlyTestCode("compute", kTextureFormat, wgpu::TextureViewDimension::e1D);
    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str(),
                                         wgpu::TextureViewDimension::e1D);

    // Verify the pixel data in the write-only storage texture is expected.
    CheckOutputStorageTexture(writeonlyStorageTexture, kTextureFormat, {kWidth, 1, 1});
}

// Test that multiple dispatches to increment values by ping-ponging between a sampled texture and
// a write-only storage texture are synchronized in one pass.
TEST_P(StorageTextureTests, SampledAndWriteonlyStorageTexturePingPong) {
    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;
    wgpu::Texture storageTexture1 =
        CreateTexture(kTextureFormat,
                      wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding |
                          wgpu::TextureUsage::CopySrc,
                      {1u, 1u});
    wgpu::Texture storageTexture2 = CreateTexture(
        kTextureFormat, wgpu::TextureUsage::TextureBinding | wgpu::TextureUsage::StorageBinding,
        {1u, 1u});
    wgpu::ShaderModule module = utils::CreateShaderModule(device, R"(
@group(0) @binding(0) var Src : texture_2d<u32>;
@group(0) @binding(1) var Dst : texture_storage_2d<r32uint, write>;
@stage(compute) @workgroup_size(1) fn main() {
  var srcValue : vec4<u32> = textureLoad(Src, vec2<i32>(0, 0), 0);
  srcValue.x = srcValue.x + 1u;
  textureStore(Dst, vec2<i32>(0, 0), srcValue);
}
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.compute.module = module;
    pipelineDesc.compute.entryPoint = "main";
    wgpu::ComputePipeline pipeline = device.CreateComputePipeline(&pipelineDesc);

    // In bindGroupA storageTexture1 is bound as read-only storage texture and storageTexture2 is
    // bound as write-only storage texture.
    wgpu::BindGroup bindGroupA = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, storageTexture1.CreateView()},
                                                          {1, storageTexture2.CreateView()},
                                                      });

    // In bindGroupA storageTexture2 is bound as read-only storage texture and storageTexture1 is
    // bound as write-only storage texture.
    wgpu::BindGroup bindGroupB = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                                      {
                                                          {0, storageTexture2.CreateView()},
                                                          {1, storageTexture1.CreateView()},
                                                      });

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    wgpu::ComputePassEncoder pass = encoder.BeginComputePass();
    pass.SetPipeline(pipeline);

    // After the first dispatch the value in storageTexture2 should be 1u.
    pass.SetBindGroup(0, bindGroupA);
    pass.DispatchWorkgroups(1);

    // After the second dispatch the value in storageTexture1 should be 2u;
    pass.SetBindGroup(0, bindGroupB);
    pass.DispatchWorkgroups(1);

    pass.End();

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(uint32_t);
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::ImageCopyTexture imageCopyTexture;
    imageCopyTexture.texture = storageTexture1;

    wgpu::ImageCopyBuffer imageCopyBuffer = utils::CreateImageCopyBuffer(resultBuffer, 0, 256, 1);
    wgpu::Extent3D extent3D = {1, 1, 1};
    encoder.CopyTextureToBuffer(&imageCopyTexture, &imageCopyBuffer, &extent3D);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    constexpr uint32_t kFinalPixelValueInTexture1 = 2u;
    EXPECT_BUFFER_U32_EQ(kFinalPixelValueInTexture1, resultBuffer, 0);
}

DAWN_INSTANTIATE_TEST(StorageTextureTests,
                      D3D12Backend(),
                      MetalBackend(),
                      OpenGLBackend(),
                      OpenGLESBackend(),
                      VulkanBackend());

class StorageTextureZeroInitTests : public StorageTextureTests {
  public:
    static std::vector<uint8_t> GetExpectedData() {
        constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

        const uint32_t texelSizeInBytes = utils::GetTexelBlockSizeInBytes(kTextureFormat);
        const size_t kDataCount = texelSizeInBytes * kWidth * kHeight;
        std::vector<uint8_t> outputData(kDataCount, 0);

        uint32_t* outputDataPtr = reinterpret_cast<uint32_t*>(&outputData[0]);
        *outputDataPtr = 1u;

        return outputData;
    }

    const char* kCommonReadOnlyZeroInitTestCode = R"(
fn doTest() -> bool {
  for (var y : i32 = 0; y < 4; y = y + 1) {
    for (var x : i32 = 0; x < 4; x = x + 1) {
      var pixel : vec4<u32> = textureLoad(srcImage, vec2<i32>(x, y));
      if (any(pixel != vec4<u32>(0u, 0u, 0u, 1u))) {
        return false;
      }
    }
  }
  return true;
})";

    const char* kCommonWriteOnlyZeroInitTestCodeFragment = R"(
@group(0) @binding(0) var dstImage : texture_storage_2d<r32uint, write>;

@stage(fragment) fn main() -> @location(0) vec4<f32> {
  textureStore(dstImage, vec2<i32>(0, 0), vec4<u32>(1u, 0u, 0u, 1u));
  return vec4<f32>();
})";
    const char* kCommonWriteOnlyZeroInitTestCodeCompute = R"(
@group(0) @binding(0) var dstImage : texture_storage_2d<r32uint, write>;

@stage(compute) @workgroup_size(1) fn main() {
  textureStore(dstImage, vec2<i32>(0, 0), vec4<u32>(1u, 0u, 0u, 1u));
})";
};

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// storage texture in a render pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInRenderPass) {
    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint,
        wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc, {kWidth, kHeight});

    WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                        kCommonWriteOnlyZeroInitTestCodeFragment);
    CheckOutputStorageTexture(writeonlyStorageTexture, wgpu::TextureFormat::R32Uint,
                              {kWidth, kHeight}, GetExpectedData());
}

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// texture in a compute pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInComputePass) {
    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint,
        wgpu::TextureUsage::StorageBinding | wgpu::TextureUsage::CopySrc, {kWidth, kHeight});

    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture,
                                         kCommonWriteOnlyZeroInitTestCodeCompute);
    CheckOutputStorageTexture(writeonlyStorageTexture, wgpu::TextureFormat::R32Uint,
                              {kWidth, kHeight}, GetExpectedData());
}

DAWN_INSTANTIATE_TEST(StorageTextureZeroInitTests,
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"}));
