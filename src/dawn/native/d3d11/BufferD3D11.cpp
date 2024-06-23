// Copyright 2023 The Dawn & Tint Authors
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

#include "dawn/native/d3d11/BufferD3D11.h"

#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

#include "dawn/common/Alloc.h"
#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/ChainUtils.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/QueueD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {

class ScopedCommandRecordingContext;

namespace {

constexpr wgpu::BufferUsage kD3D11GPUOnlyUniformBufferUsages =
    wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;

constexpr wgpu::BufferUsage kCopyUsages = wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;

constexpr wgpu::BufferUsage kStagingUsages = kMappableBufferUsages | kCopyUsages;

// Resource usage    Default    Dynamic   Immutable   Staging
// ------------------------------------------------------------
//  GPU-read         Yes        Yes       Yes         Yes[1]
//  GPU-write        Yes        No        No          Yes[1]
//  CPU-read         No         No        No          Yes[1]
//  CPU-write        No         Yes       No          Yes[1]
// ------------------------------------------------------------
// [1] GPU read or write of a resource with the D3D11_USAGE_STAGING usage is restricted to copy
// operations. You use ID3D11DeviceContext::CopySubresourceRegion and
// ID3D11DeviceContext::CopyResource for these copy operations.

bool IsMappable(wgpu::BufferUsage usage) {
    return usage & kMappableBufferUsages;
}

bool IsUpload(wgpu::BufferUsage usage) {
    return usage == (wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::MapWrite);
}

bool IsStaging(wgpu::BufferUsage usage) {
    // Must have at least MapWrite or MapRead bit
    return IsMappable(usage) && IsSubset(usage, kStagingUsages);
}

UINT D3D11BufferBindFlags(wgpu::BufferUsage usage) {
    UINT bindFlags = 0;

    if (usage & (wgpu::BufferUsage::Vertex)) {
        bindFlags |= D3D11_BIND_VERTEX_BUFFER;
    }
    if (usage & wgpu::BufferUsage::Index) {
        bindFlags |= D3D11_BIND_INDEX_BUFFER;
    }
    if (usage & (wgpu::BufferUsage::Uniform)) {
        bindFlags |= D3D11_BIND_CONSTANT_BUFFER;
    }
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        DAWN_ASSERT(!IsMappable(usage));
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }
    if (usage & kReadOnlyStorageBuffer) {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    // If the buffer only has CopySrc and CopyDst usages are used as staging buffers for copy.
    // Because D3D11 doesn't allow copying between buffer and texture, we will use compute shader
    // to copy data between buffer and texture. So the buffer needs to be bound as unordered access
    // view.
    if (IsSubset(usage, kCopyUsages)) {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    return bindFlags;
}

UINT D3D11BufferMiscFlags(wgpu::BufferUsage usage) {
    UINT miscFlags = 0;
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer | kReadOnlyStorageBuffer)) {
        miscFlags |= D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS;
    }
    if (usage & wgpu::BufferUsage::Indirect) {
        miscFlags |= D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
    }
    return miscFlags;
}

size_t D3D11BufferSizeAlignment(wgpu::BufferUsage usage) {
    if (usage & wgpu::BufferUsage::Uniform) {
        // https://learn.microsoft.com/en-us/windows/win32/api/d3d11_1/nf-d3d11_1-id3d11devicecontext1-vssetconstantbuffers1
        // Each number of constants must be a multiple of 16 shader constants(sizeof(float) * 4 *
        // 16).
        return sizeof(float) * 4 * 16;
    }

    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        // Unordered access buffers must be 4-byte aligned.
        return sizeof(uint32_t);
    }
    return 1;
}

constexpr size_t kConstantBufferUpdateAlignment = 16;

}  // namespace

// For CPU-to-GPU upload buffers(CopySrc|MapWrite), they can be emulated in the system memory, and
// then written into the dest GPU buffer via ID3D11DeviceContext::UpdateSubresource.
class UploadBuffer final : public Buffer {
  public:
    UploadBuffer(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor)
        : Buffer(device,
                 descriptor,
                 /*internalMappableFlags=*/kMappableBufferUsages) {}
    ~UploadBuffer() override = default;

  private:
    MaybeError InitializeInternal() override {
        mUploadData = std::unique_ptr<uint8_t[]>(AllocNoThrow<uint8_t>(GetAllocatedSize()));
        if (mUploadData == nullptr) {
            return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate memory for buffer uploading.");
        }
        return {};
    }

    MaybeError MapInternal(const ScopedCommandRecordingContext* commandContext,
                           wgpu::MapMode) override {
        mMappedData = mUploadData.get();
        return {};
    }

    void UnmapInternal(const ScopedCommandRecordingContext* commandContext) override {
        mMappedData = nullptr;
    }

    MaybeError ClearInternal(const ScopedCommandRecordingContext* commandContext,
                             uint8_t clearValue,
                             uint64_t offset,
                             uint64_t size) override {
        memset(mUploadData.get() + offset, clearValue, size);
        return {};
    }

    MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                              uint64_t sourceOffset,
                              size_t size,
                              Buffer* destination,
                              uint64_t destinationOffset) override {
        return destination->WriteInternal(commandContext, destinationOffset,
                                          mUploadData.get() + sourceOffset, size);
    }

    MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                   ID3D11Buffer* srcD3D11Buffer,
                                   uint64_t sourceOffset,
                                   size_t size,
                                   uint64_t destinationOffset) override {
        // Upload buffers shouldn't be copied to.
        DAWN_UNREACHABLE();
        return {};
    }

    MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                             uint64_t offset,
                             const void* data,
                             size_t size) override {
        const auto* src = static_cast<const uint8_t*>(data);
        std::copy(src, src + size, mUploadData.get() + offset);
        return {};
    }

    std::unique_ptr<uint8_t[]> mUploadData;
};

// Buffer that supports mapping and copying.
class StagingBuffer final : public Buffer {
  public:
    StagingBuffer(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor)
        : Buffer(device, descriptor, /*internalMappableFlags=*/kMappableBufferUsages) {}

  private:
    void DestroyImpl() override {
        // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
        // - It may be called if the buffer is explicitly destroyed with APIDestroy.
        //   This case is NOT thread-safe and needs proper synchronization with other
        //   simultaneous uses of the buffer.
        // - It may be called when the last ref to the buffer is dropped and the buffer
        //   is implicitly destroyed. This case is thread-safe because there are no
        //   other threads using the buffer since there are no other live refs.
        Buffer::DestroyImpl();

        mD3d11Buffer = nullptr;
    }

    void SetLabelImpl() override {
        SetDebugName(ToBackend(GetDevice()), mD3d11Buffer.Get(), "Dawn_StagingBuffer", GetLabel());
    }

    MaybeError InitializeInternal() override {
        DAWN_ASSERT(IsStaging(GetUsage()));

        D3D11_BUFFER_DESC bufferDescriptor;
        bufferDescriptor.ByteWidth = mAllocatedSize;
        bufferDescriptor.Usage = D3D11_USAGE_STAGING;
        bufferDescriptor.BindFlags = 0;
        // D3D11 doesn't allow copying between buffer and texture.
        //  - For buffer to texture copy, we need to use a staging(mappable) texture, and memcpy the
        //    data from the staging buffer to the staging texture first. So D3D11_CPU_ACCESS_READ is
        //    needed for MapWrite usage.
        //  - For texture to buffer copy, we may need copy texture to a staging (mappable)
        //    texture, and then memcpy the data from the staging texture to the staging buffer. So
        //    D3D11_CPU_ACCESS_WRITE is needed to MapRead usage.
        bufferDescriptor.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
        bufferDescriptor.MiscFlags = 0;
        bufferDescriptor.StructureByteStride = 0;

        DAWN_TRY(
            CheckOutOfMemoryHRESULT(ToBackend(GetDevice())
                                        ->GetD3D11Device()
                                        ->CreateBuffer(&bufferDescriptor, nullptr, &mD3d11Buffer),
                                    "ID3D11Device::CreateBuffer"));

        return {};
    }

    MaybeError MapInternal(const ScopedCommandRecordingContext* commandContext,
                           wgpu::MapMode) override {
        DAWN_ASSERT(IsMappable(GetUsage()));
        DAWN_ASSERT(!mMappedData);

        // Always map buffer with D3D11_MAP_READ_WRITE even for mapping wgpu::MapMode:Read, because
        // we need write permission to initialize the buffer.
        // TODO(dawn:1705): investigate the performance impact of mapping with D3D11_MAP_READ_WRITE.
        D3D11_MAPPED_SUBRESOURCE mappedResource;
        DAWN_TRY(CheckHRESULT(commandContext->Map(mD3d11Buffer.Get(),
                                                  /*Subresource=*/0, D3D11_MAP_READ_WRITE,
                                                  /*MapFlags=*/0, &mappedResource),
                              "ID3D11DeviceContext::Map"));
        mMappedData = static_cast<uint8_t*>(mappedResource.pData);

        return {};
    }

    void UnmapInternal(const ScopedCommandRecordingContext* commandContext) override {
        DAWN_ASSERT(mMappedData);
        commandContext->Unmap(mD3d11Buffer.Get(),
                              /*Subresource=*/0);
        mMappedData = nullptr;
    }

    MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                              uint64_t sourceOffset,
                              size_t size,
                              Buffer* destination,
                              uint64_t destinationOffset) override {
        return destination->CopyFromD3DInternal(commandContext, mD3d11Buffer.Get(), sourceOffset,
                                                size, destinationOffset);
    }

    MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                   ID3D11Buffer* d3d11SourceBuffer,
                                   uint64_t sourceOffset,
                                   size_t size,
                                   uint64_t destinationOffset) override {
        D3D11_BOX srcBox;
        srcBox.left = static_cast<UINT>(sourceOffset);
        srcBox.top = 0;
        srcBox.front = 0;
        srcBox.right = static_cast<UINT>(sourceOffset + size);
        srcBox.bottom = 1;
        srcBox.back = 1;

        DAWN_ASSERT(d3d11SourceBuffer);

        commandContext->CopySubresourceRegion(mD3d11Buffer.Get(), /*DstSubresource=*/0,
                                              /*DstX=*/destinationOffset,
                                              /*DstY=*/0,
                                              /*DstZ=*/0, d3d11SourceBuffer, /*SrcSubresource=*/0,
                                              &srcBox);

        return {};
    }

    MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                             uint64_t offset,
                             const void* data,
                             size_t size) override {
        if (size == 0) {
            return {};
        }

        ScopedMap scopedMap;
        DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(commandContext, this, wgpu::MapMode::Write));

        DAWN_ASSERT(scopedMap.GetMappedData());
        memcpy(scopedMap.GetMappedData() + offset, data, size);

        return {};
    }

    ComPtr<ID3D11Buffer> mD3d11Buffer;
};

