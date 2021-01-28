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
#include "common/Math.h"
#include "utils/ComboRenderPipelineDescriptor.h"
#include "utils/TextureFormatUtils.h"
#include "utils/WGPUHelpers.h"

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
                                 uint32_t arrayLayer) {
        const uint32_t pixelValue = 1 + x + kWidth * (y + kHeight * arrayLayer);
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
                                    bool is2DArray,
                                    uint32_t binding) {
        std::ostringstream ostream;
        ostream << "[[group(0), binding(" << binding << ")]] "
                << "var<uniform_constant> storageImage" << binding << " : "
                << "[[access(" << accessQualifier << ")]] "
                << "texture_storage_2d";
        if (is2DArray) {
            ostream << "_array";
        }
        ostream << "<" << utils::GetWGSLImageFormatQualifier(format) << ">;";
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
  const tolerance : f32 = 0.0000001;
  return all(abs(pixel - expected) < vec4<f32>(tolerance, tolerance, tolerance, tolerance));
})";

            default:
                UNREACHABLE();
                break;
        }

        return "";
    }

    std::string CommonReadOnlyTestCode(wgpu::TextureFormat format, bool is2DArray = false) {
        std::string componentFmt = utils::GetWGSLColorTextureComponentType(format);
        auto texelType = "vec4<" + componentFmt + ">";
        auto* layerCount = is2DArray ? "textureNumLayers(storageImage0)" : "1";
        auto* textureLoad = is2DArray ? "textureLoad(storageImage0, vec2<i32>(x, y), i32(layer))"
                                      : "textureLoad(storageImage0, vec2<i32>(x, y))";

        std::ostringstream ostream;
        ostream << GetImageDeclaration(format, "read", is2DArray, 0) << "\n"
                << GetComparisonFunction(format) << "\n";
        ostream << "fn doTest() -> bool {\n";
        ostream << "  var size : vec2<i32> = textureDimensions(storageImage0);\n";
        ostream << "  const layerCount : i32 = " << layerCount << ";\n";
        ostream << "  for (var layer : i32 = 0; layer < layerCount; layer = layer + 1) {\n";
        ostream << "    for (var y : i32 = 0; y < size.y; y = y + 1) {\n";
        ostream << "      for (var x : i32 = 0; x < size.x; x = x + 1) {\n";
        ostream << "        var value : i32 = " << kComputeExpectedValue << ";\n";
        ostream << "        var expected : " << texelType << " = " << GetExpectedPixelValue(format)
                << ";\n";
        ostream << "        var pixel : " << texelType << " = " << textureLoad << ";\n";
        ostream << "        if (!IsEqualTo(pixel, expected)) {\n";
        ostream << "          return false;\n";
        ostream << "        }\n";
        ostream << "      }\n";
        ostream << "    }\n";
        ostream << "  }\n";
        ostream << "  return true;\n";
        ostream << "}\n";

        return ostream.str();
    }

    std::string CommonWriteOnlyTestCode(const char* stage,
                                        wgpu::TextureFormat format,
                                        bool is2DArray = false) {
        std::string componentFmt = utils::GetWGSLColorTextureComponentType(format);
        auto texelType = "vec4<" + componentFmt + ">";
        auto* layerCount = is2DArray ? "textureNumLayers(storageImage0)" : "1";
        auto* textureStore = is2DArray
                                 ? "textureStore(storageImage0, vec2<i32>(x, y), layer, expected)"
                                 : "textureStore(storageImage0, vec2<i32>(x, y), expected)";

        std::ostringstream ostream;
        ostream << GetImageDeclaration(format, "write", is2DArray, 0) << "\n";
        ostream << "[[stage(" << stage << ")]]\n";
        ostream << "fn main() -> void {\n";
        ostream << "  var size : vec2<i32> = textureDimensions(storageImage0);\n";
        ostream << "  const layerCount : i32 = " << layerCount << ";\n";
        ostream << "  for (var layer : i32 = 0; layer < layerCount; layer = layer + 1) {\n";
        ostream << "    for (var y : i32 = 0; y < size.y; y = y + 1) {\n";
        ostream << "      for (var x : i32 = 0; x < size.x; x = x + 1) {\n";
        ostream << "        var value : i32 = " << kComputeExpectedValue << ";\n";
        ostream << "        var expected : " << texelType << " = " << GetExpectedPixelValue(format)
                << ";\n";
        ostream << "        " << textureStore << ";\n";
        ostream << "      }\n";
        ostream << "    }\n";
        ostream << "  }\n";
        ostream << "}\n";

        return ostream.str();
    }

    std::string CommonReadWriteTestCode(wgpu::TextureFormat format, bool is2DArray = false) {
        auto* layerCount = is2DArray ? "textureNumLayers(storageImage0)" : "1";
        auto* textureStore = is2DArray ? "textureStore(storageImage0, texcoord, layer, "
                                         "textureLoad(storageImage1, texcoord, layer))"
                                       : "textureStore(storageImage0, texcoord, "
                                         "textureLoad(storageImage1, texcoord))";

        std::ostringstream ostream;
        ostream << GetImageDeclaration(format, "write", is2DArray, 0) << "\n";
        ostream << GetImageDeclaration(format, "read", is2DArray, 1) << "\n";
        ostream << "[[stage(compute)]] fn main() -> void {\n";
        ostream << "  var size : vec2<i32> = textureDimensions(storageImage0);\n";
        ostream << "  const layerCount : i32 = " << layerCount << ";\n";
        ostream << "  for (var layer : i32 = 0; layer < layerCount; layer = layer + 1) {\n";
        ostream << "    for (var y : i32 = 0; y < size.y; y = y + 1) {\n";
        ostream << "      for (var x : i32 = 0; x < size.x; x = x + 1) {\n";
        ostream << "        var texcoord : vec2<i32> = vec2<i32>(x, y);\n";
        ostream << "        " << textureStore << ";\n";
        ostream << "      }\n";
        ostream << "    }\n";
        ostream << "  }\n";
        ostream << "}\n";
        return ostream.str();
    }

    static std::vector<uint8_t> GetExpectedData(wgpu::TextureFormat format,
                                                uint32_t arrayLayerCount = 1) {
        const uint32_t texelSizeInBytes = utils::GetTexelBlockSizeInBytes(format);

        std::vector<uint8_t> outputData(texelSizeInBytes * kWidth * kHeight * arrayLayerCount);

        for (uint32_t i = 0; i < outputData.size() / texelSizeInBytes; ++i) {
            uint8_t* pixelValuePtr = &outputData[i * texelSizeInBytes];
            const uint32_t x = i % kWidth;
            const uint32_t y = (i % (kWidth * kHeight)) / kWidth;
            const uint32_t arrayLayer = i / (kWidth * kHeight);
            FillExpectedData(pixelValuePtr, format, x, y, arrayLayer);
        }

        return outputData;
    }

    wgpu::Texture CreateTexture(wgpu::TextureFormat format,
                                wgpu::TextureUsage usage,
                                uint32_t width = kWidth,
                                uint32_t height = kHeight,
                                uint32_t arrayLayerCount = 1) {
        wgpu::TextureDescriptor descriptor;
        descriptor.size = {width, height, arrayLayerCount};
        descriptor.format = format;
        descriptor.usage = usage;
        return device.CreateTexture(&descriptor);
    }

    wgpu::Buffer CreateEmptyBufferForTextureCopy(uint32_t texelSize, uint32_t arrayLayerCount = 1) {
        ASSERT(kWidth * texelSize <= kTextureBytesPerRowAlignment);
        const size_t uploadBufferSize =
            kTextureBytesPerRowAlignment * (kHeight * arrayLayerCount - 1) + kWidth * texelSize;
        wgpu::BufferDescriptor descriptor;
        descriptor.size = uploadBufferSize;
        descriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
        return device.CreateBuffer(&descriptor);
    }

    wgpu::Texture CreateTextureWithTestData(const std::vector<uint8_t>& initialTextureData,
                                            wgpu::TextureFormat format) {
        uint32_t texelSize = utils::GetTexelBlockSizeInBytes(format);
        ASSERT(kWidth * texelSize <= kTextureBytesPerRowAlignment);

        const uint32_t bytesPerTextureRow = texelSize * kWidth;
        const uint32_t arrayLayerCount =
            static_cast<uint32_t>(initialTextureData.size() / texelSize / (kWidth * kHeight));
        const size_t uploadBufferSize =
            kTextureBytesPerRowAlignment * (kHeight * arrayLayerCount - 1) +
            kWidth * bytesPerTextureRow;

        std::vector<uint8_t> uploadBufferData(uploadBufferSize);
        for (uint32_t layer = 0; layer < arrayLayerCount; ++layer) {
            const size_t initialDataOffset = bytesPerTextureRow * kHeight * layer;
            for (size_t y = 0; y < kHeight; ++y) {
                for (size_t x = 0; x < bytesPerTextureRow; ++x) {
                    uint8_t data =
                        initialTextureData[initialDataOffset + bytesPerTextureRow * y + x];
                    size_t indexInUploadBuffer =
                        (kHeight * layer + y) * kTextureBytesPerRowAlignment + x;
                    uploadBufferData[indexInUploadBuffer] = data;
                }
            }
        }
        wgpu::Buffer uploadBuffer =
            utils::CreateBufferFromData(device, uploadBufferData.data(), uploadBufferSize,
                                        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst);

        wgpu::Texture outputTexture =
            CreateTexture(format, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopyDst, kWidth,
                          kHeight, arrayLayerCount);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        const wgpu::Extent3D copyExtent = {kWidth, kHeight, arrayLayerCount};
        wgpu::BufferCopyView bufferCopyView =
            utils::CreateBufferCopyView(uploadBuffer, 0, kTextureBytesPerRowAlignment, kHeight);
        wgpu::TextureCopyView textureCopyView;
        textureCopyView.texture = outputTexture;
        encoder.CopyBufferToTexture(&bufferCopyView, &textureCopyView, &copyExtent);

        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        return outputTexture;
    }

    wgpu::ComputePipeline CreateComputePipeline(const char* computeShader) {
        wgpu::ShaderModule csModule = utils::CreateShaderModuleFromWGSL(device, computeShader);
        wgpu::ComputePipelineDescriptor computeDescriptor;
        computeDescriptor.layout = nullptr;
        computeDescriptor.computeStage.module = csModule;
        computeDescriptor.computeStage.entryPoint = "main";
        return device.CreateComputePipeline(&computeDescriptor);
    }

    wgpu::RenderPipeline CreateRenderPipeline(const char* vertexShader,
                                              const char* fragmentShader) {
        wgpu::ShaderModule vsModule = utils::CreateShaderModuleFromWGSL(device, vertexShader);
        wgpu::ShaderModule fsModule = utils::CreateShaderModuleFromWGSL(device, fragmentShader);

        utils::ComboRenderPipelineDescriptor desc(device);
        desc.vertexStage.module = vsModule;
        desc.cFragmentStage.module = fsModule;
        desc.cColorStates[0].format = kRenderAttachmentFormat;
        desc.primitiveTopology = wgpu::PrimitiveTopology::PointList;
        return device.CreateRenderPipeline(&desc);
    }

    void CheckDrawsGreen(const char* vertexShader,
                         const char* fragmentShader,
                         wgpu::Texture readonlyStorageTexture) {
        wgpu::RenderPipeline pipeline = CreateRenderPipeline(vertexShader, fragmentShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0), {{0, readonlyStorageTexture.CreateView()}});

        // Clear the render attachment to red at the beginning of the render pass.
        wgpu::Texture outputTexture =
            CreateTexture(kRenderAttachmentFormat,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, 1, 1);
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
        EXPECT_PIXEL_RGBA8_EQ(RGBA8::kGreen, outputTexture, 0, 0)
            << "\nVertex Shader:\n"
            << vertexShader << "\n\nFragment Shader:\n"
            << fragmentShader;
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

        // TODO(jiawei.shao@intel.com): remove the render attachment when Dawn supports beginning a
        // render pass with no attachments.
        wgpu::Texture dummyOutputTexture =
            CreateTexture(kRenderAttachmentFormat,
                          wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc, 1, 1);
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

    void ReadWriteIntoStorageTextureInComputePass(wgpu::Texture readonlyStorageTexture,
                                                  wgpu::Texture writeonlyStorageTexture,
                                                  const char* computeShader) {
        // Create a compute pipeline that writes the expected pixel values into the storage texture.
        wgpu::ComputePipeline pipeline = CreateComputePipeline(computeShader);
        wgpu::BindGroup bindGroup = utils::MakeBindGroup(
            device, pipeline.GetBindGroupLayout(0),
            {{0, writeonlyStorageTexture.CreateView()}, {1, readonlyStorageTexture.CreateView()}});

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
                                   wgpu::TextureFormat format,
                                   uint32_t arrayLayerCount = 1) {
        const uint32_t texelSize = utils::GetTexelBlockSizeInBytes(format);
        const std::vector<uint8_t>& expectedData = GetExpectedData(format, arrayLayerCount);
        CheckOutputStorageTexture(writeonlyStorageTexture, texelSize, expectedData);
    }

    void CheckOutputStorageTexture(wgpu::Texture writeonlyStorageTexture,
                                   uint32_t texelSize,
                                   const std::vector<uint8_t>& expectedData) {
        // Copy the content from the write-only storage texture to the result buffer.
        const uint32_t arrayLayerCount =
            static_cast<uint32_t>(expectedData.size() / texelSize / (kWidth * kHeight));
        wgpu::Buffer resultBuffer = CreateEmptyBufferForTextureCopy(texelSize, arrayLayerCount);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        const wgpu::Extent3D copyExtent = {kWidth, kHeight, arrayLayerCount};
        wgpu::TextureCopyView textureCopyView =
            utils::CreateTextureCopyView(writeonlyStorageTexture, 0, {0, 0, 0});
        wgpu::BufferCopyView bufferCopyView =
            utils::CreateBufferCopyView(resultBuffer, 0, kTextureBytesPerRowAlignment, kHeight);
        encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &copyExtent);
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        queue.Submit(1, &commandBuffer);

        // Check if the contents in the result buffer are what we expect.
        for (size_t layer = 0; layer < arrayLayerCount; ++layer) {
            for (size_t y = 0; y < kHeight; ++y) {
                const size_t resultBufferOffset =
                    kTextureBytesPerRowAlignment * (kHeight * layer + y);
                const size_t expectedDataOffset = texelSize * kWidth * (kHeight * layer + y);
                EXPECT_BUFFER_U32_RANGE_EQ(
                    reinterpret_cast<const uint32_t*>(expectedData.data() + expectedDataOffset),
                    resultBuffer, resultBufferOffset, kWidth);
            }
        }
    }

    static constexpr size_t kWidth = 4u;
    static constexpr size_t kHeight = 4u;
    static constexpr wgpu::TextureFormat kRenderAttachmentFormat = wgpu::TextureFormat::RGBA8Unorm;

    const char* kSimpleVertexShader = R"(
