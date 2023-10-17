// Copyright 2017 The Dawn & Tint Authors
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

#include <limits>
#include <memory>

#include "dawn/common/Platform.h"
#include "dawn/tests/unittests/validation/ValidationTest.h"
#include "gmock/gmock.h"

using testing::_;
using testing::InvokeWithoutArgs;

class MockBufferMapAsyncCallback {
  public:
    MOCK_METHOD(void, Call, (WGPUBufferMapAsyncStatus status, void* userdata));
};

static std::unique_ptr<MockBufferMapAsyncCallback> mockBufferMapAsyncCallback;
static void ToMockBufferMapAsyncCallback(WGPUBufferMapAsyncStatus status, void* userdata) {
    mockBufferMapAsyncCallback->Call(status, userdata);
}

class BufferValidationTest : public ValidationTest {
  protected:
    wgpu::Buffer CreateMapReadBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::MapRead;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer CreateMapWriteBuffer(uint64_t size) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = wgpu::BufferUsage::MapWrite;

        return device.CreateBuffer(&descriptor);
    }

    wgpu::Buffer BufferMappedAtCreation(uint64_t size, wgpu::BufferUsage usage) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = size;
        descriptor.usage = usage;
        descriptor.mappedAtCreation = true;

        return device.CreateBuffer(&descriptor);
    }

    void AssertMapAsyncError(wgpu::Buffer buffer, wgpu::MapMode mode, size_t offset, size_t size) {
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _))
            .Times(1);

        ASSERT_DEVICE_ERROR(
            buffer.MapAsync(mode, offset, size, ToMockBufferMapAsyncCallback, nullptr));
    }

    wgpu::Queue queue;

  private:
    void SetUp() override {
        ValidationTest::SetUp();

        mockBufferMapAsyncCallback = std::make_unique<MockBufferMapAsyncCallback>();
        queue = device.GetQueue();
    }

    void TearDown() override {
        // Delete mocks so that expectations are checked
        mockBufferMapAsyncCallback = nullptr;

        ValidationTest::TearDown();
    }
};

// Test case where creation should succeed
TEST_F(BufferValidationTest, CreationSuccess) {
    // Success
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::Uniform;

        device.CreateBuffer(&descriptor);
    }
}

// Test case where creation should succeed
TEST_F(BufferValidationTest, CreationMaxBufferSize) {
    // Success when at limit
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = GetSupportedLimits().limits.maxBufferSize;
        descriptor.usage = wgpu::BufferUsage::Uniform;

        device.CreateBuffer(&descriptor);
    }
    // Error once it is pass the (default) limit on the device. (Note that MaxLimitTests tests at
    // max possible limit given the adapters.)
    {
        wgpu::BufferDescriptor descriptor;
        ASSERT_TRUE(GetSupportedLimits().limits.maxBufferSize <
                    std::numeric_limits<uint32_t>::max());
        descriptor.size = GetSupportedLimits().limits.maxBufferSize + 1;
        descriptor.usage = wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

// Test restriction on usages must not be None (0)
TEST_F(BufferValidationTest, CreationMapUsageNotZero) {
    // Zero (None) usage is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::None;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

// Test restriction on usages allowed with MapRead and MapWrite
TEST_F(BufferValidationTest, CreationMapUsageRestrictions) {
    // MapRead with CopyDst is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst;

        device.CreateBuffer(&descriptor);
    }

    // MapRead with something else is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }

    // MapWrite with CopySrc is ok
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;

        device.CreateBuffer(&descriptor);
    }

    // MapWrite with something else is an error
    {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = 4;
        descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Uniform;

        ASSERT_DEVICE_ERROR(device.CreateBuffer(&descriptor));
    }
}

// Test the success case for mapping buffer for reading
TEST_F(BufferValidationTest, MapAsync_ReadSuccess) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

    EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);
    WaitForAllOperations(device);

    buf.Unmap();
}

// Test the success case for mapping buffer for writing
TEST_F(BufferValidationTest, MapAsync_WriteSuccess) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);

    buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

    EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);
    WaitForAllOperations(device);

    buf.Unmap();
}

// Test map async with a buffer that's an error
TEST_F(BufferValidationTest, MapAsync_ErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buffer;
    ASSERT_DEVICE_ERROR(buffer = device.CreateBuffer(&desc));

    AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
    AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
}

