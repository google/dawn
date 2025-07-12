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

}  // namespace
