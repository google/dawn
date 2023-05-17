// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_TESTS_WHITE_BOX_GPUTIMESTAMPCALIBRATIONTESTS_H_
#define SRC_DAWN_TESTS_WHITE_BOX_GPUTIMESTAMPCALIBRATIONTESTS_H_

#include <memory>

namespace dawn {

class GPUTimestampCalibrationTestBackend {
  public:
    static std::unique_ptr<GPUTimestampCalibrationTestBackend> Create(const wgpu::Device& device);
    virtual ~GPUTimestampCalibrationTestBackend() = default;

    virtual bool IsSupported() const = 0;
    virtual void GetTimestampCalibration(uint64_t* gpuTimestamp, uint64_t* cpuTimestamp) = 0;
    virtual float GetTimestampPeriod() const = 0;
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_WHITE_BOX_GPUTIMESTAMPCALIBRATIONTESTS_H_
