// Copyright 2026 The Dawn & Tint Authors
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

#include <algorithm>
#include <vector>

#include "src/dawn/native/d3d12/BufferD3D12.h"
#include "src/dawn/native/d3d12/DeviceD3D12.h"
#include "src/dawn/tests/DawnTest.h"

namespace dawn::native::d3d12 {
namespace {

constexpr uint64_t kBufferSize = 64;

class D3D12BufferPersistentMappingTests : public DawnTest {
  protected:
    std::vector<wgpu::FeatureName> GetRequiredFeatures() override {
        std::vector<wgpu::FeatureName> requiredFeatures = {};
        if (SupportsFeatures({wgpu::FeatureName::BufferMapExtendedUsages})) {
            requiredFeatures.push_back(wgpu::FeatureName::BufferMapExtendedUsages);
        }
        // BufferMapWriteExtendedUsages is implied by BufferMapExtendedUsages, but some adapters
        // only support the former.
        if (SupportsFeatures({wgpu::FeatureName::BufferMapWriteExtendedUsages})) {
            requiredFeatures.push_back(wgpu::FeatureName::BufferMapWriteExtendedUsages);
        }
        return requiredFeatures;
    }

    void SetUp() override {
        DawnTest::SetUp();
        DAWN_TEST_UNSUPPORTED_IF(UsesWire());
        DAWN_TEST_UNSUPPORTED_IF(
            !HasToggleEnabled("d3d12_persistently_map_buffers_and_clear_buffer_before_destroy"));
    }

    wgpu::Buffer CreateBuffer(wgpu::BufferUsage usage, bool mappedAtCreation = false) {
        wgpu::BufferDescriptor descriptor;
        descriptor.size = kBufferSize;
        descriptor.usage = usage;
        descriptor.mappedAtCreation = mappedAtCreation;
        return device.CreateBuffer(&descriptor);
    }

    std::vector<uint8_t> MakeData(uint8_t seed) {
        std::vector<uint8_t> data(kBufferSize);
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = static_cast<uint8_t>(seed + i);
        }
        return data;
    }

    bool IsCommitted(const wgpu::Buffer& buffer) {
        return ToBackend(FromAPI(buffer.Get()))
            ->CheckAllocationMethodForTesting(AllocationMethod::kDirect);
    }

    ComPtr<ID3D12Resource> GetD3D12Resource(const wgpu::Buffer& buffer) {
        return ToBackend(FromAPI(buffer.Get()))->GetD3D12Resource();
    }

    // Waits until the deferred destruction of the buffers destroyed so far has completed. It is
    // scheduled at the pending command serial, which only completes after a later submit.
    void WaitForDeferredCleanup() {
        queue.Submit(0, nullptr);
        WaitForAllOperations();
    }

    // Returns `data` with the bytes that are zeroed before the native Unmap set to 0.
    static std::vector<uint8_t> WithZeroedPrefix(std::vector<uint8_t> data) {
        std::fill(data.begin(), data.begin() + kPersistentMappingBytesToZeroBeforeUnmap,
                  uint8_t{0});
        return data;
    }