[[builtin(position)]] var<out> position : vec4<f32>;
[[stage(vertex)]] fn main() -> void {
  position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
})";

    const char* kComputeExpectedValue = "1 + x + size.x * (y + size.y * layer)";
};

// Test that using read-only storage texture and write-only storage texture in BindGroupLayout is
// valid on all backends. This test is a regression test for chromium:1061156 and passes by not
// asserting or crashing.
TEST_P(StorageTextureTests, BindGroupLayoutWithStorageTextureBindingType) {
    // ReadOnly is a valid storage texture binding type to create a bind group
    // layout.
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Compute;
        entry.storageTexture.access = wgpu::StorageTextureAccess::ReadOnly;
        entry.storageTexture.format = wgpu::TextureFormat::R32Float;
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        device.CreateBindGroupLayout(&descriptor);
    }

    // WriteOnly is a valid storage texture binding type to create a bind group
    // layout.
    {
        wgpu::BindGroupLayoutEntry entry;
        entry.binding = 0;
        entry.visibility = wgpu::ShaderStage::Compute;
        entry.storageTexture.access = wgpu::StorageTextureAccess::WriteOnly;
        entry.storageTexture.format = wgpu::TextureFormat::R32Float;
        wgpu::BindGroupLayoutDescriptor descriptor;
        descriptor.entryCount = 1;
        descriptor.entries = &entry;
        device.CreateBindGroupLayout(&descriptor);
    }
}

