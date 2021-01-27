// Copyright 2017 The Dawn Authors
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

#include "tests/unittests/validation/ValidationTest.h"

#include <gmock/gmock.h>

#include <memory>

using namespace testing;

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
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Error, _)).Times(1);

        ASSERT_DEVICE_ERROR(
            buffer.MapAsync(mode, offset, size, ToMockBufferMapAsyncCallback, nullptr));
    }

    wgpu::Queue queue;

  private:
    void SetUp() override {
        ValidationTest::SetUp();

        mockBufferMapAsyncCallback = std::make_unique<MockBufferMapAsyncCallback>();
        queue = device.GetDefaultQueue();
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
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
    }
    {
        wgpu::Buffer buffer = BufferMappedAtCreation(4, wgpu::BufferUsage::MapRead);
        AssertMapAsyncError(buffer, wgpu::MapMode::Read, 0, 4);
    }
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(4);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 4, nullptr, nullptr);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
    }
    {
        wgpu::Buffer buffer = BufferMappedAtCreation(4, wgpu::BufferUsage::MapWrite);
        AssertMapAsyncError(buffer, wgpu::MapMode::Write, 0, 4);
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
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, this + 0);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, this + 0))
            .Times(1);
        buf.Unmap();

        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this + 1))
            .Times(1);
        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, this + 0);

        EXPECT_CALL(*mockBufferMapAsyncCallback,
                    Call(WGPUBufferMapAsyncStatus_UnmappedBeforeCallback, this + 0))
            .Times(1);
        buf.Unmap();

        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, this + 1);
        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, this + 1))
            .Times(1);
        WaitForAllOperations(device);
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
            .WillOnce(InvokeWithoutArgs([&]() { buf.Unmap(); }));

        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&]() { buf.Unmap(); }));

        WaitForAllOperations(device);
    }
}

// Test that the MapCallback isn't fired twice when destroy() is called inside the callback
TEST_F(BufferValidationTest, MapAsync_DestroyCalledInCallback) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&]() { buf.Destroy(); }));

        WaitForAllOperations(device);
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, ToMockBufferMapAsyncCallback, nullptr);

        EXPECT_CALL(*mockBufferMapAsyncCallback, Call(WGPUBufferMapAsyncStatus_Success, _))
            .WillOnce(InvokeWithoutArgs([&]() { buf.Destroy(); }));

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

// Test that it is valid to destroy an unmapped buffer
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

// Test that it is invalid to Unmap a destroyed buffer
TEST_F(BufferValidationTest, UnmapDestroyedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.Unmap());
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        buf.Destroy();
        ASSERT_DEVICE_ERROR(buf.Unmap());
    }
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

// Test that a map usage is required to call Unmap
TEST_F(BufferValidationTest, UnmapWithoutMapUsage) {
    wgpu::BufferDescriptor descriptor;
    descriptor.size = 4;
    descriptor.usage = wgpu::BufferUsage::CopyDst;
    wgpu::Buffer buf = device.CreateBuffer(&descriptor);

    ASSERT_DEVICE_ERROR(buf.Unmap());
}

// Test that it is valid to call Unmap on a buffer that is not mapped
TEST_F(BufferValidationTest, UnmapUnmappedBuffer) {
    {
        wgpu::Buffer buf = CreateMapReadBuffer(4);
        // Buffer starts unmapped. Unmap should fail.
        ASSERT_DEVICE_ERROR(buf.Unmap());
        buf.MapAsync(wgpu::MapMode::Read, 0, 4, nullptr, nullptr);
        buf.Unmap();
        // Unmapping a second time should fail.
        ASSERT_DEVICE_ERROR(buf.Unmap());
    }
    {
        wgpu::Buffer buf = CreateMapWriteBuffer(4);
        // Buffer starts unmapped. Unmap should fail.
        ASSERT_DEVICE_ERROR(buf.Unmap());
        buf.MapAsync(wgpu::MapMode::Write, 0, 4, nullptr, nullptr);
        buf.Unmap();
        // Unmapping a second time should fail.
        ASSERT_DEVICE_ERROR(buf.Unmap());
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
    wgpu::BufferDescriptor desc;
    desc.size = 4;
    desc.usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead;

    uint64_t kStupidLarge = uint64_t(1) << uint64_t(63);

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

    // GetMappedRange after mappedAtCreation OOM case returns nullptr.
    {
        wgpu::Buffer buffer;
        ASSERT_DEVICE_ERROR(
            buffer = BufferMappedAtCreation(
                kStupidLarge, wgpu::BufferUsage::Storage | wgpu::BufferUsage::MapRead));

        ASSERT_EQ(buffer.GetConstMappedRange(), nullptr);
        ASSERT_EQ(buffer.GetConstMappedRange(), buffer.GetMappedRange());
    }
}

// Test validation of the GetMappedRange parameters
TEST_F(BufferValidationTest, GetMappedRange_OffsetSizeOOB) {
    // TODO(crbug.com/dawn/651): Fix failures on the wire.
    DAWN_SKIP_TEST_IF(UsesWire());

    // Valid case: full range is ok
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, nullptr, nullptr);
        EXPECT_NE(buffer.GetMappedRange(0, 8), nullptr);
    }

    // Valid case: full range is ok with defaulted MapAsync size
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 0, nullptr, nullptr);
        EXPECT_NE(buffer.GetMappedRange(0, 8), nullptr);
    }

    // Valid case: empty range at the end is ok
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, nullptr, nullptr);
        EXPECT_NE(buffer.GetMappedRange(8, 0), nullptr);
    }

    // Valid case: range in the middle is ok.
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(16);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 16, nullptr, nullptr);
        EXPECT_NE(buffer.GetMappedRange(8, 4), nullptr);
    }

    // Error case: offset is larger than the mapped range (even with size = 0)
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(8);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 8, nullptr, nullptr);
        EXPECT_EQ(buffer.GetMappedRange(9, 0), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(16, 0), nullptr);
    }

    // Error case: offset + size is larger than the mapped range
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 12, nullptr, nullptr);
        EXPECT_EQ(buffer.GetMappedRange(8, 5), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(8, 8), nullptr);
    }

    // Error case: offset + size is larger than the mapped range, overflow case
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 0, 12, nullptr, nullptr);
        EXPECT_EQ(buffer.GetMappedRange(8, std::numeric_limits<size_t>::max()), nullptr);
    }

    // Error case: offset is before the start of the range
    {
        wgpu::Buffer buffer = CreateMapWriteBuffer(12);
        buffer.MapAsync(wgpu::MapMode::Write, 8, 4, nullptr, nullptr);
        EXPECT_EQ(buffer.GetMappedRange(7, 4), nullptr);
        EXPECT_EQ(buffer.GetMappedRange(0, 4), nullptr);
    }
}