    // Checks the content of `resource` by mapping it natively. Dawn must have released its own
    // mapping of `resource` already.
    static void ExpectResourceContent(ID3D12Resource* resource,
                                      const std::vector<uint8_t>& expected) {
        void* mappedPointer = nullptr;
        ASSERT_TRUE(SUCCEEDED(resource->Map(0, nullptr, &mappedPointer)));
        // SAFETY: Map returns the start of the resource, whose size is at least kBufferSize.
        Span<uint8_t> mappedData = DAWN_UNSAFE_BUFFERS(
            {static_cast<uint8_t*>(mappedPointer), static_cast<size_t>(kBufferSize)});
        EXPECT_TRUE(
            std::equal(expected.begin(), expected.end(), mappedData.begin(), mappedData.end()));
        // Apply the Intel workaround after checking the content and before native Unmap.
        std::ranges::fill(mappedData.first(kPersistentMappingBytesToZeroBeforeUnmap), uint8_t{0});
        D3D12_RANGE writtenRange = {0, kPersistentMappingBytesToZeroBeforeUnmap};
        resource->Unmap(0, &writtenRange);
    }
};

// Test that CPU-accessible buffers are allocated as committed resources.
TEST_P(D3D12BufferPersistentMappingTests, MappableBuffersAreCommitted) {
    EXPECT_TRUE(IsCommitted(CreateBuffer(wgpu::BufferUsage::MapWrite)));
    EXPECT_TRUE(
        IsCommitted(CreateBuffer(wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc)));
    EXPECT_TRUE(IsCommitted(CreateBuffer(wgpu::BufferUsage::MapRead)));
    EXPECT_TRUE(IsCommitted(CreateBuffer(wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst)));
}

// Test that the content written through several maps of the same buffer is kept.
TEST_P(D3D12BufferPersistentMappingTests, WriteAcrossMaps) {
    wgpu::Buffer buffer =
        CreateBuffer(wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, true);
    std::vector<uint8_t> data = MakeData(1);
    EXPECT_EQ(wgpu::Status::Success, buffer.WriteMappedRange(0, data.data(), data.size()));
    buffer.Unmap();
    EXPECT_BUFFER_U8_RANGE_EQ(data.data(), buffer, 0, data.size());

    // Write the second half of the buffer in a second map.
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, kBufferSize / 2, kBufferSize / 2);
    std::vector<uint8_t> newData = MakeData(100);
    EXPECT_EQ(wgpu::Status::Success,
              buffer.WriteMappedRange(kBufferSize / 2, newData.data(), kBufferSize / 2));
    buffer.Unmap();
    std::copy(newData.begin(), newData.begin() + kBufferSize / 2, data.begin() + kBufferSize / 2);
    EXPECT_BUFFER_U8_RANGE_EQ(data.data(), buffer, 0, data.size());
}

// Test that GPU writes are visible to the CPU in each map of the same buffer.
TEST_P(D3D12BufferPersistentMappingTests, ReadAcrossMaps) {
    wgpu::Buffer buffer = CreateBuffer(wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);

    for (uint8_t seed : {uint8_t{1}, uint8_t{100}}) {
        std::vector<uint8_t> data = MakeData(seed);
        queue.WriteBuffer(buffer, 0, data.data(), data.size());
        MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, kBufferSize);
        std::vector<uint8_t> readData(kBufferSize);
        EXPECT_EQ(wgpu::Status::Success,
                  buffer.ReadMappedRange(0, readData.data(), readData.size()));
        EXPECT_EQ(data, readData);
        buffer.Unmap();
    }
}

// Test that destroying a buffer right after submitting a copy from it doesn't change the copied
// data, and that the first bytes of the buffer are zeroed once the GPU has finished.
TEST_P(D3D12BufferPersistentMappingTests, DestroyAfterSubmit) {
    std::vector<uint8_t> data = MakeData(1);
    wgpu::Buffer source =
        CreateBuffer(wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, true);
    EXPECT_EQ(wgpu::Status::Success, source.WriteMappedRange(0, data.data(), data.size()));
    source.Unmap();

    wgpu::Buffer destination =
        CreateBuffer(wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(source, 0, destination, 0, kBufferSize);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);
    ComPtr<ID3D12Resource> sourceResource = GetD3D12Resource(source);
    source.Destroy();

    EXPECT_BUFFER_U8_RANGE_EQ(data.data(), destination, 0, data.size());

    WaitForDeferredCleanup();
    ExpectResourceContent(sourceResource.Get(), WithZeroedPrefix(data));
}

// Test that the first bytes of buffers mapped with MapAsync and then unmapped are zeroed after they
// are destroyed.
TEST_P(D3D12BufferPersistentMappingTests, DestroyAfterMapAsyncAndUnmap) {
    std::vector<uint8_t> data = MakeData(1);

    wgpu::Buffer writeBuffer = CreateBuffer(wgpu::BufferUsage::MapWrite);
    MapAsyncAndWait(writeBuffer, wgpu::MapMode::Write, 0, kBufferSize);
    EXPECT_EQ(wgpu::Status::Success, writeBuffer.WriteMappedRange(0, data.data(), data.size()));
    writeBuffer.Unmap();

    wgpu::Buffer readBuffer = CreateBuffer(wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);
    queue.WriteBuffer(readBuffer, 0, data.data(), data.size());
    MapAsyncAndWait(readBuffer, wgpu::MapMode::Read, 0, kBufferSize);
    readBuffer.Unmap();

    ComPtr<ID3D12Resource> writeResource = GetD3D12Resource(writeBuffer);
    ComPtr<ID3D12Resource> readResource = GetD3D12Resource(readBuffer);
    writeBuffer.Destroy();
    readBuffer.Destroy();

    WaitForDeferredCleanup();
    ExpectResourceContent(writeResource.Get(), WithZeroedPrefix(data));
    ExpectResourceContent(readResource.Get(), WithZeroedPrefix(data));
}

