// Copyright 2025 The Dawn & Tint Authors
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

// One-off "spot"/regression/smoke tests for Emdawnwebgpu.

#include <dawn/webgpu_cpp_print.h>
#include <emscripten.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <webgpu/webgpu_cpp.h>

#include <array>
#include <string>
#include <utility>

#include "dawn/utils/WGPUHelpers.h"

namespace {

namespace utils = dawn::utils;
using testing::_;
using testing::HasSubstr;

class SpotTests : public testing::Test {
  public:
    void SetUp() override {
        static constexpr auto kInstanceFeatures =
            std::array{wgpu::InstanceFeatureName::TimedWaitAny};
        wgpu::InstanceDescriptor instanceDesc{.requiredFeatureCount = kInstanceFeatures.size(),
                                              .requiredFeatures = kInstanceFeatures.data()};
        mInstance = wgpu::CreateInstance(&instanceDesc);

        wgpu::Adapter adapter;
        EXPECT_EQ(wgpu::WaitStatus::Success,
                  mInstance.WaitAny(mInstance.RequestAdapter(
                                        nullptr, wgpu::CallbackMode::WaitAnyOnly,
                                        [&adapter](wgpu::RequestAdapterStatus, wgpu::Adapter a,
                                                   wgpu::StringView) { adapter = std::move(a); }),
                                    UINT64_MAX));
        EXPECT_TRUE(adapter);
        wgpu::SupportedFeatures features;
        adapter.GetFeatures(&features);

        wgpu::DeviceDescriptor deviceDesc;
        // Enable all available features
        deviceDesc.requiredFeatureCount = features.featureCount;
        deviceDesc.requiredFeatures = features.features;
        wgpu::Device device;
        EXPECT_EQ(wgpu::WaitStatus::Success,
                  mInstance.WaitAny(
                      adapter.RequestDevice(&deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
                                            [&device](wgpu::RequestDeviceStatus, wgpu::Device d,
                                                      wgpu::StringView) { device = std::move(d); }),
                      UINT64_MAX));
        EXPECT_TRUE(device);
        this->mAdapter = adapter;
        this->mDevice = device;
    }

  protected:
    wgpu::Instance mInstance;
    wgpu::Adapter mAdapter;
    wgpu::Device mDevice;
};

TEST_F(SpotTests, QuerySet) {
    // Spot test wgpuQuerySetGetType which uses indexOf on an int-to-string table.
    wgpu::QuerySetDescriptor querySetDesc{.type = wgpu::QueryType::Timestamp, .count = 1};
    wgpu::QuerySet querySet = mDevice.CreateQuerySet(&querySetDesc);
    EXPECT_TRUE(querySet);
    EXPECT_EQ(querySet.GetType(), querySetDesc.type);
}

TEST_F(SpotTests, BufferGetMapState) {
    // Spot test one of the string-to-int tables (Int_BufferMapState) to make sure
    // that Closure's minification didn't minify its keys.
    wgpu::BufferDescriptor bufferDesc{.usage = wgpu::BufferUsage::CopyDst, .size = 4};
    wgpu::Buffer buffer = mDevice.CreateBuffer(&bufferDesc);
    EXPECT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Unmapped);
}

TEST_F(SpotTests, GetCompilationInfo) {
    for (bool valid : {true, false}) {
        wgpu::ShaderSourceWGSL wgslDesc{};
        wgslDesc.code = valid ? "" : "some invalid code";

        wgpu::ShaderModuleDescriptor descriptor{};
        descriptor.nextInChain = &wgslDesc;
        auto sm = mDevice.CreateShaderModule(&descriptor);
        auto future = sm.GetCompilationInfo(
            wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::CompilationInfoRequestStatus, const wgpu::CompilationInfo* compilationInfo) {
                // We shouldn't have tried to allocate stuff if there were no messages.
                EXPECT_EQ(compilationInfo->messageCount == 0, compilationInfo->messages == nullptr);

                // After this, any compilation info will be freed. (There was a bug here which
                // this test catches, but only in ASAN builds.)
            });
        EXPECT_EQ(wgpu::WaitStatus::Success, mInstance.WaitAny(future, UINT64_MAX));
    }
}

