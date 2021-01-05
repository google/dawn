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

#include "dawn_native/Buffer.h"
#include "dawn_native/CommandEncoder.h"
#include "dawn_native/QueryHelper.h"
#include "utils/WGPUHelpers.h"

namespace {

    void EncodeConvertTimestampsToNanoseconds(wgpu::CommandEncoder encoder,
                                              wgpu::Buffer timestamps,
                                              wgpu::Buffer availability,
                                              wgpu::Buffer params) {
        dawn_native::EncodeConvertTimestampsToNanoseconds(
            reinterpret_cast<dawn_native::CommandEncoder*>(encoder.Get()),
            reinterpret_cast<dawn_native::BufferBase*>(timestamps.Get()),
            reinterpret_cast<dawn_native::BufferBase*>(availability.Get()),
            reinterpret_cast<dawn_native::BufferBase*>(params.Get()));
    }

    class InternalShaderExpectation : public detail::Expectation {
      public:
        ~InternalShaderExpectation() override = default;

        InternalShaderExpectation(const uint64_t* values, const unsigned int count) {
            mExpected.assign(values, values + count);
        }

        // Expect the actual results are approximately equal to the expected values.
        testing::AssertionResult Check(const void* data, size_t size) override {
            DAWN_ASSERT(size == sizeof(uint64_t) * mExpected.size());
            constexpr static float kErrorToleranceRatio = 0.002f;

            const uint64_t* actual = static_cast<const uint64_t*>(data);
            for (size_t i = 0; i < mExpected.size(); ++i) {
                if (mExpected[i] == 0 && actual[i] != 0) {
                    return testing::AssertionFailure()
                           << "Expected data[" << i << "] to be 0, actual " << actual[i]
                           << std::endl;
                }

                if (abs(static_cast<int64_t>(mExpected[i] - actual[i])) >
                    mExpected[i] * kErrorToleranceRatio) {
                    return testing::AssertionFailure()
                           << "Expected data[" << i << "] to be " << mExpected[i] << ", actual "
                           << actual[i] << ". Error rate is larger than " << kErrorToleranceRatio
                           << std::endl;
                }
            }

            return testing::AssertionSuccess();
        }

      private:
        std::vector<uint64_t> mExpected;
    };

}  // anonymous namespace

class QueryInternalShaderTests : public DawnTest {};

