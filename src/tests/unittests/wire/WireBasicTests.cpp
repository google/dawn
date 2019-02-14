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
    WireBasicTests() : WireTest(true) {
    }
    ~WireBasicTests() override = default;
};

// One call gets forwarded correctly.
TEST_F(WireBasicTests, CallForwarded) {
    dawnDeviceCreateCommandBufferBuilder(device);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    EXPECT_CALL(api, CommandBufferBuilderRelease(apiCmdBufBuilder));
    FlushClient();
}

// Test that calling methods on a new object works as expected.
TEST_F(WireBasicTests, CreateThenCall) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);
    dawnCommandBufferBuilderGetResult(builder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    dawnCommandBuffer apiCmdBuf = api.GetNewCommandBuffer();
    EXPECT_CALL(api, CommandBufferBuilderGetResult(apiCmdBufBuilder)).WillOnce(Return(apiCmdBuf));

    EXPECT_CALL(api, CommandBufferBuilderRelease(apiCmdBufBuilder));
    EXPECT_CALL(api, CommandBufferRelease(apiCmdBuf));
    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireBasicTests, RefCountKeptInClient) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);

    dawnCommandBufferBuilderReference(builder);
    dawnCommandBufferBuilderRelease(builder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));
    EXPECT_CALL(api, CommandBufferBuilderRelease(apiCmdBufBuilder));

    FlushClient();
}

// Test that client reference/release do not call the backend API.
TEST_F(WireBasicTests, ReleaseCalledOnRefCount0) {
    dawnCommandBufferBuilder builder = dawnDeviceCreateCommandBufferBuilder(device);

    dawnCommandBufferBuilderRelease(builder);

    dawnCommandBufferBuilder apiCmdBufBuilder = api.GetNewCommandBufferBuilder();
    EXPECT_CALL(api, DeviceCreateCommandBufferBuilder(apiDevice))
        .WillOnce(Return(apiCmdBufBuilder));

    EXPECT_CALL(api, CommandBufferBuilderRelease(apiCmdBufBuilder));

    FlushClient();
}