// Buffer that can only be written/read by GPU.
class GPUOnlyBuffer final : public GPUUsableBuffer {
  public:
    GPUOnlyBuffer(DeviceBase* device, const UnpackedPtr<BufferDescriptor>& descriptor)
        : GPUUsableBuffer(device, descriptor, /*internalMappableFlags=*/wgpu::BufferUsage::None) {}

    ResultOrError<ID3D11Buffer*> GetD3D11ConstantBuffer(
        const ScopedCommandRecordingContext* commandContext) override;
    ResultOrError<ID3D11Buffer*> GetD3D11NonConstantBuffer(
        const ScopedCommandRecordingContext* commandContext) override;

    ResultOrError<ComPtr<ID3D11ShaderResourceView>> UseAsSRV(
        const ScopedCommandRecordingContext* commandContext,
        uint64_t offset,
        uint64_t size) override;
    ResultOrError<ComPtr<ID3D11UnorderedAccessView1>> UseAsUAV(
        const ScopedCommandRecordingContext* commandContext,
        uint64_t offset,
        uint64_t size) override;

    MaybeError PredicatedClear(const ScopedSwapStateCommandRecordingContext* commandContext,
                               ID3D11Predicate* predicate,
                               uint8_t clearValue,
                               uint64_t offset,
                               uint64_t size) override;

  private:
    // Dawn API
    void DestroyImpl() override;
    void SetLabelImpl() override;

    MaybeError InitializeInternal() override;

    MaybeError CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                              uint64_t sourceOffset,
                              size_t size,
                              Buffer* destination,
                              uint64_t destinationOffset) override;
    MaybeError CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                   ID3D11Buffer* srcD3D11Buffer,
                                   uint64_t sourceOffset,
                                   size_t size,
                                   uint64_t destinationOffset) override;

    MaybeError WriteInternal(const ScopedCommandRecordingContext* commandContext,
                             uint64_t bufferOffset,
                             const void* data,
                             size_t size) override;

    MaybeError ClearPaddingInternal(const ScopedCommandRecordingContext* commandContext) override;

    // The buffer object for constant buffer usage.
    ComPtr<ID3D11Buffer> mD3d11ConstantBuffer;
    // The buffer object for non-constant buffer usages(e.g. storage buffer, vertex buffer, etc.)
    ComPtr<ID3D11Buffer> mD3d11NonConstantBuffer;

    bool mConstantBufferIsUpdated = true;
};

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device,
                                          const UnpackedPtr<BufferDescriptor>& descriptor,
                                          const ScopedCommandRecordingContext* commandContext,
                                          bool allowUploadBufferEmulation) {
    bool useUploadBuffer = allowUploadBufferEmulation;
    useUploadBuffer &= IsUpload(descriptor->usage);
    constexpr uint64_t kMaxUploadBufferSize = 4 * 1024 * 1024;
    useUploadBuffer &= descriptor->size <= kMaxUploadBufferSize;
    Ref<Buffer> buffer;
    if (useUploadBuffer) {
        buffer = AcquireRef(new UploadBuffer(device, descriptor));
    } else if (IsStaging(descriptor->usage)) {
        buffer = AcquireRef(new StagingBuffer(device, descriptor));
    } else {
        buffer = AcquireRef(new GPUOnlyBuffer(device, descriptor));
    }
    DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation, commandContext));
    return buffer;
}

Buffer::Buffer(DeviceBase* device,
               const UnpackedPtr<BufferDescriptor>& descriptor,
               wgpu::BufferUsage internalMappableFlags)
    : BufferBase(device, descriptor), mInternalMappableFlags(internalMappableFlags) {}

MaybeError Buffer::Initialize(bool mappedAtCreation,
                              const ScopedCommandRecordingContext* commandContext) {
    // TODO(dawn:1705): handle mappedAtCreation for NonzeroClearResourcesOnCreationForTesting

    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    uint64_t size = std::max(GetSize(), uint64_t(4u));
    // The validation layer requires:
    // ByteWidth must be 12 or larger to be used with D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS.
    if (GetUsage() & wgpu::BufferUsage::Indirect) {
        size = std::max(size, uint64_t(12u));
    }
    size_t alignment = D3D11BufferSizeAlignment(GetUsage());
    // Check for overflow, bufferDescriptor.ByteWidth is a UINT.
    if (size > std::numeric_limits<UINT>::max() - alignment) {
        // Alignment would overlow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    mAllocatedSize = Align(size, alignment);

    DAWN_TRY(InitializeInternal());

    SetLabelImpl();

    if (!mappedAtCreation) {
        if (commandContext) {
            DAWN_TRY(ClearInitialResource(commandContext));
        } else {
            auto tmpCommandContext =
                ToBackend(GetDevice()->GetQueue())
                    ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
            DAWN_TRY(ClearInitialResource(&tmpCommandContext));
        }
    }

    return {};
}

MaybeError Buffer::ClearInitialResource(const ScopedCommandRecordingContext* commandContext) {
    if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
        DAWN_TRY(ClearWholeBuffer(commandContext, 1u));
    }

    // Initialize the padding bytes to zero.
    if (GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
        DAWN_TRY(ClearPaddingInternal(commandContext));
    }
    return {};
}