// Test the accuracy of timestamp compute shader which uses unsigned 32-bit integers to simulate
// unsigned 64-bit integers (timestamps) multiplied by float (period).
// The arguments pass to timestamp internal pipeline:
// - The timestamps buffer contains the original timestamps resolved from query set (created
//   manually here), and will be used to store the results processed by the compute shader.
//   Expect 0 for unavailable timestamps and nanoseconds for available timestamps in an expected
//   error tolerance ratio.
// - The availability buffer passes the data of which slot in timestamps buffer is an initialized
//   timestamp.
// - The params buffer passes the timestamp count, the offset in timestamps buffer and the
//   timestamp period (here use GPU frequency (HZ) on Intel D3D12 to calculate the period in
//   ns for testing).
TEST_P(QueryInternalShaderTests, TimestampComputeShader) {
    DAWN_SKIP_TEST_IF(UsesWire());

    // TODO(crbug.com/tint/255, crbug.com/tint/256, crbug.com/tint/400, crbug.com/tint/417):
    // There is no builtin support for doing the runtime array.
    DAWN_SKIP_TEST_IF(HasToggleEnabled("use_tint_generator"));

    constexpr uint32_t kTimestampCount = 10u;
    // A gpu frequency on Intel D3D12 (ticks/second)
    constexpr uint64_t kGPUFrequency = 12000048u;
    constexpr uint64_t kNsPerSecond = 1000000000u;
    // Timestamp period in nanoseconds
    constexpr float kPeriod = static_cast<float>(kNsPerSecond) / kGPUFrequency;

    // Original timestamp values for testing
    std::vector<uint64_t> timestamps = {
        1,            // garbage data which is not written at beginning
        10079569507,  // t0
        10394415012,  // t1
        1,            // garbage data which is not written between timestamps
        11713454943,  // t2
        38912556941,  // t3 (big value)
        10080295766,  // t4 (reset)
        12159966783,  // t5 (after reset)
        12651224612,  // t6
        39872473956,  // t7
    };

    // The buffer indicating which values are available timestamps
    std::vector<uint32_t> availabilities = {0, 1, 1, 0, 1, 1, 1, 1, 1, 1};
    wgpu::Buffer availabilityBuffer =
        utils::CreateBufferFromData(device, availabilities.data(),
                                    kTimestampCount * sizeof(uint32_t), wgpu::BufferUsage::Storage);

    // The resolve buffer storing original timestamps and the converted values
    wgpu::BufferDescriptor timestampsDesc;
    timestampsDesc.size = kTimestampCount * sizeof(uint64_t);
    timestampsDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer timestampsBuffer = device.CreateBuffer(&timestampsDesc);

    auto PrepareExpectedResults = [&](uint32_t offset) -> std::vector<uint64_t> {
        ASSERT(offset % sizeof(uint64_t) == 0);
        std::vector<uint64_t> expected;
        for (size_t i = 0; i < kTimestampCount; i++) {
            // The data before offset remains as it is
            if (i < offset / sizeof(uint64_t)) {
                expected.push_back(timestamps[i]);
                continue;
            }

            if (availabilities[i] == 0) {
                // Not a available timestamp, write 0
                expected.push_back(0u);
            } else {
                // Maybe the timestamp * period is larger than the maximum of uint64, so cast the
                // delta value to double (higher precision than float)
                expected.push_back(
                    static_cast<uint64_t>(static_cast<double>(timestamps[i]) * kPeriod));
            }
        }
        return expected;
    };

    // Convert timestamps in timestamps buffer with offset 0
    {
        constexpr uint32_t kOffset = 0u;

        // Write orignal timestamps to timestamps buffer
        queue.WriteBuffer(timestampsBuffer, 0, timestamps.data(),
                          kTimestampCount * sizeof(uint64_t));

        // The params uniform buffer
        dawn_native::TimestampParams params = {kTimestampCount, kOffset, kPeriod};
        wgpu::Buffer paramsBuffer = utils::CreateBufferFromData(device, &params, sizeof(params),
                                                                wgpu::BufferUsage::Uniform);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeConvertTimestampsToNanoseconds(encoder, timestampsBuffer, availabilityBuffer,
                                             paramsBuffer);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Expected results: Timestamp * period
        std::vector<uint64_t> expected = PrepareExpectedResults(kOffset);
        EXPECT_BUFFER(timestampsBuffer, 0, kTimestampCount * sizeof(uint64_t),
                      new InternalShaderExpectation(expected.data(), kTimestampCount));
    }

    // Convert timestamps in timestamps buffer with offset 8
    {
        constexpr uint32_t kOffset = 8u;

        // Write orignal timestamps to timestamps buffer
        queue.WriteBuffer(timestampsBuffer, 0, timestamps.data(),
                          kTimestampCount * sizeof(uint64_t));

        // The params uniform buffer
        dawn_native::TimestampParams params = {kTimestampCount, kOffset, kPeriod};
        wgpu::Buffer paramsBuffer = utils::CreateBufferFromData(device, &params, sizeof(params),
                                                                wgpu::BufferUsage::Uniform);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        EncodeConvertTimestampsToNanoseconds(encoder, timestampsBuffer, availabilityBuffer,
                                             paramsBuffer);
        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        // Expected results: Timestamp * period
        std::vector<uint64_t> expected = PrepareExpectedResults(kOffset);
        EXPECT_BUFFER(timestampsBuffer, 0, kTimestampCount * sizeof(uint64_t),
                      new InternalShaderExpectation(expected.data(), kTimestampCount));
    }
}

DAWN_INSTANTIATE_TEST(QueryInternalShaderTests, D3D12Backend(), MetalBackend(), VulkanBackend());
