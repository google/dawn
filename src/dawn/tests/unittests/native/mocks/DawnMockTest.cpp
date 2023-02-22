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

#include "dawn/tests/unittests/native/mocks/DawnMockTest.h"

#include "dawn/dawn_proc.h"

namespace dawn::native {

DawnMockTest::DawnMockTest() {
    dawnProcSetProcs(&dawn::native::GetProcs());

    mDeviceMock = new ::testing::NiceMock<DeviceMock>();
    device = wgpu::Device::Acquire(ToAPI(mDeviceMock));
}

DawnMockTest::~DawnMockTest() {
    device = wgpu::Device();
    dawnProcSetProcs(nullptr);
}

}  // namespace dawn::native
