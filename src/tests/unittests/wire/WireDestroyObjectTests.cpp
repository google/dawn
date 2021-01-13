// Copyright 2021 The Dawn Authors
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

#include "tests/unittests/wire/WireTest.h"

using namespace testing;
using namespace dawn_wire;

class WireDestroyObjectTests : public WireTest {};

// Test that destroying the device also destroys child objects.
TEST_F(WireDestroyObjectTests, DestroyDeviceDestroysChildren) {
    WGPUCommandEncoder encoder = wgpuDeviceCreateCommandEncoder(device, nullptr);

    WGPUCommandEncoder apiEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr)).WillOnce(Return(apiEncoder));

    FlushClient();

    // Release the device. It should cause the command encoder to be destroyed.
    wgpuDeviceRelease(device);

    Sequence s1, s2;
    // The device and child objects should be released.
    EXPECT_CALL(api, CommandEncoderRelease(apiEncoder)).InSequence(s1);
    EXPECT_CALL(api, QueueRelease(apiQueue)).InSequence(s2);
    EXPECT_CALL(api, DeviceRelease(apiDevice)).InSequence(s1, s2);

    FlushClient();

    // Using the command encoder should be an error.
    wgpuCommandEncoderFinish(encoder, nullptr);
    FlushClient(false);
}