// Test that read-only storage textures are supported in compute shader.
TEST_P(StorageTextureTests, ReadonlyStorageTextureInComputeShader) {
    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (IsOpenGLES() && !OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        // Prepare the read-only storage texture and fill it with the expected data.
        const std::vector<uint8_t> kInitialTextureData = GetExpectedData(format);
        wgpu::Texture readonlyStorageTexture =
            CreateTextureWithTestData(kInitialTextureData, format);

        // Create a compute shader that reads the pixels from the read-only storage texture and
        // writes 1 to DstBuffer if they all have to expected value.
        std::ostringstream csStream;
        csStream << R"(
[[block]] struct DstBuffer {
  [[offset(0)]] result : u32;
};

[[group(0), binding(1)]] var<storage_buffer> dstBuffer : DstBuffer;
)" << CommonReadOnlyTestCode(format)
                 << R"(
[[stage(compute)]] fn main() -> void {
  if (doTest()) {
    dstBuffer.result = 1u;
  } else {
    dstBuffer.result = 0u;
  }
})";

        CheckResultInStorageBuffer(readonlyStorageTexture, csStream.str());
    }
}

// Test that read-only storage textures are supported in vertex shader.
TEST_P(StorageTextureTests, ReadonlyStorageTextureInVertexShader) {
    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (IsOpenGLES() && !OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        // Prepare the read-only storage texture and fill it with the expected data.
        const std::vector<uint8_t> kInitialTextureData = GetExpectedData(format);
        wgpu::Texture readonlyStorageTexture =
            CreateTextureWithTestData(kInitialTextureData, format);

        // Create a rendering pipeline that reads the pixels from the read-only storage texture and
        // uses green as the output color, otherwise uses red instead.
        std::ostringstream vsStream;
        vsStream << R"(
[[builtin(position)]] var<out> position : vec4<f32>;
[[location(0)]] var<out> o_color : vec4<f32>;
)" << CommonReadOnlyTestCode(format)
                 << R"(
[[stage(vertex)]] fn main() -> void {
  position = vec4<f32>(0.0, 0.0, 0.0, 1.0);
  if (doTest()) {
    o_color = vec4<f32>(0.0, 1.0, 0.0, 1.0);
  } else {
    o_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  }
})";
        const char* kFragmentShader = R"(
