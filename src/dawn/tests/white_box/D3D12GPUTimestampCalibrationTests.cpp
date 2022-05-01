// Copyright 2022 The Dawn Authors
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

#include <vector>

#include "dawn/native/Buffer.h"
#include "dawn/native/CommandEncoder.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/tests/DawnTest.h"
#include "dawn/utils/WGPUHelpers.h"

namespace dawn::native::d3d12 {
namespace {
class ExpectBetweenTimestamps : public ::detail::Expectation {
  public:
    ~ExpectBetweenTimestamps() override = default;

    ExpectBetweenTimestamps(uint64_t value0, uint64_t value1) {
        mValue0 = value0;
        mValue1 = value1;
    }

    // Expect the actual results are between mValue0 and mValue1.
    testing::AssertionResult Check(const void* data, size_t size) override {
        const uint64_t* actual = static_cast<const uint64_t*>(data);
        for (size_t i = 0; i < size / sizeof(uint64_t); ++i) {
            if (actual[i] < mValue0 || actual[i] > mValue1) {
                return testing::AssertionFailure()
                       << "Expected data[" << i << "] to be between " << mValue0 << " and "
                       << mValue1 << ", actual " << actual[i] << std::endl;
            }
        }

        return testing::AssertionSuccess();
    }

  private:
    uint64_t mValue0;
    uint64_t mValue1;
};

}  // anonymous namespace

class D3D12GPUTimestampCalibrationTests : public DawnTest {
  protected:
    void SetUp() override {
        DawnTest::SetUp();

        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        // Requires that timestamp query feature is enabled and timestamp query conversion is
        // disabled.
        DAWN_TEST_UNSUPPORTED_IF(!SupportsFeatures({wgpu::FeatureName::TimestampQuery}) ||
                                 !HasToggleEnabled("disable_timestamp_query_conversion"));
    }

    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::TimestampQuery})) {
            requiredFeatures.push_back(wgpu::FeatureName::TimestampQuery);
        }
        return requiredFeatures;
    }
};

// Check that the timestamps got by timestamp query are between the two timestamps from
// GetClockCalibration() after the timestamp conversion is disabled.
TEST_P(D3D12GPUTimestampCalibrationTests, TimestampsInOrder) {
    constexpr uint32_t kQueryCount = 2;

    wgpu::QuerySetDescriptor querySetDescriptor;
    querySetDescriptor.count = kQueryCount;
    querySetDescriptor.type = wgpu::QueryType::Timestamp;
    wgpu::QuerySet querySet = device.CreateQuerySet(&querySetDescriptor);

    wgpu::BufferDescriptor bufferDescriptor;
    bufferDescriptor.size = kQueryCount * sizeof(uint64_t);
    bufferDescriptor.usage =
        wgpu::BufferUsage::QueryResolve | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer destination = device.CreateBuffer(&bufferDescriptor);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.WriteTimestamp(querySet, 0);
    encoder.WriteTimestamp(querySet, 1);
    wgpu::CommandBuffer commands = encoder.Finish();

    Device* d3DDevice = reinterpret_cast<Device*>(device.Get());
    uint64_t gpuTimestamp0, gpuTimestamp1;
    uint64_t cpuTimestamp0, cpuTimestamp1;
    d3DDevice->GetCommandQueue()->GetClockCalibration(&gpuTimestamp0, &cpuTimestamp0);
    queue.Submit(1, &commands);
    WaitForAllOperations();
    d3DDevice->GetCommandQueue()->GetClockCalibration(&gpuTimestamp1, &cpuTimestamp1);

    // Separate resolve queryset to reduce the execution time of the queue with WriteTimestamp,
    // so that the timestamp in the querySet will be closer to both gpuTimestamps from
    // GetClockCalibration.
    wgpu::CommandEncoder resolveEncoder = device.CreateCommandEncoder();
    resolveEncoder.ResolveQuerySet(querySet, 0, kQueryCount, destination, 0);
    wgpu::CommandBuffer resolveCommands = resolveEncoder.Finish();
    queue.Submit(1, &resolveCommands);

    EXPECT_BUFFER(destination, 0, kQueryCount * sizeof(uint64_t),
                  new ExpectBetweenTimestamps(gpuTimestamp0, gpuTimestamp1));
}

DAWN_INSTANTIATE_TEST(D3D12GPUTimestampCalibrationTests,
                      D3D12Backend({"disable_timestamp_query_conversion"}));

}  // namespace dawn::native::d3d12