// Test map async with an invalid offset and size alignment.
TEST_F(BufferValidationTest, MapAsync_OffsetSizeAlignment) {
    // Control case, offset aligned to 8 and size to 4 is valid
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Read, 8, 4, nullptr, nullptr);
    }
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 8, 4, nullptr, nullptr);
    }

    // Error case, offset aligned to 4 is an error.
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(12);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 4, 4);
    }
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 4, 4);
    }

    // Error case, size aligned to 2 is an error.
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(8);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 6);
    }
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 6);
    }
}

// Test map async with an invalid offset and size OOB checks
TEST_F(BufferValidationTest, MapAsync_OffsetSizeOOB) {
    // Valid case: full buffer is ok.
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Read, 0, 8, nullptr, nullptr);
    }

    // Valid case: range in the middle of the buffer is ok.
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(16);
        buffer.MapAsync(wgpu::MapMode::Read, 8, 4, nullptr, nullptr);
    }

    // Valid case: empty range at the end of the buffer is ok.
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Read, 8, 0, nullptr, nullptr);
    }

    // Error case, offset is larger than the buffer size (even if size is 0).
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(12);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 16, 0);
    }

    // Error case, offset + size is larger than the buffer
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(12);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 8, 8);
    }

    // Error case, offset + size is larger than the buffer, overflow case.
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(12);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 8,
                            std::numeric_limits<size_t>::max() & ~size_t(7));
    }
}

// Test map async with a buffer that has the wrong usage
TEST_F(BufferValidationTest, MapAsync_WrongUsage) {
    {
        wgpu::BufferDescriptor desc;
        desc.usage = wgpu::BufferUsage::Vertex;
        desc.size = 4;
        wgpu::Buffer buffer = device.CreateBuffer(&desc);

        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(4);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(4);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
    }
}

// Test map async with a wrong mode
TEST_F(BufferValidationTest, MapAsync_WrongMode) {
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(4);
        AssertMapAsyncError(buffer, wgpu::MapMode::None, 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(4);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read | wgpu::MapMode::Write, 0, 4);
    }
}

// Test map async with a buffer that's already mapped
TEST_F(BufferValidationTest, MapAsync_AlreadyMapped) {
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(4);
        buffer.MapAsync(wgpu::MapMode::Read, 0, 4, nullptr, nullptr);
        WaitForAllOperations(device);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
    }
    {
        wgpu::Buffer buffer = BufferMappedAtCreation(4, wgpu::BufferUsage::MapRead);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(4);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 4, nullptr, nullptr);
        WaitForAllOperations(device);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
    }
    {
        wgpu::Buffer buffer = BufferMappedAtCreation(4, wgpu::BufferUsage::MapWrite);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
    }
}

// Test MapAsync() immediately causes a pending map error
TEST_F(BufferValidationTest, MapAsync_PendingMap) {
    // Read + overlapping range
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(4);
        // The first map async call should succeed while the second one should fail
        buffer.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, this);
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, this + 1))
            .Times(1);
        buffer.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this))
            .Times(1);
        WaitForAllOperations(device);
    }

    // Read + non-overlapping range
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(16);
        // The first map async call should succeed while the second one should fail
        buffer.MapAsync(wgpu::MapMode::Read, 0, 8, ToMockBufferMapAsyncCallback, this);
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, this + 1))
            .Times(1);
        buffer.MapAsync(wgpu::MapMode::Read, 8, 8, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this))
            .Times(1);
        WaitForAllOperations(device);
    }

    // Write + overlapping range
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(4);
        // The first map async call should succeed while the second one should fail
        buffer.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, this);
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, this + 1))
            .Times(1);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this))
            .Times(1);
        WaitForAllOperations(device);
    }

    // Write + non-overlapping range
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(16);
        // The first map async call should succeed while the second one should fail
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, ToMockBufferMapAsyncCallback, this);
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_MappingAlreadyPending, this + 1))
            .Times(1);
        buffer.MapAsync(wgpu::MapMode::Write, 8, 8, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this))
            .Times(1);
        WaitForAllOperations(device);
    }
}