[[location(0)]] var<in> o_color : vec4<f32>;
[[location(0)]] var<out> fragColor : vec4<f32>;
[[stage(fragment)]] fn main() -> void {
  fragColor = o_color;
})";
        CheckDrawsGreen(vsStream.str().c_str(), kFragmentShader, readonlyStorageTexture);
    }
}

// Test that read-only storage textures are supported in fragment shader.
TEST_P(StorageTextureTests, ReadonlyStorageTextureInFragmentShader) {
    // TODO(crbug.com/dawn/624): this test fails on GLES. Investigate why.
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (IsOpenGLES() && !OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        // Prepare the read-only storage texture and fill it with the expected data.
        const std::vector<uint8_t> kInitialTextureData = GetExpectedData(format);
        wgpu::Texture readonlyStorageTexture =
            CreateTextureWithTestData(kInitialTextureData, format);

        // Create a rendering pipeline that reads the pixels from the read-only storage texture and
        // uses green as the output color if the pixel value is expected, otherwise uses red
        // instead.
        std::ostringstream fsStream;
        fsStream << R"(
[[location(0)]] var<out> o_color : vec4<f32>;
)" << CommonReadOnlyTestCode(format)
                 << R"(
[[stage(fragment)]] fn main() -> void {
  if (doTest()) {
    o_color = vec4<f32>(0.0, 1.0, 0.0, 1.0);
  } else {
    o_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  }
})";
        CheckDrawsGreen(kSimpleVertexShader, fsStream.str().c_str(), readonlyStorageTexture);
    }
}

