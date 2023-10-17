// Copyright 2019 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <CoreFoundation/CoreFoundation.h>
#include <CoreVideo/CVPixelBuffer.h>
#include <IOSurface/IOSurface.h>

#include <memory>
#include <thread>
#include <vector>

#include "dawn/tests/DawnTest.h"

#include "dawn/native/MetalBackend.h"
#include "dawn/utils/ComboRenderPipelineDescriptor.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn {
namespace {

void AddIntegerValue(CFMutableDictionaryRef dictionary, const CFStringRef key, int32_t value) {
    CFNumberRef number = CFNumberCreate(nullptr, kCFNumberSInt32Type, &value);
    CFDictionaryAddValue(dictionary, key, number);
    CFRelease(number);
}

class ScopedIOSurfaceRef {
  public:
    ScopedIOSurfaceRef() : mSurface(nullptr) {}
    explicit ScopedIOSurfaceRef(IOSurfaceRef surface) : mSurface(surface) {}

    ~ScopedIOSurfaceRef() {
        if (mSurface != nullptr) {
            CFRelease(mSurface);
            mSurface = nullptr;
        }
    }

    IOSurfaceRef get() const { return mSurface; }

    ScopedIOSurfaceRef(ScopedIOSurfaceRef&& other) {
        if (mSurface != nullptr) {
            CFRelease(mSurface);
        }
        mSurface = other.mSurface;
        other.mSurface = nullptr;
    }

    ScopedIOSurfaceRef& operator=(ScopedIOSurfaceRef&& other) {
        if (mSurface != nullptr) {
            CFRelease(mSurface);
        }
        mSurface = other.mSurface;
        other.mSurface = nullptr;

        return *this;
    }

    ScopedIOSurfaceRef(const ScopedIOSurfaceRef&) = delete;
    ScopedIOSurfaceRef& operator=(const ScopedIOSurfaceRef&) = delete;

  private:
    IOSurfaceRef mSurface = nullptr;
};

ScopedIOSurfaceRef CreateSinglePlaneIOSurface(uint32_t width,
                                              uint32_t height,
                                              uint32_t format,
                                              uint32_t bytesPerElement) {
    CFMutableDictionaryRef dict = CFDictionaryCreateMutable(
        kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
    AddIntegerValue(dict, kIOSurfaceWidth, width);
    AddIntegerValue(dict, kIOSurfaceHeight, height);
    AddIntegerValue(dict, kIOSurfacePixelFormat, format);
    AddIntegerValue(dict, kIOSurfaceBytesPerElement, bytesPerElement);

    IOSurfaceRef ioSurface = IOSurfaceCreate(dict);
    EXPECT_NE(nullptr, ioSurface);
    CFRelease(dict);

    return ScopedIOSurfaceRef(ioSurface);
}

class IOSurfaceTestBase : public DawnTest {
  public:
    wgpu::Texture WrapIOSurface(const wgpu::TextureDescriptor* descriptor,
                                IOSurfaceRef ioSurface,
                                bool isInitialized = true) {
        native::metal::ExternalImageDescriptorIOSurface externDesc;
        externDesc.cTextureDescriptor = reinterpret_cast<const WGPUTextureDescriptor*>(descriptor);
        externDesc.ioSurface = ioSurface;
        externDesc.isInitialized = isInitialized;
        WGPUTexture texture = native::metal::WrapIOSurface(device.Get(), &externDesc);
        return wgpu::Texture::Acquire(texture);
    }
};

// A small fixture used to initialize default data for the IOSurface validation tests.
// These tests are skipped if the harness is using the wire.
class IOSurfaceValidationTests : public IOSurfaceTestBase {
  public:
    IOSurfaceValidationTests() {
        defaultIOSurface = CreateSinglePlaneIOSurface(10, 10, kCVPixelFormatType_32BGRA, 4);

        descriptor.dimension = wgpu::TextureDimension::e2D;
        descriptor.format = wgpu::TextureFormat::BGRA8Unorm;
        descriptor.size = {10, 10, 1};
        descriptor.sampleCount = 1;
        descriptor.mipLevelCount = 1;
        descriptor.usage = wgpu::TextureUsage::RenderAttachment;
    }

  protected:
    wgpu::TextureDescriptor descriptor;
    ScopedIOSurfaceRef defaultIOSurface;
};

// Test a successful wrapping of an IOSurface in a texture
TEST_P(IOSurfaceValidationTests, Success) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get());
    ASSERT_NE(texture.Get(), nullptr);
}

