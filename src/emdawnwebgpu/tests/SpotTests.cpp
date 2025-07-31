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

namespace {

using testing::_;
using testing::HasSubstr;

class SpotTests : public testing::Test {
  public:
    void SetUp() override {
        static constexpr auto kInstanceFeatures =
            std::array{wgpu::InstanceFeatureName::TimedWaitAny};
        wgpu::InstanceDescriptor instanceDesc{.requiredFeatureCount = kInstanceFeatures.size(),
                                              .requiredFeatures = kInstanceFeatures.data()};
        instance = wgpu::CreateInstance(&instanceDesc);

        wgpu::Adapter adapter;
        EXPECT_EQ(wgpu::WaitStatus::Success,
                  instance.WaitAny(instance.RequestAdapter(
                                       nullptr, wgpu::CallbackMode::WaitAnyOnly,
                                       [&adapter](wgpu::RequestAdapterStatus, wgpu::Adapter a,
                                                  wgpu::StringView) { adapter = std::move(a); }),
                                   UINT64_MAX));
        EXPECT_TRUE(adapter);

        static constexpr auto kFeatures = std::array{wgpu::FeatureName::TimestampQuery};
        wgpu::DeviceDescriptor deviceDesc;
        deviceDesc.requiredFeatureCount = kFeatures.size();
        deviceDesc.requiredFeatures = kFeatures.data();
        wgpu::Device device;
        EXPECT_EQ(wgpu::WaitStatus::Success,
                  instance.WaitAny(
                      adapter.RequestDevice(&deviceDesc, wgpu::CallbackMode::WaitAnyOnly,
                                            [&device](wgpu::RequestDeviceStatus, wgpu::Device d,
                                                      wgpu::StringView) { device = std::move(d); }),
                      UINT64_MAX));
        EXPECT_TRUE(device);
        this->device = device;
    }

  protected:
    wgpu::Instance instance;
    wgpu::Device device;
};

TEST_F(SpotTests, QuerySet) {
    // Spot test wgpuQuerySetGetType which uses indexOf on an int-to-string table.
    wgpu::QuerySetDescriptor querySetDesc{.type = wgpu::QueryType::Timestamp, .count = 1};
    wgpu::QuerySet querySet = device.CreateQuerySet(&querySetDesc);
    EXPECT_TRUE(querySet);
    EXPECT_EQ(querySet.GetType(), querySetDesc.type);
}

TEST_F(SpotTests, BufferGetMapState) {
    // Spot test one of the string-to-int tables (Int_BufferMapState) to make sure
    // that Closure's minification didn't minify its keys.
    wgpu::BufferDescriptor bufferDesc{.usage = wgpu::BufferUsage::CopyDst, .size = 4};
    wgpu::Buffer buffer = device.CreateBuffer(&bufferDesc);
    EXPECT_EQ(buffer.GetMapState(), wgpu::BufferMapState::Unmapped);
}

TEST_F(SpotTests, GetCompilationInfo) {
    for (bool valid : {true, false}) {
        wgpu::ShaderSourceWGSL wgslDesc{};
        wgslDesc.code = valid ? "" : "some invalid code";

        wgpu::ShaderModuleDescriptor descriptor{};
        descriptor.nextInChain = &wgslDesc;
        auto sm = device.CreateShaderModule(&descriptor);
        auto future = sm.GetCompilationInfo(
            wgpu::CallbackMode::WaitAnyOnly,
            [](wgpu::CompilationInfoRequestStatus, const wgpu::CompilationInfo* compilationInfo) {
                // We shouldn't have tried to allocate stuff if there were no messages.
                EXPECT_EQ(compilationInfo->messageCount == 0, compilationInfo->messages == nullptr);

                // After this, any compilation info will be freed. (There was a bug here which
                // this test catches, but only in ASAN builds.)
            });
        EXPECT_EQ(wgpu::WaitStatus::Success, instance.WaitAny(future, UINT64_MAX));
    }
}

}  // namespace
