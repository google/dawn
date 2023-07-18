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

#ifndef DAWN_TESTS_BENCHMARKS_NULLDEVICESETUP
#define DAWN_TESTS_BENCHMARKS_NULLDEVICESETUP

#include <benchmark/benchmark.h>
#include <dawn/webgpu_cpp.h>
#include <condition_variable>
#include <mutex>

namespace wgpu {
struct DeviceDescriptor;
}  // namespace wgpu

namespace dawn {

class NullDeviceBenchmarkFixture : public benchmark::Fixture {
  public:
    void SetUp(const benchmark::State& state) override;
    void TearDown(const benchmark::State& state) override;

  protected:
    wgpu::Adapter adapter = nullptr;
    wgpu::Device device = nullptr;

  private:
    virtual wgpu::DeviceDescriptor GetDeviceDescriptor() const = 0;

    // Lock and conditional variable used to synchronize the benchmark global adapter/device.
    std::mutex mMutex;
    std::condition_variable mCv;
    int mNumDoneThreads = 0;
};

}  // namespace dawn

#endif  // DAWN_TESTS_BENCHMARKS_NULLDEVICESETUP