// Test map async with a buffer that's destroyed
TEST_F(BufferValidationTest, MapAsync_Destroy) {
    {
        wgpu::Buffer buffer = CreateMapReadBuffer(4);
        buffer.Destroy();
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(4);
        buffer.Destroy();
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
    }
}

// Test map async but unmapping before the result is ready.
TEST_F(BufferValidationTest, MapAsync_UnmapBeforeResult) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
            .Times(1);
        buf.Unmap();

        // The callback shouldn't be called again.
        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
            .Times(1);
        buf.Unmap();

        // The callback shouldn't be called again.
        WaitForAllOperations(device);
    }
}

// When a MapAsync is cancelled with Unmap it might still be in flight, test doing a new request
// works as expected and we don't get the cancelled request's data.
TEST_F(BufferValidationTest, MapAsync_UnmapBeforeResultAndMapAgain) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(16);
        buf.MapAsync(wgpu::MapMode::Read, 0, 8, ToMockBufferMapAsyncCallback, this + 0);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, this + 0))
            .Times(1);
        buf.Unmap();

        buf.MapAsync(wgpu::MapMode::Read, 8, 8, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this + 1))
            .Times(1);
        WaitForAllOperations(device);

        // Check that only the second MapAsync had an effect
        ASSERT_EQ(nullptr, buf.GetConstMappedRange(0));
        ASSERT_NE(nullptr, buf.GetConstMappedRange(8));
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(16);
        buf.MapAsync(wgpu::MapMode::Write, 0, 8, ToMockBufferMapAsyncCallback, this + 0);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, this + 0))
            .Times(1);
        buf.Unmap();

        buf.MapAsync(wgpu::MapMode::Write, 8, 8, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this + 1))
            .Times(1);
        WaitForAllOperations(device);

        // Check that only the second MapAsync had an effect
        ASSERT_EQ(nullptr, buf.GetConstMappedRange(0));
        ASSERT_NE(nullptr, buf.GetConstMappedRange(8));
    }
}

// Test map async but destroying before the result is ready.
TEST_F(BufferValidationTest, MapAsync_DestroyBeforeResult) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
            .Times(1);
        buf.Destroy();

        // The callback shouldn't be called again.
        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
            .Times(1);
        buf.Destroy();

        // The callback shouldn't be called again.
        WaitForAllOperations(device);
    }
}

// Test that the MapCallback isn't fired twice when unmap() is called inside the callback
TEST_F(BufferValidationTest, MapAsync_UnmapCalledInCallback) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&] { buf.Unmap(); }));

        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&] { buf.Unmap(); }));

        WaitForAllOperations(device);
    }
}

// Test that the MapCallback isn't fired twice when destroy() is called inside the callback
TEST_F(BufferValidationTest, MapAsync_DestroyCalledInCallback) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&] { buf.Destroy(); }));

        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&] { buf.Destroy(); }));

        WaitForAllOperations(device);
    }
}

// Test MapAsync call in MapAsync success callback
// This test is disabled now because there seems to be a reeantrancy bug in the
// FlushWire call. See https://dawn-review.googlesource.com/c/dawn/+/116220 for the details.
TEST_F(BufferValidationTest, DISABLED_MapAsync_MapAsyncInMapAsyncSuccessCallback) {
    // Test MapAsync call in MapAsync validation success callback
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&] {
                EXPECT_CALL(*mockBufferMapAsyncCallback,
                            Call(WGPUBufferMapAsyncStatus_ValidationError, _));
                // Should cause validation error because of already mapped buffer
                ASSERT_DEVICE_ERROR(
                    buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr));
            }));

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        WaitForAllOperations(device);
        // we need another wire flush to make the MapAsync in the callback to the server
        WaitForAllOperations(device);
    }
}