TEST_F(SpotTests, ExternalRefCount) {
    wgpu::BufferDescriptor bufferDesc{
        .usage = wgpu::BufferUsage::MapRead, .size = 16, .mappedAtCreation = true};

    wgpu::Buffer buffer = mDevice.CreateBuffer(&bufferDesc);
    ASSERT_TRUE(buffer);
    EXPECT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Mapped);
    {
        // Add and then release an extra external ref.
        wgpu::Buffer tmp = buffer;
    }

    // Make sure the device wasn't implicitly destroyed (because we thought
    // the last external ref was dropped).
    EXPECT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Mapped);
}

TEST_F(SpotTests, InvalidComponentSwizzle) {
    wgpu::TextureDescriptor textureDesc = {};
    textureDesc.size = {1, 1, 0};
    textureDesc.usage = wgpu::TextureUsage::TextureBinding;
    textureDesc.format = wgpu::TextureFormat::RGBA8Unorm;
    wgpu::Texture texture = mDevice.CreateTexture(&textureDesc);

    wgpu::TextureViewDescriptor viewDesc = {};
    wgpu::TextureComponentSwizzleDescriptor swizzleDesc = {};
    // An invalid ComponentSwizzle value doesn't crash.
    swizzleDesc.swizzle.r = static_cast<wgpu::ComponentSwizzle>(-1);
    viewDesc.nextInChain = &swizzleDesc;
    wgpu::TextureView view = texture.CreateView(&viewDesc);
    ASSERT_TRUE(view);
}

template <typename T>
void TestGetFeatures(T o) {  // o is either wgpu::Adapter or wgpu::Device.
    wgpu::SupportedFeatures f;
    o.GetFeatures(&f);
    auto features = std::span(f.features, f.featureCount);
    for (auto feature : features) {
        // GetFeatures should filter out any unknown features.
        EXPECT_NE(feature, wgpu::FeatureName{0});
        EXPECT_TRUE(o.HasFeature(feature));
    }

    // Test some specific features to make sure minification worked.
    bool haveCompressedTexture = false;
    if (EM_ASM_INT(
            { return WebGPU.getJsObject($0).features.has('texture-compression-bc'); }, o.Get())) {
        auto feature = wgpu::FeatureName::TextureCompressionBC;
        EXPECT_NE(std::find(features.begin(), features.end(), feature), features.end());
        EXPECT_TRUE(o.HasFeature(feature));
        haveCompressedTexture = true;
    }
    if (EM_ASM_INT(
            { return WebGPU.getJsObject($0).features.has('texture-compression-etc2'); }, o.Get())) {
        auto feature = wgpu::FeatureName::TextureCompressionETC2;
        EXPECT_NE(std::find(features.begin(), features.end(), feature), features.end());
        EXPECT_TRUE(o.HasFeature(feature));
        haveCompressedTexture = true;
    }
    EXPECT_TRUE(haveCompressedTexture);

    // "subgroups" is a valid JS identifier (no hyphens), so it's
    // vulnerable to Closure minification.
    if (EM_ASM_INT({ return WebGPU.getJsObject($0).features.has('subgroups'); }, o.Get())) {
        auto feature = wgpu::FeatureName::Subgroups;
        EXPECT_NE(std::find(features.begin(), features.end(), feature), features.end());
        EXPECT_TRUE(o.HasFeature(feature));
    }
}

// Test GetFeatures and HasFeature enum lookups.
TEST_F(SpotTests, GetFeatures) {
    TestGetFeatures(mAdapter);
    TestGetFeatures(mDevice);
}

TEST_F(SpotTests, GetWGSLLanguageFeatures) {
    wgpu::SupportedWGSLLanguageFeatures f;
    mInstance.GetWGSLLanguageFeatures(&f);
    auto features = std::span(f.features, f.featureCount);
    for (auto feature : features) {
        // GetWGSLLanguageFeatures should filter out any unknown features.
        EXPECT_NE(feature, wgpu::WGSLLanguageFeatureName{0});
        EXPECT_TRUE(mInstance.HasWGSLLanguageFeature(feature));
    }

    // Test a specific feature to make sure minification worked.
    // WGSL feature names are valid JS identifiers (they use underscores instead
    // of hyphens), so they're vulnerable to Closure minification.
    if (EM_ASM_INT({
            return navigator.gpu.wgslLanguageFeatures.has('unrestricted_pointer_parameters');
        })) {
        auto feature = wgpu::WGSLLanguageFeatureName::UnrestrictedPointerParameters;
        EXPECT_NE(std::find(features.begin(), features.end(), feature), features.end());
        EXPECT_TRUE(mInstance.HasWGSLLanguageFeature(feature));
    }
}