Buffer::~Buffer() = default;

bool Buffer::IsCPUWritableAtCreation() const {
    return IsCPUWritable();
}

bool Buffer::IsCPUWritable() const {
    return mInternalMappableFlags & wgpu::BufferUsage::MapWrite;
}

bool Buffer::IsCPUReadable() const {
    return mInternalMappableFlags & wgpu::BufferUsage::MapRead;
}

MaybeError Buffer::MapAtCreationImpl() {
    DAWN_ASSERT(IsCPUWritable());
    auto commandContext = ToBackend(GetDevice()->GetQueue())
                              ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
    return MapInternal(&commandContext, wgpu::MapMode::Write);
}

MaybeError Buffer::MapInternal(const ScopedCommandRecordingContext* commandContext,
                               wgpu::MapMode mode) {
    DAWN_UNREACHABLE();

    return {};
}

void Buffer::UnmapInternal(const ScopedCommandRecordingContext* commandContext) {
    DAWN_UNREACHABLE();
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    DAWN_ASSERT((mode == wgpu::MapMode::Write && IsCPUWritable()) ||
                (mode == wgpu::MapMode::Read && IsCPUReadable()));

    mMapReadySerial = mLastUsageSerial;
    const ExecutionSerial completedSerial = GetDevice()->GetQueue()->GetCompletedCommandSerial();
    // We may run into map stall in case that the buffer is still being used by previous submitted
    // commands. To avoid that, instead we ask Queue to do the map later when mLastUsageSerial has
    // passed.
    if (mMapReadySerial > completedSerial) {
        ToBackend(GetDevice()->GetQueue())->TrackPendingMapBuffer({this}, mode, mMapReadySerial);
    } else {
        auto commandContext = ToBackend(GetDevice()->GetQueue())
                                  ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
        DAWN_TRY(FinalizeMap(&commandContext, completedSerial, mode));
    }

    return {};
}

MaybeError Buffer::FinalizeMap(ScopedCommandRecordingContext* commandContext,
                               ExecutionSerial completedSerial,
                               wgpu::MapMode mode) {
    // Needn't map the buffer if this is for a previous mapAsync that was cancelled.
    if (completedSerial >= mMapReadySerial) {
        // TODO(dawn:1705): make sure the map call is not blocked by the GPU operations.
        DAWN_TRY(MapInternal(commandContext, mode));

        DAWN_TRY(EnsureDataInitialized(commandContext));
    }

    return {};
}

void Buffer::UnmapImpl() {
    DAWN_ASSERT(IsMappable(GetUsage()));
    mMapReadySerial = kMaxExecutionSerial;
    if (mMappedData) {
        auto commandContext = ToBackend(GetDevice()->GetQueue())
                                  ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
        UnmapInternal(&commandContext);
    }
}

void* Buffer::GetMappedPointer() {
    // The frontend asks that the pointer returned is from the start of the resource
    // irrespective of the offset passed in MapAsyncImpl, which is what mMappedData is.
    return mMappedData;
}

void Buffer::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    BufferBase::DestroyImpl();
    if (mMappedData) {
        UnmapImpl();
    }
}