// Test an error occurs if the texture descriptor is invalid
TEST_P(IOSurfaceValidationTests, InvalidTextureDescriptor) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    wgpu::ChainedStruct chainedDescriptor;
    descriptor.nextInChain = &chainedDescriptor;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor dimension isn't 2D
// TODO(crbug.com/dawn/814): Test 1D textures when implemented
TEST_P(IOSurfaceValidationTests, InvalidTextureDimension) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.dimension = wgpu::TextureDimension::e3D;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor mip level count isn't 1
TEST_P(IOSurfaceValidationTests, InvalidMipLevelCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.mipLevelCount = 2;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor depth isn't 1
TEST_P(IOSurfaceValidationTests, InvalidDepth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.depthOrArrayLayers = 2;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor sample count isn't 1
TEST_P(IOSurfaceValidationTests, InvalidSampleCount) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.sampleCount = 4;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor width doesn't match the surface's
TEST_P(IOSurfaceValidationTests, InvalidWidth) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.width = 11;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor height doesn't match the surface's
TEST_P(IOSurfaceValidationTests, InvalidHeight) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.size.height = 11;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Test an error occurs if the descriptor format isn't compatible with the IOSurface's
TEST_P(IOSurfaceValidationTests, InvalidFormat) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    descriptor.format = wgpu::TextureFormat::R8Unorm;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

class IOSurfaceTransientAttachmentValidationTests : public IOSurfaceValidationTests {
    void SetUp() override {
        IOSurfaceValidationTests::SetUp();

        // Skip all tests if the transient attachments feature is not supported.
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TransientAttachments}));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TransientAttachments})) {
            requiredFeatures.push_back(wgpu::FeatureName::TransientAttachments);
        }
        return requiredFeatures;
    }
};

// Test that an error occurs if the transient attachment is specified.
TEST_P(IOSurfaceTransientAttachmentValidationTests, ErrorWhenSpecified) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    descriptor.usage |= wgpu::TextureUsage::TransientAttachment;

    ASSERT_DEVICE_ERROR(wgpu::Texture texture = WrapIOSurface(&descriptor, defaultIOSurface.get()));
    ASSERT_EQ(texture.Get(), nullptr);
}

