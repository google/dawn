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

#include <dawn/webgpu_cpp.h>

namespace benchmark {
class State;
}

namespace wgpu {
struct DeviceDescriptor;
}

void SetupNullBackend(const benchmark::State& state);
wgpu::Device CreateNullDevice(const wgpu::DeviceDescriptor& desc);

#endif  // DAWN_TESTS_BENCHMARKS_NULLDEVICESETUP