MaybeError Buffer::EnsureDataInitialized(const ScopedCommandRecordingContext* commandContext) {
    if (!NeedsInitialization()) {
        return {};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(
    const ScopedCommandRecordingContext* commandContext,
    uint64_t offset,
    uint64_t size) {
    if (!NeedsInitialization()) {
        return {};
    }

    if (IsFullBufferRange(offset, size)) {
        SetInitialized(true);
        return {};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(
    const ScopedCommandRecordingContext* commandContext,
    const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return {};
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetInitialized(true);
    } else {
        DAWN_TRY(InitializeToZero(commandContext));
    }

    return {};
}

MaybeError Buffer::InitializeToZero(const ScopedCommandRecordingContext* commandContext) {
    DAWN_ASSERT(NeedsInitialization());

    DAWN_TRY(ClearWholeBuffer(commandContext, uint8_t(0u)));
    SetInitialized(true);
    GetDevice()->IncrementLazyClearCountForTesting();

    return {};
}

MaybeError Buffer::PredicatedClear(const ScopedSwapStateCommandRecordingContext* commandContext,
                                   ID3D11Predicate* predicate,
                                   uint8_t clearValue,
                                   uint64_t offset,
                                   uint64_t size) {
    DAWN_UNREACHABLE();
    return {};
}

MaybeError Buffer::Clear(const ScopedCommandRecordingContext* commandContext,
                         uint8_t clearValue,
                         uint64_t offset,
                         uint64_t size) {
    DAWN_ASSERT(!mMappedData);

    if (size == 0) {
        return {};
    }

    // Map the buffer if it is possible, so EnsureDataInitializedAsDestination() and ClearInternal()
    // can write the mapped memory directly.
    ScopedMap scopedMap;
    DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(commandContext, this, wgpu::MapMode::Write));

    // For non-staging buffers, we can use UpdateSubresource to write the data.
    DAWN_TRY(EnsureDataInitializedAsDestination(commandContext, offset, size));
    return ClearInternal(commandContext, clearValue, offset, size);
}

MaybeError Buffer::ClearWholeBuffer(const ScopedCommandRecordingContext* commandContext,
                                    uint8_t clearValue) {
    return ClearInternal(commandContext, clearValue, 0, GetAllocatedSize());
}

MaybeError Buffer::ClearInternal(const ScopedCommandRecordingContext* commandContext,
                                 uint8_t clearValue,
                                 uint64_t offset,
                                 uint64_t size) {
    DAWN_ASSERT(size != 0);

    // TODO(dawn:1705): use a reusable zero staging buffer to clear the buffer to avoid this CPU to
    // GPU copy.
    std::vector<uint8_t> clearData(size, clearValue);
    return WriteInternal(commandContext, offset, clearData.data(), size);
}

MaybeError Buffer::ClearPaddingInternal(const ScopedCommandRecordingContext* commandContext) {
    uint32_t paddingBytes = GetAllocatedSize() - GetSize();
    if (paddingBytes == 0) {
        return {};
    }
    uint32_t clearSize = paddingBytes;
    uint64_t clearOffset = GetSize();
    DAWN_TRY(ClearInternal(commandContext, 0, clearOffset, clearSize));

    return {};
}

MaybeError Buffer::Write(const ScopedCommandRecordingContext* commandContext,
                         uint64_t offset,
                         const void* data,
                         size_t size) {
    DAWN_ASSERT(size != 0);

    MarkUsedInPendingCommands();
    // Map the buffer if it is possible, so EnsureDataInitializedAsDestination() and WriteInternal()
    // can write the mapped memory directly.
    ScopedMap scopedMap;
    DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(commandContext, this, wgpu::MapMode::Write));

    // For non-staging buffers, we can use UpdateSubresource to write the data.
    DAWN_TRY(EnsureDataInitializedAsDestination(commandContext, offset, size));
    return WriteInternal(commandContext, offset, data, size);
}

// static
MaybeError Buffer::Copy(const ScopedCommandRecordingContext* commandContext,
                        Buffer* source,
                        uint64_t sourceOffset,
                        size_t size,
                        Buffer* destination,
                        uint64_t destinationOffset) {
    DAWN_ASSERT(size != 0);

    DAWN_TRY(source->EnsureDataInitialized(commandContext));
    DAWN_TRY(
        destination->EnsureDataInitializedAsDestination(commandContext, destinationOffset, size));
    return source->CopyToInternal(commandContext, sourceOffset, size, destination,
                                  destinationOffset);
}

ResultOrError<Buffer::ScopedMap> Buffer::ScopedMap::Create(
    const ScopedCommandRecordingContext* commandContext,
    Buffer* buffer,
    wgpu::MapMode mode) {
    if (mode == wgpu::MapMode::Write && !buffer->IsCPUWritable()) {
        return ScopedMap();
    }
    if (mode == wgpu::MapMode::Read && !buffer->IsCPUReadable()) {
        return ScopedMap();
    }

    if (buffer->mMappedData) {
        return ScopedMap(commandContext, buffer, /*needsUnmap=*/false);
    }

    DAWN_TRY(buffer->MapInternal(commandContext, mode));
    return ScopedMap(commandContext, buffer, /*needsUnmap=*/true);
}

// ScopedMap
Buffer::ScopedMap::ScopedMap() = default;

Buffer::ScopedMap::ScopedMap(const ScopedCommandRecordingContext* commandContext,
                             Buffer* buffer,
                             bool needsUnmap)
    : mCommandContext(commandContext), mBuffer(buffer), mNeedsUnmap(needsUnmap) {}

Buffer::ScopedMap::~ScopedMap() {
    Reset();
}

Buffer::ScopedMap::ScopedMap(Buffer::ScopedMap&& other) {
    this->operator=(std::move(other));
}

Buffer::ScopedMap& Buffer::ScopedMap::operator=(Buffer::ScopedMap&& other) {
    Reset();
    mCommandContext = other.mCommandContext;
    mBuffer = other.mBuffer;
    mNeedsUnmap = other.mNeedsUnmap;
    other.mBuffer = nullptr;
    other.mNeedsUnmap = false;
    return *this;
}

void Buffer::ScopedMap::Reset() {
    if (mNeedsUnmap) {
        mBuffer->UnmapInternal(mCommandContext);
    }
    mCommandContext = nullptr;
    mBuffer = nullptr;
    mNeedsUnmap = false;
}

uint8_t* Buffer::ScopedMap::GetMappedData() const {
    return mBuffer ? mBuffer->mMappedData.get() : nullptr;
}

// GPUUsableBuffer
ID3D11Buffer* GPUUsableBuffer::GetD3D11ConstantBufferForTesting() {
    ID3D11Buffer* buffer;
    if (GetDevice()->ConsumedError(GetD3D11ConstantBuffer(nullptr), &buffer)) {
        return nullptr;
    }

    return buffer;
}

ID3D11Buffer* GPUUsableBuffer::GetD3D11NonConstantBufferForTesting() {
    ID3D11Buffer* buffer;
    if (GetDevice()->ConsumedError(GetD3D11NonConstantBuffer(nullptr), &buffer)) {
        return nullptr;
    }

    return buffer;
}

ResultOrError<ComPtr<ID3D11ShaderResourceView>>
GPUUsableBuffer::CreateD3D11ShaderResourceViewFromD3DBuffer(ID3D11Buffer* d3d11Buffer,
                                                            uint64_t offset,
                                                            uint64_t size) {
    DAWN_ASSERT(IsAligned(offset, 4u));
    DAWN_ASSERT(IsAligned(size, 4u));
    UINT firstElement = static_cast<UINT>(offset / 4);
    UINT numElements = static_cast<UINT>(size / 4);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
    desc.BufferEx.FirstElement = firstElement;
    desc.BufferEx.NumElements = numElements;
    desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
    ComPtr<ID3D11ShaderResourceView> srv;
    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device()
                              ->CreateShaderResourceView(d3d11Buffer, &desc, &srv),
                          "ShaderResourceView creation"));

    return srv;
}