// Fixture to test using IOSurfaces through different usages.
// These tests are skipped if the harness is using the wire.
class IOSurfaceUsageTests : public IOSurfaceTestBase {
  public:
    // Test that sampling a 1x1 works.
    void DoSampleTest(IOSurfaceRef ioSurface,
                      wgpu::TextureFormat format,
                      void* data,
                      size_t dataSize,
                      utils::RGBA8 expectedColor) {
        // Write the data to the IOSurface
        IOSurfaceLock(ioSurface, 0, nullptr);
        memcpy(IOSurfaceGetBaseAddress(ioSurface), data, dataSize);
        IOSurfaceUnlock(ioSurface, 0, nullptr);

        // The simplest texture sampling pipeline.
        wgpu::RenderPipeline pipeline;
        {
            wgpu::ShaderModule vs = utils::CreateShaderModule(device, R"(
                struct VertexOut {
                    @location(0) texCoord : vec2f,
                    @builtin(position) position : vec4f,
                }

                @vertex
                fn main(@builtin(vertex_index) VertexIndex : u32) -> VertexOut {
                    var pos = array(
                        vec2f(-2.0, -2.0),
                        vec2f(-2.0,  2.0),
                        vec2f( 2.0, -2.0),
                        vec2f(-2.0,  2.0),
                        vec2f( 2.0, -2.0),
                        vec2f( 2.0,  2.0));

                    var texCoord = array(
                        vec2f(0.0, 0.0),
                        vec2f(0.0, 1.0),
                        vec2f(1.0, 0.0),
                        vec2f(0.0, 1.0),
                        vec2f(1.0, 0.0),
                        vec2f(1.0, 1.0));

                    var output : VertexOut;
                    output.position = vec4f(pos[VertexIndex], 0.0, 1.0);
                    output.texCoord = texCoord[VertexIndex];
                    return output;
                }
            )");
            wgpu::ShaderModule fs = utils::CreateShaderModule(device, R"(
                @group(0) @binding(0) var sampler0 : sampler;
                @group(0) @binding(1) var texture0 : texture_2d<f32>;

                @fragment
                fn main(@location(0) texCoord : vec2f) -> @location(0) vec4f {
                    return textureSample(texture0, sampler0, texCoord);
                }
            )");

            utils::ComboRenderPipelineDescriptor descriptor;
            descriptor.vertex.module = vs;
            descriptor.cFragment.module = fs;
            descriptor.cTargets[0].format = wgpu::TextureFormat::RGBA8Unorm;

            pipeline = device.CreateRenderPipeline(&descriptor);
        }

        // The bindgroup containing the texture view for the ioSurface as well as the sampler.
        wgpu::BindGroup bindGroup;
        {
            wgpu::TextureDescriptor textureDescriptor;
            textureDescriptor.dimension = wgpu::TextureDimension::e2D;
            textureDescriptor.format = format;
            textureDescriptor.size = {1, 1, 1};
            textureDescriptor.sampleCount = 1;
            textureDescriptor.mipLevelCount = 1;
            textureDescriptor.usage = wgpu::TextureUsage::TextureBinding;
            wgpu::Texture wrappingTexture = WrapIOSurface(&textureDescriptor, ioSurface);

            wgpu::TextureView textureView = wrappingTexture.CreateView();

            wgpu::Sampler sampler = device.CreateSampler();

            bindGroup = utils::MakeBindGroup(device, pipeline.GetBindGroupLayout(0),
                                             {{0, sampler}, {1, textureView}});
        }

        // Submit commands samping from the ioSurface and writing the result to renderPass.color
        utils::BasicRenderPass renderPass = utils::CreateBasicRenderPass(device, 1, 1);
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        {
            wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPass.renderPassInfo);
            pass.SetPipeline(pipeline);
            pass.SetBindGroup(0, bindGroup);
            pass.Draw(6);
            pass.End();
        }

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_PIXEL_RGBA8_EQ(expectedColor, renderPass.color, 0, 0);
    }

    // Test that clearing using BeginRenderPass writes correct data in the ioSurface.
    void DoClearTest(IOSurfaceRef ioSurface,
                     wgpu::TextureFormat format,
                     void* data,
                     size_t dataSize) {
        // Get a texture view for the ioSurface
        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = wgpu::TextureDimension::e2D;
        textureDescriptor.format = format;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.sampleCount = 1;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment;
        wgpu::Texture ioSurfaceTexture = WrapIOSurface(&textureDescriptor, ioSurface);

        wgpu::TextureView ioSurfaceView = ioSurfaceTexture.CreateView();

        utils::ComboRenderPassDescriptor renderPassDescriptor({ioSurfaceView}, {});
        renderPassDescriptor.cColorAttachments[0].clearValue = {1 / 255.0f, 2 / 255.0f, 3 / 255.0f,
                                                                4 / 255.0f};

        // Execute commands to clear the ioSurface
        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        wgpu::RenderPassEncoder pass = encoder.BeginRenderPass(&renderPassDescriptor);
        pass.End();

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Wait for the commands touching the IOSurface to be scheduled
        native::metal::WaitForCommandsToBeScheduled(device.Get());

        // Check the correct data was written
        IOSurfaceLock(ioSurface, kIOSurfaceLockReadOnly, nullptr);
        ASSERT_EQ(0, memcmp(IOSurfaceGetBaseAddress(ioSurface), data, dataSize));
        IOSurfaceUnlock(ioSurface, kIOSurfaceLockReadOnly, nullptr);
    }
};

// Test sampling from a R8 IOSurface
TEST_P(IOSurfaceUsageTests, SampleFromR8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface =
        CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_OneComponent8, 1);

    uint8_t data = 0x01;
    DoSampleTest(ioSurface.get(), wgpu::TextureFormat::R8Unorm, &data, sizeof(data),
                 utils::RGBA8(1, 0, 0, 255));
}

// Test clearing a R8 IOSurface
TEST_P(IOSurfaceUsageTests, ClearR8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface =
        CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_OneComponent8, 1);

    uint8_t data = 0x01;
    DoClearTest(ioSurface.get(), wgpu::TextureFormat::R8Unorm, &data, sizeof(data));
}

// Test sampling from a RG8 IOSurface
TEST_P(IOSurfaceUsageTests, SampleFromRG8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface =
        CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_TwoComponent8, 2);

    uint16_t data = 0x0102;  // Stored as (G, R)
    DoSampleTest(ioSurface.get(), wgpu::TextureFormat::RG8Unorm, &data, sizeof(data),
                 utils::RGBA8(2, 1, 0, 255));
}