// Test MapAsync call in MapAsync rejection callback
TEST_F(BufferValidationTest, MapAsync_MapAsyncInMapAsyncRejectionCallback) {
    // Test MapAsync call in MapAsync validation error callback
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_ValidationError, _))
            .WillOnce(InvokeWithoutArgs([&] {
                // Retry with valid parameter and it should succeed
                EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _));
                buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
            }));

        // Write map mode on read buffer is invalid and it should reject with validation error
        ASSERT_DEVICE_ERROR(
            buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr));

        WaitForAllOperations(device);
        // we need another wire flush to make the MapAsync in the callback to the server
        WaitForAllOperations(device);
    }

    // Test MapAsync call in MapAsync Unmapped before callback callback
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
            .WillOnce(InvokeWithoutArgs([&] {
                // MapAsync call on unmapped buffer should be valid
                EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _));
                buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
            }));

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        buf.Unmap();
        WaitForAllOperations(device);
        WaitForAllOperations(device);
    }

    // Test MapAsync call in MapAsync Destroyed before callback callback
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
            .WillOnce(InvokeWithoutArgs([&] {
                // MapAsync call on destroyed buffer should be invalid
                EXPECT_CALL(*mockBufferMapAsyncCallback,
                            Call(WGPUBufferMapAsyncStatus_ValidationError, _));
                ASSERT_DEVICE_ERROR(
                    buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr));
            }));

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        buf.Destroy();
        WaitForAllOperations(device);
        WaitForAllOperations(device);
    }
}

// Test the success case for mappedAtCreation
TEST_F(BufferValidationTest, MappedAtCreationSuccess) {
    BufferMappedAtCreation(4, wgpu::BufferUsage::MapWrite);
}

// Test the success case for mappedAtCreation for a non-mappable usage
TEST_F(BufferValidationTest, NonMappableMappedAtCreationSuccess) {
    BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
}

// Test there is an error when mappedAtCreation is set but the size isn't aligned to 4.
TEST_F(BufferValidationTest, MappedAtCreationSizeAlignment) {
    ASSERT_DEVICE_ERROR(BufferMappedAtCreation(2, wgpu::BufferUsage::MapWrite));
}

// Test that it is valid to destroy an error buffer
TEST_F(BufferValidationTest, DestroyErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    buf.Destroy();
}

// Test that it is valid to Destroy an unmapped buffer
TEST_F(BufferValidationTest, DestroyUnmappedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
    }
}

// Test that it is valid to Destroy a destroyed buffer
TEST_F(BufferValidationTest, DestroyDestroyedBuffer) {
    wgpu::Buffer buf = CreateMapWriteBuffer(4);
    buf.Destroy();
    buf.Destroy();
}

// Test that it is valid to Unmap an error buffer
TEST_F(BufferValidationTest, UnmapErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    buf.Unmap();
}

// Test that it is valid to Unmap a destroyed buffer
TEST_F(BufferValidationTest, UnmapDestroyedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
        buf.Unmap();
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
        buf.Unmap();
    }
}

// Test that unmap then mapping a destroyed buffer is an error.
// Regression test for crbug.com/1388920.
TEST_F(BufferValidationTest, MapDestroyedBufferAfterUnmap) {
    wgpu::Buffer buffer = CreateMapReadBuffer(4);
    buffer.Destroy();
    buffer.Unmap();

    ASSERT_DEVICE_ERROR(buffer.MapAsync(
        wgpu::MapMode::Read, 0, wgpu::kWholeMapSize,
        [](WGPUBufferMapAsyncStatus status, void* userdata) {
            EXPECT_EQ(WGPUBufferMapAsyncStatus_ValidationError, status);
        },
        nullptr));
    WaitForAllOperations(device);
}

// Test that it is valid to submit a buffer in a queue with a map usage if it is unmapped
TEST_F(BufferValidationTest, SubmitBufferWithMapUsage) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;

    wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
    wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
}

// Test that it is invalid to submit a mapped buffer in a queue
TEST_F(BufferValidationTest, SubmitMappedBuffer) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead;
    {
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        bufA.MapAsync(wgpu::MapMode::Write, 0, 4, nullptr, nullptr);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        bufB.MapAsync(wgpu::MapMode::Read, 0, 4, nullptr, nullptr);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations(device);
    }
    {
        wgpu::BufferDescriptor mappedBufferDesc = descriptorA;
        mappedBufferDesc.mappedAtCreation = true;
        wgpu::Buffer bufA = device.CreateBuffer(&mappedBufferDesc);
        wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations(device);
    }
    {
        wgpu::BufferDescriptor mappedBufferDesc = descriptorB;
        mappedBufferDesc.mappedAtCreation = true;
        wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
        wgpu::Buffer bufB = device.CreateBuffer(&mappedBufferDesc);

        wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
        encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
        wgpu::CommandBuffer commands = encoder.Finish();
        ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
        WaitForAllOperations(device);
    }
}

