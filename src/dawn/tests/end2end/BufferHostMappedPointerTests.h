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

#ifndef SRC_DAWN_TESTS_END2END_BUFFERHOSTMAPPEDPOINTERTESTS_H_
#define SRC_DAWN_TESTS_END2END_BUFFERHOSTMAPPEDPOINTERTESTS_H_

#include <utility>
#include <vector>

#include "dawn/tests/DawnTest.h"

namespace dawn {

class BufferHostMappedPointerTestBackend {
  public:
    // The name used in gtest parameterization.
    virtual const char* Name() const = 0;

    std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(wgpu::Device device,
                                                          wgpu::BufferUsage usage,
                                                          size_t size);

    virtual std::pair<wgpu::Buffer, void*> CreateHostMappedBuffer(
        wgpu::Device device,
        wgpu::BufferUsage usage,
        size_t size,
        std::function<void(void*)> Populate) = 0;
};

inline std::ostream& operator<<(std::ostream& o, BufferHostMappedPointerTestBackend* backend) {
    o << backend->Name();
    return o;
}

using Backend = BufferHostMappedPointerTestBackend*;
DAWN_TEST_PARAM_STRUCT(BufferHostMappedPointerTestParams, Backend);

class BufferHostMappedPointerTests : public DawnTestWithParams<BufferHostMappedPointerTestParams> {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override;
    void SetUp() override;

    uint32_t mRequiredAlignment;
};

}  // namespace dawn

#endif  // SRC_DAWN_TESTS_END2END_BUFFERHOSTMAPPEDPOINTERTESTS_H_