// Test that write-only storage textures are supported in compute shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInComputeShader) {
    // TODO(crbug.com/dawn/647): diagnose and fix this OpenGL ES failure.
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (!OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        // TODO(jiawei.shao@intel.com): investigate why this test fails with RGBA8Snorm on Linux
        // Intel OpenGL driver.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsIntel() && IsOpenGL() && IsLinux()) {
            continue;
        }

        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

        // Write the expected pixel values into the write-only storage texture.
        const std::string computeShader = CommonWriteOnlyTestCode("compute", format);
        WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format);
    }
}

// Test that reading from one read-only storage texture then writing into another write-only storage
// texture in one dispatch are supported in compute shader.
TEST_P(StorageTextureTests, ReadWriteDifferentStorageTextureInOneDispatchInComputeShader) {
    // TODO(crbug.com/dawn/636): diagnose and fix this failure on OpenGL ES
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (IsOpenGLES() && !OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        // TODO(jiawei.shao@intel.com): investigate why this test fails with RGBA8Snorm on Linux
        // Intel OpenGL driver.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsIntel() && IsOpenGL() && IsLinux()) {
            continue;
        }

        // Prepare the read-only storage texture.
        const std::vector<uint8_t> kInitialTextureData = GetExpectedData(format);
        wgpu::Texture readonlyStorageTexture =
            CreateTextureWithTestData(kInitialTextureData, format);

        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

        // Write the expected pixel values into the write-only storage texture.
        const std::string computeShader = CommonReadWriteTestCode(format);
        ReadWriteIntoStorageTextureInComputePass(readonlyStorageTexture, writeonlyStorageTexture,
                                                 computeShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format);
    }
}