// Test clearing a RG8 IOSurface
TEST_P(IOSurfaceUsageTests, ClearRG8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface =
        CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_TwoComponent8, 2);

    uint16_t data = 0x0201;
    DoClearTest(ioSurface.get(), wgpu::TextureFormat::RG8Unorm, &data, sizeof(data));
}

// Test sampling from a BGRA8 IOSurface
TEST_P(IOSurfaceUsageTests, SampleFromBGRA8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32BGRA, 4);

    uint32_t data = 0x01020304;  // Stored as (A, R, G, B)
    DoSampleTest(ioSurface.get(), wgpu::TextureFormat::BGRA8Unorm, &data, sizeof(data),
                 utils::RGBA8(2, 3, 4, 1));
}

// Test clearing a BGRA8 IOSurface
TEST_P(IOSurfaceUsageTests, ClearBGRA8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32BGRA, 4);

    uint32_t data = 0x04010203;
    DoClearTest(ioSurface.get(), wgpu::TextureFormat::BGRA8Unorm, &data, sizeof(data));
}

// Test sampling from an RGBA8 IOSurface
TEST_P(IOSurfaceUsageTests, SampleFromRGBA8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32RGBA, 4);

    uint32_t data = 0x01020304;  // Stored as (A, B, G, R)
    DoSampleTest(ioSurface.get(), wgpu::TextureFormat::RGBA8Unorm, &data, sizeof(data),
                 utils::RGBA8(4, 3, 2, 1));
}

// Test clearing an RGBA8 IOSurface
TEST_P(IOSurfaceUsageTests, ClearRGBA8IOSurface) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32RGBA, 4);

    uint32_t data = 0x04030201;
    DoClearTest(ioSurface.get(), wgpu::TextureFormat::RGBA8Unorm, &data, sizeof(data));
}

// Test that texture with color is cleared when isInitialized = false
TEST_P(IOSurfaceUsageTests, UninitializedTextureIsCleared) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32RGBA, 4);
    uint32_t data = 0x04030201;

    IOSurfaceLock(ioSurface.get(), 0, nullptr);
    memcpy(IOSurfaceGetBaseAddress(ioSurface.get()), &data, sizeof(data));
    IOSurfaceUnlock(ioSurface.get(), 0, nullptr);

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.dimension = wgpu::TextureDimension::e2D;
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.sampleCount = 1;
    textureDescriptor.mipLevelCount = 1;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

    // wrap ioSurface and ensure color is not visible when isInitialized set to false
    wgpu::Texture ioSurfaceTexture = WrapIOSurface(&textureDescriptor, ioSurface.get(), false);
    EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 0, 0, 0), ioSurfaceTexture, 0, 0);

    native::metal::ExternalImageIOSurfaceEndAccessDescriptor endAccessDesc;
    native::metal::IOSurfaceEndAccess(ioSurfaceTexture.Get(), &endAccessDesc);
    EXPECT_TRUE(endAccessDesc.isInitialized);
}

// Test that exporting a texture wrapping an IOSurface sets the isInitialized bit to
// false if the contents are discard.
TEST_P(IOSurfaceUsageTests, UninitializedOnEndAccess) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32RGBA, 4);
    uint32_t data = 0x04030201;

    IOSurfaceLock(ioSurface.get(), 0, nullptr);
    memcpy(IOSurfaceGetBaseAddress(ioSurface.get()), &data, sizeof(data));
    IOSurfaceUnlock(ioSurface.get(), 0, nullptr);

    wgpu::TextureDescriptor textureDescriptor;
    textureDescriptor.dimension = wgpu::TextureDimension::e2D;
    textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDescriptor.size = {1, 1, 1};
    textureDescriptor.sampleCount = 1;
    textureDescriptor.mipLevelCount = 1;
    textureDescriptor.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

    // Wrap ioSurface
    wgpu::Texture ioSurfaceTexture = WrapIOSurface(&textureDescriptor, ioSurface.get(), true);

    // Uninitialize the teuxture with a render pass.
    utils::ComboRenderPassDescriptor renderPassDescriptor({ioSurfaceTexture.CreateView()});
    renderPassDescriptor.cColorAttachments[0].storeOp = wgpu::StoreOp::Discard;
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.BeginRenderPass(&renderPassDescriptor).End();
    wgpu::CommandBuffer commandBuffer = encoder.Finish();
    queue.Submit(1, &commandBuffer);

    native::metal::ExternalImageIOSurfaceEndAccessDescriptor endAccessDesc;
    native::metal::IOSurfaceEndAccess(ioSurfaceTexture.Get(), &endAccessDesc);
    EXPECT_FALSE(endAccessDesc.isInitialized);
}

