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

#include "tests/unittests/wire/WireTest.h"

using namespace testing;
using namespace dawn_wire;

class WireBasicTests : public WireTest {
  public:
    WireBasicTests() {
    }
    ~WireBasicTests() override = default;
};

// One call gets forwarded correctly.
TEST_F(WireBasicTests, CallForwarded) {
    dawnDeviceCreateCommandEncoder(device, nullptr);

    DawnCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));

    FlushClient();
}

// Test that calling methods on a new object works as expected.
TEST_F(WireBasicTests, CreateThenCall) {
    DawnCommandEncoder encoder = dawnDeviceCreateCommandEncoder(device, nullptr);
    dawnCommandEncoderFinish(encoder, nullptr);

    DawnCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));

    DawnCommandBuffer apiCmdBuf = api.GetNewCommandBuffer();
    EXPECT_CALL(api, CommandEncoderFinish(apiCmdBufEncoder, nullptr)).WillOnce(Return(apiCmdBuf));

    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireBasicTests, RefCountKeptInClient) {
    DawnCommandEncoder encoder = dawnDeviceCreateCommandEncoder(device, nullptr);

    dawnCommandEncoderReference(encoder);
    dawnCommandEncoderRelease(encoder);

    DawnCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));

    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireBasicTests, ReleaseCalledOnRefCount0) {
    DawnCommandEncoder encoder = dawnDeviceCreateCommandEncoder(device, nullptr);

    dawnCommandEncoderRelease(encoder);

    DawnCommandEncoder apiCmdBufEncoder = api.GetNewCommandEncoder();
    EXPECT_CALL(api, DeviceCreateCommandEncoder(apiDevice, nullptr))
        .WillOnce(Return(apiCmdBufEncoder));

    EXPECT_CALL(api, CommandEncoderRelease(apiCmdBufEncoder));

    FlushClient();
}