// Test that write-only storage textures are supported in fragment shader.
TEST_P(StorageTextureTests, WriteonlyStorageTextureInFragmentShader) {
    // TODO(crbug.com/dawn/647): diagnose and fix this OpenGL ES failure.
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    for (wgpu::TextureFormat format : utils::kAllTextureFormats) {
        if (!utils::TextureFormatSupportsStorageTexture(format)) {
            continue;
        }
        if (IsOpenGLES() && !OpenGLESSupportsStorageTexture(format)) {
            continue;
        }

        // TODO(jiawei.shao@intel.com): investigate why this test fails with RGBA8Snorm on Linux
        // Intel OpenGL driver.
        if (format == wgpu::TextureFormat::RGBA8Snorm && IsIntel() && IsOpenGL() && IsLinux()) {
            continue;
        }

        // Prepare the write-only storage texture.
        wgpu::Texture writeonlyStorageTexture =
            CreateTexture(format, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

        // Write the expected pixel values into the write-only storage texture.
        const std::string fragmentShader = CommonWriteOnlyTestCode("fragment", format);
        WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                            fragmentShader.c_str());

        // Verify the pixel data in the write-only storage texture is expected.
        CheckOutputStorageTexture(writeonlyStorageTexture, format);
    }
}

// Verify 2D array read-only storage texture works correctly.
TEST_P(StorageTextureTests, Readonly2DArrayStorageTexture) {
    constexpr uint32_t kArrayLayerCount = 3u;

    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

    const std::vector<uint8_t> initialTextureData =
        GetExpectedData(kTextureFormat, kArrayLayerCount);
    wgpu::Texture readonlyStorageTexture =
        CreateTextureWithTestData(initialTextureData, kTextureFormat);

    // Create a compute shader that reads the pixels from the read-only storage texture and writes 1
    // to DstBuffer if they all have to expected value.
    std::ostringstream csStream;
    csStream << R"(
[[block]] struct DstBuffer {
  [[offset(0)]] result : u32;
};

[[group(0), binding(1)]] var<storage_buffer> dstBuffer : DstBuffer;
)" << CommonReadOnlyTestCode(kTextureFormat, true)
             << R"(
[[stage(compute)]] fn main() -> void {
  if (doTest()) {
    dstBuffer.result = 1u;
  } else {
    dstBuffer.result = 0u;
  }
})";

    CheckResultInStorageBuffer(readonlyStorageTexture, csStream.str());
}

// Verify 2D array write-only storage texture works correctly.
TEST_P(StorageTextureTests, Writeonly2DArrayStorageTexture) {
    constexpr uint32_t kArrayLayerCount = 3u;

    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;

    // Prepare the write-only storage texture.
    wgpu::Texture writeonlyStorageTexture =
        CreateTexture(kTextureFormat, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc,
                      kWidth, kHeight, kArrayLayerCount);

    // Write the expected pixel values into the write-only storage texture.
    const std::string computeShader = CommonWriteOnlyTestCode("compute", kTextureFormat, true);
    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture, computeShader.c_str());

    // Verify the pixel data in the write-only storage texture is expected.
    CheckOutputStorageTexture(writeonlyStorageTexture, kTextureFormat, kArrayLayerCount);
}