// Test that it is invalid to submit a destroyed buffer in a queue
TEST_F(BufferValidationTest, SubmitDestroyedBuffer) {
    wgpu::BufferDescriptor descriptorA;
    descriptorA.size = 4;
    descriptorA.usage = wgpu::BufferUsage::CopySrc;

    wgpu::BufferDescriptor descriptorB;
    descriptorB.size = 4;
    descriptorB.usage = wgpu::BufferUsage::CopyDst;

    wgpu::Buffer bufA = device.CreateBuffer(&descriptorA);
    wgpu::Buffer bufB = device.CreateBuffer(&descriptorB);

    bufA.Destroy();
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(bufA, 0, bufB, 0, 4);
    wgpu::CommandBuffer commands = encoder.Finish();
    ASSERT_DEVICE_ERROR(queue.Submit(1, &commands));
}

// Test that a map usage is not required to call Unmap
TEST_F(BufferValidationTest, UnmapWithoutMapUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buf = device.CreateBuffer(&descriptor);

    buf.Unmap();
}

// Test that it is valid to call Unmap on a buffer that is not mapped
TEST_F(BufferValidationTest, UnmapUnmappedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        // Buffer starts unmapped. Unmap shouldn't fail.
        buf.Unmap();
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, nullptr, nullptr);
        buf.Unmap();
        // Unmapping a second time shouldn't fail.
        buf.Unmap();
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        // Buffer starts unmapped. Unmap shouldn't fail.
        buf.Unmap();
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, nullptr, nullptr);
        buf.Unmap();
        // Unmapping a second time shouldn't fail.
        buf.Unmap();
    }
}

// Test that it is invalid to call GetMappedRange on an unmapped buffer.
TEST_F(BufferValidationTest, GetMappedRange_OnUnmappedBuffer) {
    // Unmapped at creation case.
    {
        wgpu::BufferDescriptor desc;
        desc.size = 4;
        desc.usage = wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buf = device.CreateBuffer(&desc);

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Unmapped after mappedAtCreation case.
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        buf.Unmap();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Unmapped after MapAsync read case.
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        WaitForAllOperations(device);
        buf.Unmap();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Unmapped after MapAsync write case.
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        WaitForAllOperations(device);
        buf.Unmap();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }
}

// Test that it is invalid to call GetMappedRange on a destroyed buffer.
TEST_F(BufferValidationTest, GetMappedRange_OnDestroyedBuffer) {
    // Destroyed after creation case.
    {
        wgpu::BufferDescriptor desc;
        desc.size = 4;
        desc.usage = wgpu::BufferUsage::CopySrc;
        wgpu::Buffer buf = device.CreateBuffer(&desc);
        buf.Destroy();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Destroyed after mappedAtCreation case.
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        buf.Destroy();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Destroyed after MapAsync read case.
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        WaitForAllOperations(device);
        buf.Destroy();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }

    // Destroyed after MapAsync write case.
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        WaitForAllOperations(device);
        buf.Destroy();

        ASSERT_EQ(nullptr, buf.GetMappedRange());
        ASSERT_EQ(nullptr, buf.GetConstMappedRange());
    }
}

// Test that it is invalid to call GetMappedRange on a buffer after MapAsync for reading
TEST_F(BufferValidationTest, GetMappedRange_NonConstOnMappedForReading) {
    wgpu::Buffer buf = CreateMapReadBuffer(4);

    buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
    EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _)).Times(1);
    WaitForAllOperations(device);

    ASSERT_EQ(nullptr, buf.GetMappedRange());
}

// Test valid cases to call GetMappedRange on a buffer.
TEST_F(BufferValidationTest, GetMappedRange_ValidBufferStateCases) {
    // GetMappedRange after mappedAtCreation case.
    {
        wgpu::Buffer buffer = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }

    // GetMappedRange after MapAsync for reading case.
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, nullptr, nullptr);
        WaitForAllOperations(device);

        ASSERT_NE(buf.GetConstMappedRange(), nullptr);
    }

    // GetMappedRange after MapAsync for writing case.
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);

        buf.MapAsync(wgpu::MapMode::Write, 0, 4, nullptr, nullptr);
        WaitForAllOperations(device);

        ASSERT_NE(buf.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buf.GetConstMappedRange(), buf.GetMappedRange());
    }
}