// Test that an IOSurface may be imported across multiple devices.
TEST_P(IOSurfaceUsageTests, WriteThenConcurrentReadThenWrite) {
    DAWN_TEST_UNSUPPORTED_IF(UsesWire());

    ScopedIOSurfaceRef ioSurface = CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32RGBA, 4);
    uint32_t data = 0x04030201;

    IOSurfaceLock(ioSurface.get(), 0, nullptr);
    memcpy(IOSurfaceGetBaseAddress(ioSurface.get()), &data, sizeof(data));
    IOSurfaceUnlock(ioSurface.get(), 0, nullptr);

    // Make additional devices. We will import with the writeDevice, then read it concurrently with
    // both readDevices.
    wgpu::Device writeDevice = device;
    wgpu::Device readDevice1 = CreateDevice();
    wgpu::Device readDevice2 = CreateDevice();

    wgpu::TextureDescriptor textureDesc;
    textureDesc.dimension = wgpu::TextureDimension::e2D;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    textureDesc.size = {1, 1, 1};
    textureDesc.sampleCount = 1;
    textureDesc.mipLevelCount = 1;
    textureDesc.usage = wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

    // Wrap ioSurface
    native::metal::ExternalImageDescriptorIOSurface writeExternDesc;
    writeExternDesc.cTextureDescriptor =
        reinterpret_cast<const WGPUTextureDescriptor*>(&textureDesc);
    writeExternDesc.ioSurface = ioSurface.get();
    writeExternDesc.isInitialized = true;

    wgpu::Texture writeTexture =
        wgpu::Texture::Acquire(native::metal::WrapIOSurface(writeDevice.Get(), &writeExternDesc));

    // Clear the texture to green.
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({writeTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].clearValue = {0.0, 1.0, 0.0, 1.0};
        wgpu::CommandEncoder encoder = writeDevice.CreateCommandEncoder();
        encoder.BeginRenderPass(&renderPassDescriptor).End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        writeDevice.GetQueue().Submit(1, &commandBuffer);
    }

    // End access.
    native::metal::ExternalImageIOSurfaceEndAccessDescriptor endWriteAccessDesc;
    native::metal::IOSurfaceEndAccess(writeTexture.Get(), &endWriteAccessDesc);
    EXPECT_TRUE(endWriteAccessDesc.isInitialized);

    native::metal::ExternalImageDescriptorIOSurface externDesc;
    externDesc.cTextureDescriptor = reinterpret_cast<const WGPUTextureDescriptor*>(&textureDesc);
    externDesc.ioSurface = ioSurface.get();
    externDesc.isInitialized = true;
    externDesc.waitEvents.push_back(
        {endWriteAccessDesc.sharedEvent, endWriteAccessDesc.signaledValue});

    // Wrap on two separate devices to read it.
    wgpu::Texture readTexture1 =
        wgpu::Texture::Acquire(native::metal::WrapIOSurface(readDevice1.Get(), &externDesc));
    wgpu::Texture readTexture2 =
        wgpu::Texture::Acquire(native::metal::WrapIOSurface(readDevice2.Get(), &externDesc));

    // Expect the texture to be green
    EXPECT_TEXTURE_EQ(readDevice1, utils::RGBA8(0, 255, 0, 255), readTexture1, {0, 0});
    EXPECT_TEXTURE_EQ(readDevice2, utils::RGBA8(0, 255, 0, 255), readTexture2, {0, 0});

    // End access on both read textures.
    native::metal::ExternalImageIOSurfaceEndAccessDescriptor endReadAccessDesc1;
    native::metal::IOSurfaceEndAccess(readTexture1.Get(), &endReadAccessDesc1);
    EXPECT_TRUE(endReadAccessDesc1.isInitialized);

    native::metal::ExternalImageIOSurfaceEndAccessDescriptor endReadAccessDesc2;
    native::metal::IOSurfaceEndAccess(readTexture2.Get(), &endReadAccessDesc2);
    EXPECT_TRUE(endReadAccessDesc2.isInitialized);

    // Import again for writing. It should not race with the previous reads.
    writeExternDesc.waitEvents = {endReadAccessDesc1, endReadAccessDesc2};
    writeExternDesc.isInitialized = true;
    writeTexture =
        wgpu::Texture::Acquire(native::metal::WrapIOSurface(writeDevice.Get(), &writeExternDesc));

    // Clear the texture to blue.
    {
        utils::ComboRenderPassDescriptor renderPassDescriptor({writeTexture.CreateView()});
        renderPassDescriptor.cColorAttachments[0].clearValue = {0.0, 0.0, 1.0, 1.0};
        wgpu::CommandEncoder encoder = writeDevice.CreateCommandEncoder();
        encoder.BeginRenderPass(&renderPassDescriptor).End();
        wgpu::CommandBuffer commandBuffer = encoder.Finish();
        writeDevice.GetQueue().Submit(1, &commandBuffer);
    }
    // Finally, expect the contents to be blue now.
    EXPECT_TEXTURE_EQ(writeDevice, utils::RGBA8(0, 0, 255, 255), writeTexture, {0, 0});
    native::metal::IOSurfaceEndAccess(writeTexture.Get(), &endWriteAccessDesc);
    EXPECT_TRUE(endWriteAccessDesc.isInitialized);
}