TEST_F(SpotTests, ImportExternalTexture) {
    auto cExternalTexture = static_cast<WGPUExternalTexture>(EM_ASM_PTR(
        {
            const cDevice = $0;
            const device = WebGPU.getJsObject(cDevice);

            const cvs = document.createElement('canvas');
            cvs.width = 1;
            cvs.height = 1;
            const ctx = cvs.getContext('2d');
            ctx.fillStyle = '#0f0';
            ctx.fillRect(0, 0, 1, 1);
            window.myVideoFrame = new VideoFrame(cvs, {timestamp : 0});

            const jsExternalTexture = device.importExternalTexture({source : window.myVideoFrame});
            const cExternalTexture = WebGPU.importJsExternalTexture(jsExternalTexture);
            return cExternalTexture;
        },
        mDevice.Get()));
    auto externalTexture = wgpu::ExternalTexture::Acquire(cExternalTexture);

    wgpu::BufferDescriptor bufferDesc{
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        .size = sizeof(uint32_t),
    };
    auto buffer = mDevice.CreateBuffer(&bufferDesc);

    wgpu::BufferDescriptor readbackDesc{
        .usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead,
        .size = sizeof(uint32_t),
    };
    auto readback = mDevice.CreateBuffer(&readbackDesc);

    wgpu::BindGroupLayout bgl = utils::MakeBindGroupLayout(
        mDevice, {{0, wgpu::ShaderStage::Compute, &utils::kExternalTextureBindingLayout},
                  {1, wgpu::ShaderStage::Compute, wgpu::BufferBindingType::Storage}});
    wgpu::BindGroup bg = utils::MakeBindGroup(mDevice, bgl, {{0, externalTexture}, {1, buffer}});

    auto module = utils::CreateShaderModule(mDevice, R"(
        @group(0) @binding(0) var t: texture_external;
        @group(0) @binding(1) var<storage, read_write> b: u32;

        @compute @workgroup_size(1) fn main() {
            b = pack4x8unorm(textureLoad(t, vec2u(0, 0)));
        })");

    wgpu::ComputePipelineDescriptor pipelineDesc{
        .layout = utils::MakeBasicPipelineLayout(mDevice, &bgl),
        .compute = {.module = module},
    };
    auto pipeline = mDevice.CreateComputePipeline(&pipelineDesc);

    auto encoder = mDevice.CreateCommandEncoder();
    {
        auto pass = encoder.BeginComputePass();
        pass.SetBindGroup(0, bg);
        pass.SetPipeline(pipeline);
        pass.DispatchWorkgroups(1);
        pass.End();
    }
    encoder.CopyBufferToBuffer(buffer, 0, readback, 0, sizeof(uint32_t));
    auto commandBuffer = encoder.Finish();
    mDevice.GetQueue().Submit(1, &commandBuffer);

    // Note we can't (yet) use EXPECT_BUFFER_U32_EQ here.
    uint32_t result = 0;
    mInstance.WaitAny(
        readback.MapAsync(
            wgpu::MapMode::Read, 0, wgpu::kWholeMapSize, wgpu::CallbackMode::WaitAnyOnly,
            [&](wgpu::MapAsyncStatus, wgpu::StringView) {
                result = static_cast<const uint32_t*>(readback.GetConstMappedRange())[0];
                readback.Unmap();
            }),
        UINT64_MAX);
    EXPECT_EQ(result, uint32_t(0xff00ff00));  // ABGR

    EM_ASM({
        // VideoFrames should always be closed manually.
        window.myVideoFrame.close();
        delete window.myVideoFrame;
    });
}

}  // namespace