// Test valid cases to call GetMappedRange on an error buffer.
TEST_F(BufferValidationTest, GetMappedRange_OnErrorBuffer) {
    // GetMappedRange after mappedAtCreation a zero-sized buffer returns a non-nullptr.
    // This is to check we don't do a malloc(0).
    {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(buffer = BufferMappedAtCreation(
                                0, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead));

        ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }

    // GetMappedRange after mappedAtCreation non-OOM returns a non-nullptr.
    {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(buffer = BufferMappedAtCreation(
                                4, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead));

        ASSERT_NE(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }
}

// Test valid cases to call GetMappedRange on an error buffer that's also OOM.
TEST_F(BufferValidationTest, GetMappedRange_OnErrorBuffer_OOM) {
    // TODO(crbug.com/dawn/1506): new (std::nothrow) crashes on OOM on Mac ARM64 because libunwind
    // doesn't see the previous catchall try-catch.
    DAWN_SKIP_TEST_IF(DAWN_PLATFORM_IS(MACOS) && DAWN_PLATFORM_IS(ARM64));

    uint64_t kStupidLarge = uint64_t(1) << uint64_t(63);

    if (UsesWire()) {
        wgpu::Buffer buffer = BufferMappedAtCreation(
            kStupidLarge, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead);
        ASSERT_EQ(nullptr, buffer.Get());
    } else {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(
            buffer = BufferMappedAtCreation(
                kStupidLarge, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead));

        // GetMappedRange after mappedAtCreation OOM case returns nullptr.
        ASSERT_EQ(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }
}

// Test validation of the GetMappedRange parameters
TEST_F(BufferValidationTest, GetMappedRange_OffsetSizeOOB) {
    // Valid case: full range is ok
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_NE(buffer.GetMappedRange(0, 8), nullptr);
    }

    // Valid case: full range is ok with defaulted MapAsync size
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, wgpu::kWholeMapSize, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_NE(buffer.GetMappedRange(0, 8), nullptr);
    }

    // Valid case: full range is ok with defaulted MapAsync size and defaulted GetMappedRangeSize
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, wgpu::kWholeMapSize, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_NE(buffer.GetMappedRange(0, wgpu::kWholeMapSize), nullptr);
    }

    // Valid case: empty range at the end is ok
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_NE(buffer.GetMappedRange(8, 0), nullptr);
    }

    // Valid case: range in the middle is ok.
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(16);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 16, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_NE(buffer.GetMappedRange(8, 4), nullptr);
    }

    // Error case: offset is larger than the mapped range (even with size = 0)
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_EQ(buffer.GetMappedRange(9, 0), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(16, 0), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(std::numeric_limits<size_t>::max(), 0), nullptr);
    }

    // Error case: offset is larger than the buffer size (even with size = 0)
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(16);
        buffer.MapAsync(wgpu::MapMode::Write, 8, 8, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_EQ(buffer.GetMappedRange(16, 4), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(24, 0), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(std::numeric_limits<size_t>::max(), 0), nullptr);
    }

    // Error case: offset + size is larger than the mapped range
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 12, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_EQ(buffer.GetMappedRange(8, 5), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(8, 8), nullptr);
    }

    // Error case: offset + size is larger than the mapped range, overflow case
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 12, nullptr, nullptr);
        WaitForAllOperations(device);
        // set size to (max - 1) to avoid being equal to kWholeMapSize
        EXPECT_EQ(buffer.GetMappedRange(8, std::numeric_limits<size_t>::max() - 1), nullptr);
    }

    // Error case: size is larger than the mapped range when using default kWholeMapSize
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_EQ(buffer.GetMappedRange(0), nullptr);
    }

    // Error case: offset is before the start of the range (even with size = 0)
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 8, 4, nullptr, nullptr);
        WaitForAllOperations(device);
        EXPECT_EQ(buffer.GetMappedRange(7, 4), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(0, 4), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(0, 12), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(0, 0), nullptr);
    }
}

