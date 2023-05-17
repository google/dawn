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

#include <memory>

#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/tests/white_box/GPUTimestampCalibrationTests.h"

namespace dawn {
namespace {

class GPUTimestampCalibrationTestsMetal : public GPUTimestampCalibrationTestBackend {
  public:
    explicit GPUTimestampCalibrationTestsMetal(const wgpu::Device& device) {
        mBackendDevice = dawn::native::metal::ToBackend(dawn::native::FromAPI(device.Get()));
    }

    // The API used in timestamp calibration is only available on macOS 10.15+ and iOS 14.0+
    bool IsSupported() const override {
        if (@available(macos 10.15, iOS 14.0, *)) {
            return true;
        }
        return false;
    }

    void GetTimestampCalibration(uint64_t* gpuTimestamp, uint64_t* cpuTimestamp) override {
        if (@available(macos 10.15, iOS 14.0, *)) {
            [mBackendDevice->GetMTLDevice() sampleTimestamps:cpuTimestamp
                                                gpuTimestamp:gpuTimestamp];
        }
    }

    float GetTimestampPeriod() const override { return mBackendDevice->GetTimestampPeriodInNS(); }

  private:
    dawn::native::metal::Device* mBackendDevice;
};

}  // anonymous namespace

// static
std::unique_ptr<GPUTimestampCalibrationTestBackend> GPUTimestampCalibrationTestBackend::Create(
    const wgpu::Device& device) {
    return std::make_unique<GPUTimestampCalibrationTestsMetal>(device);
}

}  // namespace dawn