ResultOrError<ComPtr<ID3D11UnorderedAccessView1>>
GPUUsableBuffer::CreateD3D11UnorderedAccessViewFromD3DBuffer(ID3D11Buffer* d3d11Buffer,
                                                             uint64_t offset,
                                                             uint64_t size) {
    DAWN_ASSERT(IsAligned(offset, 4u));
    DAWN_ASSERT(IsAligned(size, 4u));

    UINT firstElement = static_cast<UINT>(offset / 4);
    UINT numElements = static_cast<UINT>(size / 4);

    D3D11_UNORDERED_ACCESS_VIEW_DESC1 desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = firstElement;
    desc.Buffer.NumElements = numElements;
    desc.Buffer.Flags = D3D11_BUFFER_UAV_FLAG_RAW;

    ComPtr<ID3D11UnorderedAccessView1> uav;
    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device5()
                              ->CreateUnorderedAccessView1(d3d11Buffer, &desc, &uav),
                          "UnorderedAccessView creation"));

    return uav;
}

MaybeError GPUUsableBuffer::UpdateD3D11ConstantBuffer(
    const ScopedCommandRecordingContext* commandContext,
    ID3D11Buffer* d3d11Buffer,
    bool firstTimeUpdate,
    uint64_t offset,
    const void* data,
    size_t size) {
    DAWN_ASSERT(size > 0);

    // For a full size write, UpdateSubresource1(D3D11_COPY_DISCARD) can be used to update
    // constant buffer.
    // WriteInternal() can be called with GetAllocatedSize(). We treat it as a full buffer write as
    // well.
    bool fullSizeUpdate = size >= GetSize() && offset == 0;
    if (fullSizeUpdate || firstTimeUpdate) {
        // Offset and size must be aligned with 16 for using UpdateSubresource1() on constant
        // buffer.
        size_t alignedOffset;
        if (offset < kConstantBufferUpdateAlignment - 1) {
            alignedOffset = 0;
        } else {
            // For offset we align to value <= offset.
            alignedOffset = Align(offset - (kConstantBufferUpdateAlignment - 1),
                                  kConstantBufferUpdateAlignment);
        }
        size_t alignedEnd = Align(offset + size, kConstantBufferUpdateAlignment);
        size_t alignedSize = alignedEnd - alignedOffset;

        DAWN_ASSERT((alignedSize % kConstantBufferUpdateAlignment) == 0);
        DAWN_ASSERT(alignedSize <= GetAllocatedSize());
        DAWN_ASSERT(offset >= alignedOffset);

        // Extra bytes on the left of offset we could write to. This is only valid if
        // firstTimeUpdate = true.
        size_t leftExtraBytes = offset - alignedOffset;
        DAWN_ASSERT(leftExtraBytes == 0 || firstTimeUpdate);

        // The layout of the buffer is like this:
        // |..........................| leftExtraBytes |     data   | ............... |
        // |<----------------- offset ---------------->|<-- size -->|
        // |<----- alignedOffset ---->|<--------- alignedSize --------->|
        std::unique_ptr<uint8_t[]> alignedBuffer;
        if (size != alignedSize) {
            alignedBuffer.reset(new uint8_t[alignedSize]);
            std::memcpy(alignedBuffer.get() + leftExtraBytes, data, size);
            data = alignedBuffer.get();
        }

        D3D11_BOX dstBox;
        dstBox.left = static_cast<UINT>(alignedOffset);
        dstBox.top = 0;
        dstBox.front = 0;
        dstBox.right = static_cast<UINT>(alignedOffset + alignedSize);
        dstBox.bottom = 1;
        dstBox.back = 1;
        // For full buffer write, D3D11_COPY_DISCARD is used to avoid GPU CPU synchronization.
        commandContext->UpdateSubresource1(d3d11Buffer, /*DstSubresource=*/0, &dstBox, data,
                                           /*SrcRowPitch=*/0,
                                           /*SrcDepthPitch=*/0,
                                           /*CopyFlags=*/D3D11_COPY_DISCARD);
        return {};
    }

    // If copy offset and size are not 16 bytes aligned, we have to create a staging buffer for
    // transfer the data to constant buffer.
    Ref<BufferBase> stagingBuffer;
    DAWN_TRY_ASSIGN(stagingBuffer, ToBackend(GetDevice())->GetStagingBuffer(commandContext, size));
    stagingBuffer->MarkUsedInPendingCommands();
    DAWN_TRY(ToBackend(stagingBuffer)->WriteInternal(commandContext, 0, data, size));
    DAWN_TRY(ToBackend(stagingBuffer.Get())
                 ->CopyToInternal(commandContext,
                                  /*sourceOffset=*/0,
                                  /*size=*/size, this, offset));
    ToBackend(GetDevice())->ReturnStagingBuffer(std::move(stagingBuffer));

    return {};
}