// Test that multiple dispatches to increment values by ping-ponging between a read-only storage
// texture and a write-only storage texture are synchronized in one pass.
TEST_P(StorageTextureTests, ReadonlyAndWriteonlyStorageTexturePingPong) {
    // TODO(crbug.com/dawn/636): diagnose and fix this failure on OpenGL ES
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;
    wgpu::Texture storageTexture1 = CreateTexture(
        kTextureFormat, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc, 1u, 1u);
    wgpu::Texture storageTexture2 = CreateTexture(
        kTextureFormat, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc, 1u, 1u);

    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
[[group(0), binding(0)]] var<uniform_constant> Src : [[access(read)]]  texture_storage_2d<r32uint>;
[[group(0), binding(1)]] var<uniform_constant> Dst : [[access(write)]] texture_storage_2d<r32uint>;
[[stage(compute)]] fn main() -> void {
  var srcValue : vec4<u32> = textureLoad(Src, vec2<i32>(0, 0));
  srcValue.x = srcValue.x + 1u;
  textureStore(Dst, vec2<i32>(0, 0), srcValue);
}
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
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
    pass.Dispatch(1);

    // After the second dispatch the value in storageTexture1 should be 2u;
    pass.SetBindGroup(0, bindGroupB);
    pass.Dispatch(1);

    pass.EndPass();

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(uint32_t);
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TextureCopyView textureCopyView;
    textureCopyView.texture = storageTexture1;

    wgpu::BufferCopyView bufferCopyView = utils::CreateBufferCopyView(resultBuffer, 0, 256, 1);
    wgpu::Extent3D extent3D = {1, 1, 1};
    encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &extent3D);

    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    constexpr uint32_t kFinalPixelValueInTexture1 = 2u;
    EXPECT_BUFFER_U32_EQ(kFinalPixelValueInTexture1, resultBuffer, 0);
}

// Test that multiple dispatches to increment values by ping-ponging between a sampled texture and
// a write-only storage texture are synchronized in one pass.
TEST_P(StorageTextureTests, SampledAndWriteonlyStorageTexturePingPong) {
    // TODO(crbug.com/dawn/636): diagnose and fix this failure on OpenGL ES
    DAWN_SKIP_TEST_IF(IsOpenGLES());

    constexpr wgpu::TextureFormat kTextureFormat = wgpu::TextureFormat::R32Uint;
    wgpu::Texture storageTexture1 = CreateTexture(
        kTextureFormat,
        wgpu::TextureUsage::Sampled | wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc, 1u,
        1u);
    wgpu::Texture storageTexture2 = CreateTexture(
        kTextureFormat, wgpu::TextureUsage::Sampled | wgpu::TextureUsage::Storage, 1u, 1u);
    wgpu::ShaderModule module = utils::CreateShaderModuleFromWGSL(device, R"(
[[group(0), binding(0)]] var<uniform_constant> Src : texture_2d<u32>;
[[group(0), binding(1)]] var<uniform_constant> Dst : [[access(write)]] texture_storage_2d<r32uint>;
[[stage(compute)]] fn main() -> void {
  var srcValue : vec4<u32> = textureLoad(Src, vec2<i32>(0, 0));
  srcValue.x = srcValue.x + 1u;
  textureStore(Dst, vec2<i32>(0, 0), srcValue);
}
    )");

    wgpu::ComputePipelineDescriptor pipelineDesc = {};
    pipelineDesc.computeStage.module = module;
    pipelineDesc.computeStage.entryPoint = "main";
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
    pass.Dispatch(1);

    // After the second dispatch the value in storageTexture1 should be 2u;
    pass.SetBindGroup(0, bindGroupB);
    pass.Dispatch(1);

    pass.EndPass();

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = sizeof(uint32_t);
    bufferDescriptor.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer resultBuffer = device.CreateBuffer(&bufferDescriptor);

    wgpu::TextureCopyView textureCopyView;
    textureCopyView.texture = storageTexture1;

    wgpu::BufferCopyView bufferCopyView = utils::CreateBufferCopyView(resultBuffer, 0, 256, 1);
    wgpu::Extent3D extent3D = {1, 1, 1};
    encoder.CopyTextureToBuffer(&textureCopyView, &bufferCopyView, &extent3D);

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
[[group(0), binding(0)]] var<uniform_constant> dstImage : [[access(write)]] texture_storage_2d<r32uint>;

[[stage(fragment)]] fn main() -> void {
  textureStore(dstImage, vec2<i32>(0, 0), vec4<u32>(1u, 0u, 0u, 1u));
})";
    const char* kCommonWriteOnlyZeroInitTestCodeCompute = R"(
