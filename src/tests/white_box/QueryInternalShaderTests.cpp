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
                                              wgpu::Buffer input,
                                              wgpu::Buffer availability,
                                              wgpu::Buffer output,
                                              wgpu::Buffer params) {
        dawn_native::EncodeConvertTimestampsToNanoseconds(
            reinterpret_cast<dawn_native::CommandEncoder*>(encoder.Get()),
            reinterpret_cast<dawn_native::BufferBase*>(input.Get()),
            reinterpret_cast<dawn_native::BufferBase*>(availability.Get()),
            reinterpret_cast<dawn_native::BufferBase*>(output.Get()),
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
// - The input buffer passes the original timestamps resolved from query set (created by manual
//   here).
// - The availability buffer passes the data of which slot in input buffer is an initialized
//   timestamp.
// - The output buffer stores the converted results, expect 0 for unavailable timestamps and
//   nanoseconds for available timestamps in an expected error rate.
// - The params buffer passes the offset of input and output buffers, the count of timestamps and
//   the timestamp period (here use GPU frequency (HZ) on Intel D3D12 to calculate the period in
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
    constexpr uint64_t kOne = 1u;

    // Original timestamp values for testing
    std::array<uint64_t, kTimestampCount> timestamps;
    timestamps[0] = 0;            // not written at beginning
    timestamps[1] = 10079569507;  // t0
    timestamps[2] = 10394415012;  // t1
    timestamps[3] = 0;            // not written between timestamps
    timestamps[4] = 11713454943;  // t2
    timestamps[5] = 38912556941;  // t3 (big value)
    timestamps[6] = 10080295766;  // t4 (reset)
    timestamps[7] = 12159966783;  // t5 (after reset)
    timestamps[8] = 12651224612;  // t6
    timestamps[9] = 39872473956;  // t7

    // Expected results: Timestamp value * kNsPerSecond / kGPUFrequency
    std::array<uint64_t, kTimestampCount> expected;
    // The availablility state of each timestamp
    std::array<uint32_t, kTimestampCount> availabilities;

    for (size_t i = 0; i < kTimestampCount; i++) {
        if (timestamps[i] == 0) {
            // Not a timestamp value, keep original value
            expected[i] = 0u;
            availabilities[i] = 0u;
        } else {
            // Maybe the timestamp * 10^9 is larger than the maximum of uint64, so cast the delta
            // value to double (higher precision than float)
            expected[i] = static_cast<uint64_t>(static_cast<double>(timestamps[i]) * kNsPerSecond /
                                                kGPUFrequency);
            availabilities[i] = 1u;
        }
    }

    // The input storage buffer
    wgpu::Buffer inputBuffer =
        utils::CreateBufferFromData(device, timestamps.data(), sizeof(timestamps),
                                    wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc);
    EXPECT_BUFFER_U64_RANGE_EQ(timestamps.data(), inputBuffer, 0, kTimestampCount);

    // To indicate which value is available
    wgpu::Buffer availabilityBuffer = utils::CreateBufferFromData(
        device, availabilities.data(), sizeof(availabilities), wgpu::BufferUsage::Storage);

    // The output storage buffer
    wgpu::BufferDescriptor outputDesc;
    outputDesc.size = kTimestampCount * sizeof(uint64_t);
    outputDesc.usage =
        wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    wgpu::Buffer outputBuffer = device.CreateBuffer(&outputDesc);

    std::array<uint64_t, kTimestampCount> ones;
    ones.fill(kOne);

    // Convert timestamps to output buffer with offset 0
    {
        queue.WriteBuffer(outputBuffer, 0, ones.data(), sizeof(ones));

        constexpr uint32_t kOffset = 0u;
        // The params uniform buffer
        dawn_native::TimestampParams params = {kOffset, kOffset, kTimestampCount, kPeriod};
        wgpu::Buffer paramsBuffer = utils::CreateBufferFromData(device, &params, sizeof(params),
                                                                wgpu::BufferUsage::Uniform);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        EncodeConvertTimestampsToNanoseconds(encoder, inputBuffer, availabilityBuffer, outputBuffer,
                                             paramsBuffer);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER(outputBuffer, kOffset, kTimestampCount * sizeof(uint64_t),
                      new InternalShaderExpectation(expected.data(), kTimestampCount));
    }

    // Convert timestamps to output buffer with offset 8 from input buffer with offset 8
    {
        queue.WriteBuffer(outputBuffer, 0, ones.data(), sizeof(ones));

        constexpr uint32_t kOffset = 8u;
        // The params uniform buffer
        dawn_native::TimestampParams params = {kOffset, kOffset, kTimestampCount, kPeriod};
        wgpu::Buffer paramsBuffer = utils::CreateBufferFromData(device, &params, sizeof(params),
                                                                wgpu::BufferUsage::Uniform);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();

        EncodeConvertTimestampsToNanoseconds(encoder, inputBuffer, availabilityBuffer, outputBuffer,
                                             paramsBuffer);

        wgpu::CommandBuffer commands = encoder.Finish();
        queue.Submit(1, &commands);

        EXPECT_BUFFER_U64_RANGE_EQ(&kOne, outputBuffer, 0, 1);
        EXPECT_BUFFER(outputBuffer, kOffset, (kTimestampCount - 1) * sizeof(uint64_t),
                      new InternalShaderExpectation(expected.data() + 1, kTimestampCount - 1));
    }
}

DAWN_INSTANTIATE_TEST(QueryInternalShaderTests, D3D12Backend(), MetalBackend(), VulkanBackend());