// GPUOnlyBuffer
void GPUOnlyBuffer::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the buffer is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the buffer.
    // - It may be called when the last ref to the buffer is dropped and the buffer
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the buffer since there are no other live refs.
    GPUUsableBuffer::DestroyImpl();

    mD3d11ConstantBuffer = nullptr;
    mD3d11NonConstantBuffer = nullptr;
}

void GPUOnlyBuffer::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mD3d11NonConstantBuffer.Get(), "Dawn_Buffer", GetLabel());
    SetDebugName(ToBackend(GetDevice()), mD3d11ConstantBuffer.Get(), "Dawn_ConstantBuffer",
                 GetLabel());
}

MaybeError GPUOnlyBuffer::InitializeInternal() {
    DAWN_ASSERT(!IsMappable(GetUsage()));

    bool needsConstantBuffer = GetUsage() & wgpu::BufferUsage::Uniform;
    bool onlyNeedsConstantBuffer =
        needsConstantBuffer && IsSubset(GetUsage(), kD3D11GPUOnlyUniformBufferUsages);

    if (!onlyNeedsConstantBuffer) {
        // Create mD3d11NonConstantBuffer
        wgpu::BufferUsage nonUniformUsage = GetUsage() & ~wgpu::BufferUsage::Uniform;
        D3D11_BUFFER_DESC bufferDescriptor;
        bufferDescriptor.ByteWidth = mAllocatedSize;
        bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
        bufferDescriptor.BindFlags = D3D11BufferBindFlags(nonUniformUsage);
        bufferDescriptor.CPUAccessFlags = 0;
        bufferDescriptor.MiscFlags = D3D11BufferMiscFlags(nonUniformUsage);
        bufferDescriptor.StructureByteStride = 0;

        DAWN_TRY(CheckOutOfMemoryHRESULT(
            ToBackend(GetDevice())
                ->GetD3D11Device()
                ->CreateBuffer(&bufferDescriptor, nullptr, &mD3d11NonConstantBuffer),
            "ID3D11Device::CreateBuffer"));
    }

    if (needsConstantBuffer) {
        // Create mD3d11ConstantBuffer
        D3D11_BUFFER_DESC bufferDescriptor;
        bufferDescriptor.ByteWidth = mAllocatedSize;
        bufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
        bufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDescriptor.CPUAccessFlags = 0;
        bufferDescriptor.MiscFlags = 0;
        bufferDescriptor.StructureByteStride = 0;

        DAWN_TRY(CheckOutOfMemoryHRESULT(
            ToBackend(GetDevice())
                ->GetD3D11Device()
                ->CreateBuffer(&bufferDescriptor, nullptr, &mD3d11ConstantBuffer),
            "ID3D11Device::CreateBuffer"));
    }

    DAWN_ASSERT(mD3d11NonConstantBuffer || mD3d11ConstantBuffer);

    return {};
}

MaybeError GPUOnlyBuffer::PredicatedClear(
    const ScopedSwapStateCommandRecordingContext* commandContext,
    ID3D11Predicate* predicate,
    uint8_t clearValue,
    uint64_t offset,
    uint64_t size) {
    // The clear will *NOT* be performed if the predicate's data is false.
    commandContext->GetD3D11DeviceContext4()->SetPredication(predicate, false);
    auto result = Clear(commandContext, clearValue, offset, size);
    commandContext->GetD3D11DeviceContext4()->SetPredication(nullptr, false);
    return result;
}

ResultOrError<ID3D11Buffer*> GPUOnlyBuffer::GetD3D11ConstantBuffer(
    const ScopedCommandRecordingContext* commandContext) {
    if (mConstantBufferIsUpdated) {
        return mD3d11ConstantBuffer.Get();
    }

    DAWN_ASSERT(mD3d11NonConstantBuffer);
    DAWN_ASSERT(mD3d11ConstantBuffer);
    if (commandContext) {
        commandContext->CopyResource(mD3d11ConstantBuffer.Get(), mD3d11NonConstantBuffer.Get());
    } else {
        auto tempCommandContext =
            ToBackend(GetDevice()->GetQueue())
                ->GetScopedPendingCommandContext(QueueBase::SubmitMode::Normal);
        tempCommandContext.CopyResource(mD3d11ConstantBuffer.Get(), mD3d11NonConstantBuffer.Get());
    }
    mConstantBufferIsUpdated = true;

    return mD3d11ConstantBuffer.Get();
}

ResultOrError<ID3D11Buffer*> GPUOnlyBuffer::GetD3D11NonConstantBuffer(
    const ScopedCommandRecordingContext*) {
    return mD3d11NonConstantBuffer.Get();
}

ResultOrError<ComPtr<ID3D11ShaderResourceView>>
GPUOnlyBuffer::UseAsSRV(const ScopedCommandRecordingContext*, uint64_t offset, uint64_t size) {
    return CreateD3D11ShaderResourceViewFromD3DBuffer(mD3d11NonConstantBuffer.Get(), offset, size);
}

ResultOrError<ComPtr<ID3D11UnorderedAccessView1>>
GPUOnlyBuffer::UseAsUAV(const ScopedCommandRecordingContext*, uint64_t offset, uint64_t size) {
    ComPtr<ID3D11UnorderedAccessView1> uav;
    DAWN_TRY_ASSIGN(uav, CreateD3D11UnorderedAccessViewFromD3DBuffer(mD3d11NonConstantBuffer.Get(),
                                                                     offset, size));

    // Since UAV will modify the non-constant buffer's content, the constant buffer's content would
    // also need to be updated afterwards.
    mConstantBufferIsUpdated = false;

    return uav;
}