// Test that the first bytes of buffers destroyed while they are mapped are zeroed.
TEST_P(D3D12BufferPersistentMappingTests, DestroyWhileMapped) {
    std::vector<uint8_t> data = MakeData(1);

    wgpu::Buffer writeBuffer = CreateBuffer(wgpu::BufferUsage::MapWrite, true);
    EXPECT_EQ(wgpu::Status::Success, writeBuffer.WriteMappedRange(0, data.data(), data.size()));

    wgpu::Buffer readBuffer = CreateBuffer(wgpu::BufferUsage::MapRead | wgpu::BufferUsage::CopyDst);
    queue.WriteBuffer(readBuffer, 0, data.data(), data.size());
    MapAsyncAndWait(readBuffer, wgpu::MapMode::Read, 0, kBufferSize);

    ComPtr<ID3D12Resource> writeResource = GetD3D12Resource(writeBuffer);
    ComPtr<ID3D12Resource> readResource = GetD3D12Resource(readBuffer);
    writeBuffer.Destroy();
    readBuffer.Destroy();

    WaitForDeferredCleanup();
    ExpectResourceContent(writeResource.Get(), WithZeroedPrefix(data));
    ExpectResourceContent(readResource.Get(), WithZeroedPrefix(data));
}

// Test that the first bytes of buffers destroyed after the device is lost are zeroed.
TEST_P(D3D12BufferPersistentMappingTests, DestroyAfterDeviceLoss) {
    std::vector<uint8_t> data = MakeData(1);
    wgpu::Buffer mappedBuffer = CreateBuffer(wgpu::BufferUsage::MapWrite, true);
    EXPECT_EQ(wgpu::Status::Success, mappedBuffer.WriteMappedRange(0, data.data(), data.size()));
    wgpu::Buffer unmappedBuffer = CreateBuffer(wgpu::BufferUsage::MapWrite, true);
    EXPECT_EQ(wgpu::Status::Success, unmappedBuffer.WriteMappedRange(0, data.data(), data.size()));
    unmappedBuffer.Unmap();

    // Keep the native resources alive to inspect them after the device is destroyed.
    ComPtr<ID3D12Resource> mappedResource = GetD3D12Resource(mappedBuffer);
    ComPtr<ID3D12Resource> unmappedResource = GetD3D12Resource(unmappedBuffer);

    LoseDeviceForTesting();
    mappedBuffer.Destroy();
    unmappedBuffer.Destroy();

    // Destroying the device completes all the pending cleanup.
    device.Destroy();

    ExpectResourceContent(mappedResource.Get(), WithZeroedPrefix(data));
    ExpectResourceContent(unmappedResource.Get(), WithZeroedPrefix(data));
}

// Test that losing the device while the cleanup of a destroyed buffer is pending neither clears the
// buffer before the GPU has finished reading it, nor skips the clear.
TEST_P(D3D12BufferPersistentMappingTests, DeviceLossWhileCleanupIsPending) {
    std::vector<uint8_t> data = MakeData(1);
    wgpu::Buffer source =
        CreateBuffer(wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, true);
    EXPECT_EQ(wgpu::Status::Success, source.WriteMappedRange(0, data.data(), data.size()));
    source.Unmap();

    // The destination is CPU-readable so that its content can be checked after the device is lost.
    wgpu::Buffer destination =
        CreateBuffer(wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::MapRead);
    wgpu::CommandEncoder encoder = device.CreateCommandEncoder();
    encoder.CopyBufferToBuffer(source, 0, destination, 0, kBufferSize);
    wgpu::CommandBuffer commands = encoder.Finish();
    queue.Submit(1, &commands);

    // Keep the native resources alive to inspect them after the device is destroyed.
    ComPtr<ID3D12Resource> sourceResource = GetD3D12Resource(source);
    ComPtr<ID3D12Resource> destinationResource = GetD3D12Resource(destination);
    source.Destroy();

    // Destroying the device waits for the GPU and completes all the pending cleanup.
    LoseDeviceForTesting();
    device.Destroy();

    // The copy must have read the source before the source was cleared.
    ExpectResourceContent(destinationResource.Get(), data);
    ExpectResourceContent(sourceResource.Get(), WithZeroedPrefix(data));
}