[[group(0), binding(0)]] var<uniform_constant> dstImage : [[access(write)]] texture_storage_2d<r32uint>;

[[stage(compute)]] fn main() -> void {
  textureStore(dstImage, vec2<i32>(0, 0), vec4<u32>(1u, 0u, 0u, 1u));
})";
};

// Verify that the texture is correctly cleared to 0 before its first usage as a read-only storage
// texture in a render pass.
TEST_P(StorageTextureZeroInitTests, ReadonlyStorageTextureClearsToZeroInRenderPass) {
    wgpu::Texture readonlyStorageTexture =
        CreateTexture(wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage);

    // Create a rendering pipeline that reads the pixels from the read-only storage texture and uses
    // green as the output color, otherwise uses red instead.
    const char* kVertexShader = kSimpleVertexShader;
    const std::string kFragmentShader = std::string(R"(
[[group(0), binding(0)]] var<uniform_constant> srcImage : [[access(read)]] texture_storage_2d<r32uint>;
[[location(0)]] var<out> o_color : vec4<f32>;
)") + kCommonReadOnlyZeroInitTestCode +
                                        R"(
[[stage(fragment)]] fn main() -> void {
  if (doTest()) {
    o_color = vec4<f32>(0.0, 1.0, 0.0, 1.0);
  } else {
    o_color = vec4<f32>(1.0, 0.0, 0.0, 1.0);
  }
})";
    CheckDrawsGreen(kVertexShader, kFragmentShader.c_str(), readonlyStorageTexture);
}

// Verify that the texture is correctly cleared to 0 before its first usage as a read-only storage
// texture in a compute pass.
TEST_P(StorageTextureZeroInitTests, ReadonlyStorageTextureClearsToZeroInComputePass) {
    wgpu::Texture readonlyStorageTexture =
        CreateTexture(wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage);

    // Create a compute shader that reads the pixels from the read-only storage texture and writes 1
    // to DstBuffer if they all have to expected value.
    const std::string kComputeShader = std::string(R"(
[[block]] struct DstBuffer {
  [[offset(0)]] result : u32;
};

[[group(0), binding(0)]] var<uniform_constant> srcImage : [[access(read)]] texture_storage_2d<r32uint>;
[[group(0), binding(1)]] var<storage_buffer> dstBuffer : DstBuffer;
)") + kCommonReadOnlyZeroInitTestCode + R"(
[[stage(compute)]] fn main() -> void {
  if (doTest()) {
    dstBuffer.result = 1u;
  } else {
    dstBuffer.result = 0u;
  }
})";

    CheckResultInStorageBuffer(readonlyStorageTexture, kComputeShader);
}

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// storage texture in a render pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInRenderPass) {
    // Prepare the write-only storage texture.
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

    WriteIntoStorageTextureInRenderPass(writeonlyStorageTexture, kSimpleVertexShader,
                                        kCommonWriteOnlyZeroInitTestCodeFragment);
    CheckOutputStorageTexture(writeonlyStorageTexture, kTexelSizeR32Uint, GetExpectedData());
}

// Verify that the texture is correctly cleared to 0 before its first usage as a write-only storage
// texture in a compute pass.
TEST_P(StorageTextureZeroInitTests, WriteonlyStorageTextureClearsToZeroInComputePass) {
    // Prepare the write-only storage texture.
    constexpr uint32_t kTexelSizeR32Uint = 4u;
    wgpu::Texture writeonlyStorageTexture = CreateTexture(
        wgpu::TextureFormat::R32Uint, wgpu::TextureUsage::Storage | wgpu::TextureUsage::CopySrc);

    WriteIntoStorageTextureInComputePass(writeonlyStorageTexture,
                                         kCommonWriteOnlyZeroInitTestCodeCompute);
    CheckOutputStorageTexture(writeonlyStorageTexture, kTexelSizeR32Uint, GetExpectedData());
}

DAWN_INSTANTIATE_TEST(StorageTextureZeroInitTests,
                      D3D12Backend({"nonzero_clear_resources_on_creation_for_testing"}),
                      OpenGLBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      OpenGLESBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      MetalBackend({"nonzero_clear_resources_on_creation_for_testing"}),
                      VulkanBackend({"nonzero_clear_resources_on_creation_for_testing"}));