MaybeError GPUOnlyBuffer::WriteInternal(const ScopedCommandRecordingContext* commandContext,
                                        uint64_t offset,
                                        const void* data,
                                        size_t size) {
    if (size == 0) {
        return {};
    }

    if (mD3d11NonConstantBuffer) {
        D3D11_BOX box;
        box.left = static_cast<UINT>(offset);
        box.top = 0;
        box.front = 0;
        box.right = static_cast<UINT>(offset + size);
        box.bottom = 1;
        box.back = 1;
        commandContext->UpdateSubresource1(mD3d11NonConstantBuffer.Get(), /*DstSubresource=*/0,
                                           /*pDstBox=*/&box, data,
                                           /*SrcRowPitch=*/0,
                                           /*SrcDepthPitch=*/0,
                                           /*CopyFlags=*/0);
        if (!mD3d11ConstantBuffer) {
            return {};
        }

        // if mConstantBufferIsUpdated is false, the content of mD3d11ConstantBuffer will be
        // updated by EnsureConstantBufferIsUpdated() when the constant buffer is about to be used.
        if (!mConstantBufferIsUpdated) {
            return {};
        }

        // Copy the modified part of the mD3d11NonConstantBuffer to mD3d11ConstantBuffer.
        commandContext->CopySubresourceRegion(
            mD3d11ConstantBuffer.Get(), /*DstSubresource=*/0, /*DstX=*/offset,
            /*DstY=*/0,
            /*DstZ=*/0, mD3d11NonConstantBuffer.Get(), /*SrcSubresource=*/0, /*pSrcBux=*/&box);

        return {};
    }

    DAWN_ASSERT(mD3d11ConstantBuffer);

    return UpdateD3D11ConstantBuffer(commandContext, mD3d11ConstantBuffer.Get(),
                                     /*firstUpdate=*/false, offset, data, size);
}

MaybeError GPUOnlyBuffer::CopyToInternal(const ScopedCommandRecordingContext* commandContext,
                                         uint64_t sourceOffset,
                                         size_t size,
                                         Buffer* destination,
                                         uint64_t destinationOffset) {
    ID3D11Buffer* d3d11SourceBuffer =
        mD3d11NonConstantBuffer ? mD3d11NonConstantBuffer.Get() : mD3d11ConstantBuffer.Get();
    DAWN_ASSERT(d3d11SourceBuffer);

    return destination->CopyFromD3DInternal(commandContext, d3d11SourceBuffer, sourceOffset, size,
                                            destinationOffset);
}

MaybeError GPUOnlyBuffer::CopyFromD3DInternal(const ScopedCommandRecordingContext* commandContext,
                                              ID3D11Buffer* d3d11SourceBuffer,
                                              uint64_t sourceOffset,
                                              size_t size,
                                              uint64_t destinationOffset) {
    D3D11_BOX srcBox;
    srcBox.left = static_cast<UINT>(sourceOffset);
    srcBox.top = 0;
    srcBox.front = 0;
    srcBox.right = static_cast<UINT>(sourceOffset + size);
    srcBox.bottom = 1;
    srcBox.back = 1;

    if (mD3d11NonConstantBuffer) {
        commandContext->CopySubresourceRegion(mD3d11NonConstantBuffer.Get(), /*DstSubresource=*/0,
                                              /*DstX=*/destinationOffset,
                                              /*DstY=*/0,
                                              /*DstZ=*/0, d3d11SourceBuffer, /*SrcSubresource=*/0,
                                              &srcBox);
    }

    // if mConstantBufferIsUpdated is false, the content of mD3d11ConstantBuffer  will be
    // updated by EnsureConstantBufferIsUpdated() when the constant buffer is about to be used.
    if (!mConstantBufferIsUpdated) {
        return {};
    }

    if (mD3d11ConstantBuffer) {
        commandContext->CopySubresourceRegion(mD3d11ConstantBuffer.Get(), /*DstSubresource=*/0,
                                              /*DstX=*/destinationOffset,
                                              /*DstY=*/0,
                                              /*DstZ=*/0, d3d11SourceBuffer, /*SrcSubresource=*/0,
                                              &srcBox);
    }

    return {};
}

MaybeError GPUOnlyBuffer::ClearPaddingInternal(
    const ScopedCommandRecordingContext* commandContext) {
    uint32_t paddingBytes = GetAllocatedSize() - GetSize();
    if (paddingBytes == 0) {
        return {};
    }

    uint32_t clearSize = paddingBytes;
    uint64_t clearOffset = GetSize();
    // 'UpdateSubresource1' is more preferable for updating uniform buffers, as it incurs no
    // GPU stall.
    if (mD3d11ConstantBuffer && !mD3d11NonConstantBuffer) {
        clearSize = Align(paddingBytes, kConstantBufferUpdateAlignment);
        clearOffset = GetAllocatedSize() - clearSize;

        std::vector<uint8_t> clearData(clearSize, 0);
        DAWN_TRY(UpdateD3D11ConstantBuffer(commandContext, mD3d11ConstantBuffer.Get(),
                                           /*firstTimeUpdate=*/true, clearOffset, clearData.data(),
                                           clearSize));
    } else {
        DAWN_TRY(ClearInternal(commandContext, 0, clearOffset, paddingBytes));
    }

    return {};
}

}  // namespace dawn::native::d3d11