// Test that the buffer creation parameters are correctly reflected for succesfully created buffers.
TEST_F(BufferValidationTest, CreationParameterReflectionForValidBuffer) {
    // Test reflection on two succesfully created but different buffers. The reflected data should
    // be different!
    {
        wgpu::BufferDescriptor desc;
        desc.size = 16;
        desc.usage = wgpu::BufferUsage::Uniform;
        wgpu::Buffer buf = device.CreateBuffer(&desc);

        EXPECT_EQ(wgpu::BufferUsage::Uniform, buf.GetUsage());
        EXPECT_EQ(16u, buf.GetSize());
    }
    {
        wgpu::BufferDescriptor desc;
        desc.size = 32;
        desc.usage = wgpu::BufferUsage::Storage;
        wgpu::Buffer buf = device.CreateBuffer(&desc);

        EXPECT_EQ(wgpu::BufferUsage::Storage, buf.GetUsage());
        EXPECT_EQ(32u, buf.GetSize());
    }
}

// Test that the buffer creation parameters are correctly reflected for buffers invalid because of
// validation errors.
TEST_F(BufferValidationTest, CreationParameterReflectionForErrorBuffer) {
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::Uniform;
    desc.size = 19;
    desc.mappedAtCreation = true;

    // Error! MappedAtCreation requires size % 4 == 0.
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    // Reflection data is still exactly what was in the descriptor.
    EXPECT_EQ(wgpu::BufferUsage::Uniform, buf.GetUsage());
    EXPECT_EQ(19u, buf.GetSize());
}

// Test that the buffer creation parameters are correctly reflected for buffers invalid because of
// OOM.
TEST_F(BufferValidationTest, CreationParameterReflectionForOOMBuffer) {
    constexpr uint64_t kAmazinglyLargeSize = 0x1234'5678'90AB'CDEF;
    wgpu::BufferDescriptor desc;
    desc.usage = wgpu::BufferUsage::Storage;
    desc.size = kAmazinglyLargeSize;

    // OOM!
    wgpu::Buffer buf;
    ASSERT_DEVICE_ERROR(buf = device.CreateBuffer(&desc));

    // Reflection data is still exactly what was in the descriptor.
    EXPECT_EQ(wgpu::BufferUsage::Storage, buf.GetUsage());
    EXPECT_EQ(kAmazinglyLargeSize, buf.GetSize());
}

// Test that buffer reflection doesn't show internal usages
TEST_F(BufferValidationTest, CreationParameterReflectionNoInternalUsage) {
    wgpu::BufferDescriptor desc;
    desc.size = 16;
    // QueryResolve also adds kInternalStorageBuffer for processing of queries.
    desc.usage = wgpu::BufferUsage::QueryResolve;
    wgpu::Buffer buf = device.CreateBuffer(&desc);

    // The reflection shouldn't show kInternalStorageBuffer
    EXPECT_EQ(wgpu::BufferUsage::QueryResolve, buf.GetUsage());
    EXPECT_EQ(16u, buf.GetSize());
}

// Test that GetMapState() shows expected buffer map state
TEST_F(BufferValidationTest, GetMapState) {
    // MapRead + MapAsync + Unmap
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapRead + MapAsync + Unmap before the callback
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapRead + MapAsync + Destroy
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapRead + MapAsync + Destroy before the callback
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapWrite + MapAsync + Unmap
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapWrite + MapAsync + Unmap before the callback
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapWrite + MapAsync + Destroy
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MapWrite + MapAsync + Destroy before the callback
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_DestroyedBeforeCallback, _))
            .Times(1);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);
        EXPECT_EQ(wgpu::BufferMapState::Pending, buf.GetMapState());
        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
        WaitForAllOperations(device);
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MappedAtCreation + Unmap
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Unmap();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }

    // MappedAtCreation + Destroy
    {
        wgpu::Buffer buf = BufferMappedAtCreation(4, wgpu::BufferUsage::CopySrc);
        EXPECT_EQ(wgpu::BufferMapState::Mapped, buf.GetMapState());
        buf.Destroy();
        EXPECT_EQ(wgpu::BufferMapState::Unmapped, buf.GetMapState());
    }
}