class IOSurfaceMultithreadTests : public IOSurfaceUsageTests {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> features;
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        if (!UsesWire()) {
            features.push_back(wgpu::FeatureName::ImplicitDeviceSynchronization);
        }
        return features;
    }

    void SetUp() override {
        IOSurfaceUsageTests::SetUp();
        // TODO(crbug.com/dawn/1678): DawnWire doesn't support thread safe API yet.
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
    }
};

// Test that texture with color is cleared when isInitialized = false. There shoudn't be any data
// race if multiple of them are created on multiple threads.
TEST_P(IOSurfaceMultithreadTests, UninitializedTexturesAreCleared_OnMultipleThreads) {
    utils::RunInParallel(10, [this](uint32_t) {
        ScopedIOSurfaceRef ioSurface =
            CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32RGBA, 4);
        uint32_t data = 0x04030201;

        IOSurfaceLock(ioSurface.get(), 0, nullptr);
        memcpy(IOSurfaceGetBaseAddress(ioSurface.get()), &data, sizeof(data));
        IOSurfaceUnlock(ioSurface.get(), 0, nullptr);

        wgpu::TextureDescriptor textureDescriptor;
        textureDescriptor.dimension = wgpu::TextureDimension::e2D;
        textureDescriptor.format = wgpu::TextureFormat::RGBA8Unorm;
        textureDescriptor.size = {1, 1, 1};
        textureDescriptor.sampleCount = 1;
        textureDescriptor.mipLevelCount = 1;
        textureDescriptor.usage =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::CopySrc;

        // wrap ioSurface and ensure color is not visible when isInitialized set to false
        wgpu::Texture ioSurfaceTexture = WrapIOSurface(&textureDescriptor, ioSurface.get(), false);
        EXPECT_PIXEL_RGBA8_EQ(utils::RGBA8(0, 0, 0, 0), ioSurfaceTexture, 0, 0);

        native::metal::ExternalImageIOSurfaceEndAccessDescriptor endAccessDesc;
        native::metal::IOSurfaceEndAccess(ioSurfaceTexture.Get(), &endAccessDesc);
        EXPECT_TRUE(endAccessDesc.isInitialized);
    });
}

// Test that wrapping multiple IOSurface and clear them on multiple threads work.
TEST_P(IOSurfaceMultithreadTests, WrapAndClear_OnMultipleThreads) {
    utils::RunInParallel(10, [this](uint32_t) {
        ScopedIOSurfaceRef ioSurface =
            CreateSinglePlaneIOSurface(1, 1, kCVPixelFormatType_32BGRA, 4);

        uint32_t data = 0x04010203;
        DoClearTest(ioSurface.get(), wgpu::TextureFormat::BGRA8Unorm, &data, sizeof(data));
    });
}

DAWN_INSTANTIATE_TEST(IOSurfaceValidationTests, MetalBackend());
DAWN_INSTANTIATE_TEST(IOSurfaceTransientAttachmentValidationTests, MetalBackend());
DAWN_INSTANTIATE_TEST(IOSurfaceUsageTests, MetalBackend());
DAWN_INSTANTIATE_TEST(IOSurfaceMultithreadTests, MetalBackend());

}  // anonymous namespace
}  // namespace dawn
