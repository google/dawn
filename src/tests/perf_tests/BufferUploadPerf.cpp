// Copyright 2019 The Dawn Authors
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

#include "tests/perf_tests/DawnPerfTest.h"

#include "tests/ParamGenerator.h"
#include "utils/DawnHelpers.h"

namespace {

    constexpr unsigned int kNumIterations = 50;
    constexpr uint32_t kBufferSize = 1024 * 1024;

    enum class UploadMethod {
        SetSubData,
        CreateBufferMapped,
    };

    struct BufferUploadParams : DawnTestParam {
        BufferUploadParams(const DawnTestParam& param, UploadMethod uploadMethod)
            : DawnTestParam(param), uploadMethod(uploadMethod) {
        }

        UploadMethod uploadMethod;
    };

    std::ostream& operator<<(std::ostream& ostream, const BufferUploadParams& param) {
        ostream << static_cast<const DawnTestParam&>(param);

        switch (param.uploadMethod) {
            case UploadMethod::SetSubData:
                ostream << "_SetSubData";
                break;
            case UploadMethod::CreateBufferMapped:
                ostream << "_CreateBufferMapped";
                break;
        }
        return ostream;
    }

}  // namespace

// Test uploading |kBufferSize| bytes of data |kNumIterations| times.
class BufferUploadPerf : public DawnPerfTestWithParams<BufferUploadParams> {
  public:
    BufferUploadPerf() : DawnPerfTestWithParams(kNumIterations), data(kBufferSize) {
    }
    ~BufferUploadPerf() override = default;

    void SetUp() override;

  private:
    void Step() override;

    dawn::Buffer dst;
    std::vector<uint8_t> data;
};

void BufferUploadPerf::SetUp() {
    DawnPerfTestWithParams<BufferUploadParams>::SetUp();

    dawn::BufferDescriptor desc = {};
    desc.size = kBufferSize;
    desc.usage = dawn::BufferUsage::CopyDst;

    dst = device.CreateBuffer(&desc);
}

void BufferUploadPerf::Step() {
    switch (GetParam().uploadMethod) {
        case UploadMethod::SetSubData: {
            for (unsigned int i = 0; i < kNumIterations; ++i) {
                dst.SetSubData(0, kBufferSize, data.data());
            }
            // Make sure all SetSubData's are flushed.
            queue.Submit(0, nullptr);
        } break;

        case UploadMethod::CreateBufferMapped: {
            dawn::BufferDescriptor desc = {};
            desc.size = kBufferSize;
            desc.usage = dawn::BufferUsage::CopySrc | dawn::BufferUsage::MapWrite;

            dawn::CommandEncoder encoder = device.CreateCommandEncoder();

            for (unsigned int i = 0; i < kNumIterations; ++i) {
                auto result = device.CreateBufferMapped(&desc);
                memcpy(result.data, data.data(), kBufferSize);
                result.buffer.Unmap();
                encoder.CopyBufferToBuffer(result.buffer, 0, dst, 0, kBufferSize);
            }

            dawn::CommandBuffer commands = encoder.Finish();
            queue.Submit(1, &commands);
        } break;
    }

    // Wait for the GPU in this batch of iterations.
    // If we don't wait, we can't properly compute the number of steps to run during
    // calibration.
    // The wait time gets amortized over the kNumIterations.
    WaitForGPU();
}

TEST_P(BufferUploadPerf, Run) {
    RunTest();
}

DAWN_INSTANTIATE_PERF_TEST_SUITE_P(BufferUploadPerf,
                                   {D3D12Backend, MetalBackend, OpenGLBackend, VulkanBackend},
                                   {UploadMethod::SetSubData, UploadMethod::CreateBufferMapped});