// Test mappedAtCreation on a buffer that isn't CPU-accessible, which is written through a
// persistently mapped staging buffer.
TEST_P(D3D12BufferPersistentMappingTests, MappedAtCreationWithStagingBuffer) {
    std::vector<uint8_t> data = MakeData(1);

    // The copy from the staging buffer must read it before it is cleared.
    wgpu::Buffer buffer =
        CreateBuffer(wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst, true);
    EXPECT_EQ(wgpu::Status::Success, buffer.WriteMappedRange(0, data.data(), data.size()));
    buffer.Unmap();
    EXPECT_BUFFER_U8_RANGE_EQ(data.data(), buffer, 0, data.size());

    // Destroying the buffer while it is mapped releases the staging buffer without copying it.
    wgpu::Buffer destroyedBuffer =
        CreateBuffer(wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst, true);
    EXPECT_EQ(wgpu::Status::Success, destroyedBuffer.WriteMappedRange(0, data.data(), data.size()));
    destroyedBuffer.Destroy();
}

// Test that buffers with MapRead combined with other usages, which use the
// Custom_WriteBack_OnlyBuffers heap kind with BufferMapExtendedUsages, are committed, keep their
// content, and get their first bytes zeroed after they are destroyed.
TEST_P(D3D12BufferPersistentMappingTests, MapReadExtendedUsages) {
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::BufferMapExtendedUsages));
    std::vector<uint8_t> data = MakeData(1);

    wgpu::Buffer buffer = CreateBuffer(wgpu::BufferUsage::MapRead | wgpu::BufferUsage::Storage |
                                       wgpu::BufferUsage::CopyDst);
    EXPECT_TRUE(IsCommitted(buffer));
    queue.WriteBuffer(buffer, 0, data.data(), data.size());
    MapAsyncAndWait(buffer, wgpu::MapMode::Read, 0, kBufferSize);
    std::vector<uint8_t> readData(kBufferSize);
    EXPECT_EQ(wgpu::Status::Success, buffer.ReadMappedRange(0, readData.data(), readData.size()));
    EXPECT_EQ(data, readData);
    buffer.Unmap();

    ComPtr<ID3D12Resource> resource = GetD3D12Resource(buffer);
    buffer.Destroy();

    WaitForDeferredCleanup();
    ExpectResourceContent(resource.Get(), WithZeroedPrefix(data));
}

// Test that buffers with MapWrite combined with other usages, which use the
// Custom_WriteCombine_OnlyBuffers heap kind (or Custom_WriteBack_OnlyBuffers on cache-coherent UMA)
// with BufferMapWriteExtendedUsages, are committed, keep their content across maps, and get their
// first bytes zeroed after they are destroyed.
TEST_P(D3D12BufferPersistentMappingTests, MapWriteExtendedUsages) {
    DAWN_TEST_UNSUPPORTED_IF(!device.HasFeature(wgpu::FeatureName::BufferMapWriteExtendedUsages));
    std::vector<uint8_t> data = MakeData(1);

    wgpu::Buffer buffer = CreateBuffer(
        wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopySrc,
        true);
    EXPECT_TRUE(IsCommitted(buffer));
    EXPECT_EQ(wgpu::Status::Success, buffer.WriteMappedRange(0, data.data(), data.size()));
    buffer.Unmap();
    EXPECT_BUFFER_U8_RANGE_EQ(data.data(), buffer, 0, data.size());

    // Write the second half of the buffer in a second map.
    MapAsyncAndWait(buffer, wgpu::MapMode::Write, kBufferSize / 2, kBufferSize / 2);
    std::vector<uint8_t> newData = MakeData(100);
    EXPECT_EQ(wgpu::Status::Success,
              buffer.WriteMappedRange(kBufferSize / 2, newData.data(), kBufferSize / 2));
    buffer.Unmap();
    std::copy(newData.begin(), newData.begin() + kBufferSize / 2, data.begin() + kBufferSize / 2);
    // The expectation submits its copy immediately, so the buffer can be destroyed afterwards.
    EXPECT_BUFFER_U8_RANGE_EQ(data.data(), buffer, 0, data.size());

    ComPtr<ID3D12Resource> resource = GetD3D12Resource(buffer);
    buffer.Destroy();

    WaitForDeferredCleanup();
    ExpectResourceContent(resource.Get(), WithZeroedPrefix(data));
}

DAWN_INSTANTIATE_TEST(
    D3D12BufferPersistentMappingTests,
    D3D12Backend({"d3d12_persistently_map_buffers_and_clear_buffer_before_destroy"}));

}  // anonymous namespace
}  // namespace dawn::native::d3d12
