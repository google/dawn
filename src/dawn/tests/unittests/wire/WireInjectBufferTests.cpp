// Copyright 2024 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "dawn/tests/unittests/wire/WireTest.h"

#include "dawn/wire/WireClient.h"
#include "dawn/wire/WireServer.h"

namespace dawn::wire {
namespace {

using testing::Mock;
using testing::Return;

class WireInjectBufferTests : public WireTest {
  public:
    WireInjectBufferTests() {}
    ~WireInjectBufferTests() override = default;

    // A placeholder buffer format for ReserveBuffer. The data in it doesn't matter as long as
    // we don't call buffer reflection methods.
    WGPUBufferDescriptor placeholderDesc = {};
};

// Test that reserving and injecting a buffer makes calls on the client object forward to the
// server object correctly.
TEST_F(WireInjectBufferTests, CallAfterReserveInject) {
    auto reserved = GetWireClient()->ReserveBuffer(device, &placeholderDesc);

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    ASSERT_TRUE(GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));

    wgpuBufferDestroy(reserved.buffer);
    EXPECT_CALL(api, BufferDestroy(apiBuffer));
    FlushClient();
}

// Test that reserve correctly returns different IDs each time.
TEST_F(WireInjectBufferTests, ReserveDifferentIDs) {
    auto reserved1 = GetWireClient()->ReserveBuffer(device, &placeholderDesc);
    auto reserved2 = GetWireClient()->ReserveBuffer(device, &placeholderDesc);

    ASSERT_NE(reserved1.handle.id, reserved2.handle.id);
    ASSERT_NE(reserved1.buffer, reserved2.buffer);
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectBufferTests, InjectExistingID) {
    auto reserved = GetWireClient()->ReserveBuffer(device, &placeholderDesc);

    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    ASSERT_TRUE(GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));

    // ID already in use, call fails.
    ASSERT_FALSE(GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));
}

// Test that injecting the same id without a destroy first fails.
TEST_F(WireInjectBufferTests, ReuseIDAndGeneration) {
    // Do this loop multiple times since the first time, we can't test `generation - 1` since
    // generation == 0.
    ReservedBuffer reserved;
    WGPUBuffer apiBuffer = nullptr;
    for (int i = 0; i < 2; ++i) {
        reserved = GetWireClient()->ReserveBuffer(device, &placeholderDesc);

        apiBuffer = api.GetNewBuffer();
        EXPECT_CALL(api, BufferAddRef(apiBuffer));
        ASSERT_TRUE(
            GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));

        // Release the buffer. It should be possible to reuse the ID now, but not the generation
        wgpuBufferRelease(reserved.buffer);
        EXPECT_CALL(api, BufferRelease(apiBuffer));
        FlushClient();

        // Invalid to inject with the same ID and generation.
        ASSERT_FALSE(
            GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));
        if (i > 0) {
            EXPECT_GE(reserved.handle.generation, 1u);

            // Invalid to inject with the same ID and lesser generation.
            reserved.handle.generation -= 1;
            ASSERT_FALSE(
                GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));
        }
    }

    // Valid to inject with the same ID and greater generation.
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    reserved.handle.generation += 2;
    ASSERT_TRUE(GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));
}

// Test that the server only borrows the buffer and does a single reference-release
TEST_F(WireInjectBufferTests, InjectedBufferLifetime) {
    auto reserved = GetWireClient()->ReserveBuffer(device, &placeholderDesc);

    // Injecting the buffer adds a reference
    WGPUBuffer apiBuffer = api.GetNewBuffer();
    EXPECT_CALL(api, BufferAddRef(apiBuffer));
    ASSERT_TRUE(GetWireServer()->InjectBuffer(apiBuffer, reserved.handle, reserved.deviceHandle));

    // Releasing the buffer removes a single reference.
    wgpuBufferRelease(reserved.buffer);
    EXPECT_CALL(api, BufferRelease(apiBuffer));
    FlushClient();

    // Deleting the server doesn't release a second reference.
    DeleteServer();
    Mock::VerifyAndClearExpectations(&api);
}

// Test that a buffer reservation can be reclaimed. This is necessary to
// avoid leaking ObjectIDs for reservations that are never injected.
TEST_F(WireInjectBufferTests, ReclaimBufferReservation) {
    // Test that doing a reservation and full release is an error.
    {
        auto reserved = GetWireClient()->ReserveBuffer(device, &placeholderDesc);
        wgpuBufferRelease(reserved.buffer);
        FlushClient(false);
    }

    // Test that doing a reservation and then reclaiming it recycles the ID.
    {
        auto reserved1 = GetWireClient()->ReserveBuffer(device, &placeholderDesc);
        GetWireClient()->ReclaimBufferReservation(reserved1);

        auto reserved2 = GetWireClient()->ReserveBuffer(device, &placeholderDesc);

        // The ID is the same, but the generation is still different.
        ASSERT_EQ(reserved1.handle.id, reserved2.handle.id);
        ASSERT_NE(reserved1.handle.generation, reserved2.handle.generation);

        // No errors should occur.
        FlushClient();
    }
}

// Test the reflection of buffer creation parameters for reserved buffer.
TEST_F(WireInjectBufferTests, ReservedBufferReflection) {
    WGPUBufferDescriptor desc = {};
    desc.size = 10;
    desc.usage = WGPUBufferUsage_Storage;

    auto reserved = GetWireClient()->ReserveBuffer(device, &desc);
    WGPUBuffer buffer = reserved.buffer;

    ASSERT_EQ(desc.size, wgpuBufferGetSize(buffer));
    ASSERT_EQ(desc.usage, wgpuBufferGetUsage(buffer));
}

}  // anonymous namespace
}  // namespace dawn::wire
