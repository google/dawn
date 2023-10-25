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
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/Constants.h"
#include "dawn/common/Math.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/DynamicUploader.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {
namespace {

constexpr wgpu::BufferUsage kD3D11AllowedUniformBufferUsages =
    wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::CopySrc;

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

D3D11_USAGE D3D11BufferUsage(wgpu::BufferUsage usage) {
    if (IsMappable(usage)) {
        return D3D11_USAGE_STAGING;
    } else {
        return D3D11_USAGE_DEFAULT;
    }
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
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }
    if (usage & kReadOnlyStorageBuffer) {
        bindFlags |= D3D11_BIND_SHADER_RESOURCE;
    }

    constexpr wgpu::BufferUsage kCopyUsages =
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    // If the buffer only has CopySrc and CopyDst usages are used as staging buffers for copy.
    // Because D3D11 doesn't allow copying between buffer and texture, we will use compute shader
    // to copy data between buffer and texture. So the buffer needs to be bound as unordered access
    // view.
    if (IsSubset(usage, kCopyUsages)) {
        bindFlags |= D3D11_BIND_UNORDERED_ACCESS;
    }

    return bindFlags;
}

UINT D3D11CpuAccessFlags(wgpu::BufferUsage usage) {
    UINT cpuAccessFlags = 0;
    if (IsMappable(usage)) {
        // D3D11 doesn't allow copying between buffer and texture.
        //  - For buffer to texture copy, we need to use a staging(mappable) texture, and memcpy the
        //    data from the staging buffer to the staging texture first. So D3D11_CPU_ACCESS_READ is
        //    needed for MapWrite usage.
        //  - For texture to buffer copy, we may need copy texture to a staging (mappable)
        //    texture, and then memcpy the data from the staging texture to the staging buffer. So
        //    D3D11_CPU_ACCESS_WRITE is needed to MapRead usage.
        cpuAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    }
    return cpuAccessFlags;
}

UINT D3D11BufferMiscFlags(wgpu::BufferUsage usage) {
    UINT miscFlags = 0;
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
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

}  // namespace

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device, const BufferDescriptor* descriptor) {
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));
    DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation));
    return buffer;
}

MaybeError Buffer::Initialize(bool mappedAtCreation) {
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

    bool needsConstantBuffer = GetUsage() & wgpu::BufferUsage::Uniform;
    bool onlyNeedsConstantBuffer =
        needsConstantBuffer && IsSubset(GetUsage(), kD3D11AllowedUniformBufferUsages);

    if (!onlyNeedsConstantBuffer) {
        // Create mD3d11NonConstantBuffer
        wgpu::BufferUsage nonUniformUsage = GetUsage() & ~wgpu::BufferUsage::Uniform;
        D3D11_BUFFER_DESC bufferDescriptor;
        bufferDescriptor.ByteWidth = mAllocatedSize;
        bufferDescriptor.Usage = D3D11BufferUsage(nonUniformUsage);
        bufferDescriptor.BindFlags = D3D11BufferBindFlags(nonUniformUsage);
        bufferDescriptor.CPUAccessFlags = D3D11CpuAccessFlags(nonUniformUsage);
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

    SetLabelImpl();

    if (!mappedAtCreation) {
        if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting)) {
            DAWN_TRY(ClearInternal(ToBackend(GetDevice())->GetPendingCommandContext(), 1u));
        }

        // Initialize the padding bytes to zero.
        if (GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse)) {
            uint32_t paddingBytes = GetAllocatedSize() - GetSize();
            if (paddingBytes > 0) {
                uint32_t clearSize = paddingBytes;
                uint64_t clearOffset = GetSize();
                DAWN_TRY(ClearInternal(ToBackend(GetDevice())->GetPendingCommandContext(), 0,
                                       clearOffset, clearSize));
            }
        }
    }

    return {};
}

Buffer::~Buffer() = default;

bool Buffer::IsCPUWritableAtCreation() const {
    return IsMappable(GetUsage());
}

MaybeError Buffer::MapInternal() {
    DAWN_ASSERT(IsMappable(GetUsage()));
    DAWN_ASSERT(!mMappedData);

    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();

    // Always map buffer with D3D11_MAP_READ_WRITE even for mapping wgpu::MapMode:Read, because we
    // need write permission to initialize the buffer.
    // TODO(dawn:1705): investigate the performance impact of mapping with D3D11_MAP_READ_WRITE.
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    DAWN_TRY(CheckHRESULT(
        commandContext->GetD3D11DeviceContext4()->Map(mD3d11NonConstantBuffer.Get(),
                                                      /*Subresource=*/0, D3D11_MAP_READ_WRITE,
                                                      /*MapFlags=*/0, &mappedResource),
        "ID3D11DeviceContext::Map"));
    mMappedData = reinterpret_cast<uint8_t*>(mappedResource.pData);

    return {};
}

void Buffer::UnmapInternal() {
    DAWN_ASSERT(mMappedData);

    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();
    commandContext->GetD3D11DeviceContext4()->Unmap(mD3d11NonConstantBuffer.Get(),
                                                    /*Subresource=*/0);
    mMappedData = nullptr;
}

MaybeError Buffer::MapAtCreationImpl() {
    DAWN_ASSERT(IsMappable(GetUsage()));
    return MapInternal();
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    DAWN_ASSERT(mD3d11NonConstantBuffer);

    // TODO(dawn:1705): make sure the map call is not blocked by the GPU operations.
    DAWN_TRY(MapInternal());

    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();
    DAWN_TRY(EnsureDataInitialized(commandContext));

    return {};
}

void Buffer::UnmapImpl() {
    DAWN_ASSERT(mD3d11NonConstantBuffer);
    DAWN_ASSERT(mMappedData);
    UnmapInternal();
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
        UnmapInternal();
    }
    mD3d11NonConstantBuffer = nullptr;
}

void Buffer::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mD3d11NonConstantBuffer.Get(), "Dawn_Buffer", GetLabel());
    SetDebugName(ToBackend(GetDevice()), mD3d11ConstantBuffer.Get(), "Dawn_ConstantBuffer",
                 GetLabel());
}

MaybeError Buffer::EnsureDataInitialized(CommandRecordingContext* commandContext) {
    if (!NeedsInitialization()) {
        return {};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                      uint64_t offset,
                                                      uint64_t size) {
    if (!NeedsInitialization()) {
        return {};
    }

    if (IsFullBufferRange(offset, size)) {
        SetIsDataInitialized();
        return {};
    }

    DAWN_TRY(InitializeToZero(commandContext));
    return {};
}

MaybeError Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                      const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return {};
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetIsDataInitialized();
    } else {
        DAWN_TRY(InitializeToZero(commandContext));
    }

    return {};
}

MaybeError Buffer::InitializeToZero(CommandRecordingContext* commandContext) {
    DAWN_ASSERT(NeedsInitialization());

    DAWN_TRY(ClearInternal(commandContext, uint8_t(0u)));
    SetIsDataInitialized();
    GetDevice()->IncrementLazyClearCountForTesting();

    return {};
}

void Buffer::MarkMutated() {
    mConstantBufferIsUpdated = false;
}

void Buffer::EnsureConstantBufferIsUpdated(CommandRecordingContext* commandContext) {
    if (mConstantBufferIsUpdated) {
        return;
    }

    DAWN_ASSERT(mD3d11NonConstantBuffer);
    DAWN_ASSERT(mD3d11ConstantBuffer);
    commandContext->GetD3D11DeviceContext4()->CopyResource(mD3d11ConstantBuffer.Get(),
                                                           mD3d11NonConstantBuffer.Get());
    mConstantBufferIsUpdated = true;
}

ResultOrError<ComPtr<ID3D11ShaderResourceView>> Buffer::CreateD3D11ShaderResourceView(
    uint64_t offset,
    uint64_t size) const {
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
    DAWN_TRY(
        CheckHRESULT(ToBackend(GetDevice())
                         ->GetD3D11Device()
                         ->CreateShaderResourceView(mD3d11NonConstantBuffer.Get(), &desc, &srv),
                     "ShaderResourceView creation"));

    return srv;
}

ResultOrError<ComPtr<ID3D11UnorderedAccessView1>> Buffer::CreateD3D11UnorderedAccessView1(
    uint64_t offset,
    uint64_t size) const {
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
    DAWN_TRY(
        CheckHRESULT(ToBackend(GetDevice())
                         ->GetD3D11Device5()
                         ->CreateUnorderedAccessView1(mD3d11NonConstantBuffer.Get(), &desc, &uav),
                     "UnorderedAccessView creation"));
    return uav;
}

MaybeError Buffer::Clear(CommandRecordingContext* commandContext,
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
    DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(this));

    // For non-staging buffers, we can use UpdateSubresource to write the data.
    DAWN_TRY(EnsureDataInitializedAsDestination(commandContext, offset, size));
    return ClearInternal(commandContext, clearValue, offset, size);
}

MaybeError Buffer::ClearInternal(CommandRecordingContext* commandContext,
                                 uint8_t clearValue,
                                 uint64_t offset,
                                 uint64_t size) {
    if (size <= 0) {
        DAWN_ASSERT(offset == 0);
        size = GetAllocatedSize();
    }

    if (mMappedData) {
        memset(mMappedData + offset, clearValue, size);
        // The WebGPU uniform buffer is not mappable.
        DAWN_ASSERT(!mD3d11ConstantBuffer);
        return {};
    }

    // TODO(dawn:1705): use a reusable zero staging buffer to clear the buffer to avoid this CPU to
    // GPU copy.
    std::vector<uint8_t> clearData(size, clearValue);
    return WriteInternal(commandContext, offset, clearData.data(), size);
}

MaybeError Buffer::Write(CommandRecordingContext* commandContext,
                         uint64_t offset,
                         const void* data,
                         size_t size) {
    DAWN_ASSERT(size != 0);

    MarkUsedInPendingCommands();
    // Map the buffer if it is possible, so EnsureDataInitializedAsDestination() and WriteInternal()
    // can write the mapped memory directly.
    ScopedMap scopedMap;
    DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(this));

    // For non-staging buffers, we can use UpdateSubresource to write the data.
    DAWN_TRY(EnsureDataInitializedAsDestination(commandContext, offset, size));
    return WriteInternal(commandContext, offset, data, size);
}

MaybeError Buffer::WriteInternal(CommandRecordingContext* commandContext,
                                 uint64_t offset,
                                 const void* data,
                                 size_t size) {
    if (size == 0) {
        return {};
    }

    // Map the buffer if it is possible, so WriteInternal() can write the mapped memory directly.
    ScopedMap scopedMap;
    DAWN_TRY_ASSIGN(scopedMap, ScopedMap::Create(this));

    if (scopedMap.GetMappedData()) {
        memcpy(scopedMap.GetMappedData() + offset, data, size);
        // The WebGPU uniform buffer is not mappable.
        DAWN_ASSERT(!mD3d11ConstantBuffer);
        return {};
    }

    // UpdateSubresource can only be used to update non-mappable buffers.
    DAWN_ASSERT(!IsMappable(GetUsage()));

    auto* d3d11DeviceContext = commandContext->GetD3D11DeviceContext4();

    if (mD3d11NonConstantBuffer) {
        D3D11_BOX box;
        box.left = offset;
        box.right = offset + size;
        box.top = 0;
        box.bottom = 1;
        box.front = 0;
        box.back = 1;
        d3d11DeviceContext->UpdateSubresource(mD3d11NonConstantBuffer.Get(), /*DstSubresource=*/0,
                                              &box, data,
                                              /*SrcRowPitch=*/0,
                                              /*SrcDepthPitch*/ 0);
        if (!mD3d11ConstantBuffer) {
            return {};
        }

        // if mConstantBufferIsUpdated is false, the content of mD3d11ConstantBuffer will be
        // updated by EnsureConstantBufferIsUpdated() when the constant buffer is about to be used.
        if (!mConstantBufferIsUpdated) {
            return {};
        }

        // Copy the modified part of the mD3d11NonConstantBuffer to mD3d11ConstantBuffer.
        d3d11DeviceContext->CopySubresourceRegion(
            mD3d11ConstantBuffer.Get(), /*DstSubresource=*/0, /*DstX=*/offset,
            /*DstY=*/0,
            /*DstZ=*/0, mD3d11NonConstantBuffer.Get(), /*SrcSubresource=*/0, &box);

        return {};
    }

    DAWN_ASSERT(mD3d11ConstantBuffer);

    // For a full size write, UpdateSubresource() can be used to update mD3d11ConstantBuffer.
    if (size == GetSize() && offset == 0) {
        if (size == mAllocatedSize) {
            d3d11DeviceContext->UpdateSubresource(mD3d11ConstantBuffer.Get(), /*DstSubresource=*/0,
                                                  nullptr, data,
                                                  /*SrcRowPitch=*/size,
                                                  /*SrcDepthPitch*/ 0);
        } else {
            std::vector<uint8_t> allocatedData(mAllocatedSize, 0);
            std::memcpy(allocatedData.data(), data, size);
            d3d11DeviceContext->UpdateSubresource(mD3d11ConstantBuffer.Get(), /*DstSubresource=*/0,
                                                  nullptr, allocatedData.data(),
                                                  /*SrcRowPitch=*/mAllocatedSize,
                                                  /*SrcDepthPitch*/ 0);
        }
        return {};
    }

    // If the mD3d11NonConstantBuffer is null, we have to create a staging buffer for transfer the
    // data to mD3d11ConstantBuffer.
    BufferDescriptor descriptor;
    descriptor.usage = wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc;
    descriptor.size = Align(size, D3D11BufferSizeAlignment(descriptor.usage));
    descriptor.mappedAtCreation = false;
    descriptor.label = "DawnWriteStagingBuffer";
    Ref<BufferBase> stagingBuffer;
    DAWN_TRY_ASSIGN(stagingBuffer, GetDevice()->CreateBuffer(&descriptor));

    DAWN_TRY(ToBackend(stagingBuffer)->WriteInternal(commandContext, 0, data, size));

    return Buffer::CopyInternal(commandContext, ToBackend(stagingBuffer.Get()), /*sourceOffset=*/0,
                                /*size=*/size, this, offset);
}

// static
MaybeError Buffer::Copy(CommandRecordingContext* commandContext,
                        Buffer* source,
                        uint64_t sourceOffset,
                        size_t size,
                        Buffer* destination,
                        uint64_t destinationOffset) {
    DAWN_ASSERT(size != 0);

    DAWN_TRY(source->EnsureDataInitialized(commandContext));
    DAWN_TRY(
        destination->EnsureDataInitializedAsDestination(commandContext, destinationOffset, size));
    return CopyInternal(commandContext, source, sourceOffset, size, destination, destinationOffset);
}

// static
MaybeError Buffer::CopyInternal(CommandRecordingContext* commandContext,
                                Buffer* source,
                                uint64_t sourceOffset,
                                size_t size,
                                Buffer* destination,
                                uint64_t destinationOffset) {
    D3D11_BOX srcBox;
    srcBox.left = sourceOffset;
    srcBox.right = sourceOffset + size;
    srcBox.top = 0;
    srcBox.bottom = 1;
    srcBox.front = 0;
    srcBox.back = 1;
    ID3D11Buffer* d3d11SourceBuffer = source->mD3d11NonConstantBuffer
                                          ? source->mD3d11NonConstantBuffer.Get()
                                          : source->mD3d11ConstantBuffer.Get();
    DAWN_ASSERT(d3d11SourceBuffer);

    if (destination->mD3d11NonConstantBuffer) {
        commandContext->GetD3D11DeviceContext4()->CopySubresourceRegion(
            destination->mD3d11NonConstantBuffer.Get(), /*DstSubresource=*/0,
            /*DstX=*/destinationOffset,
            /*DstY=*/0,
            /*DstZ=*/0, d3d11SourceBuffer, /*SrcSubresource=*/0, &srcBox);
    }

    // if mConstantBufferIsUpdated is false, the content of mD3d11ConstantBuffer  will be
    // updated by EnsureConstantBufferIsUpdated() when the constant buffer is about to be used.
    if (!destination->mConstantBufferIsUpdated) {
        return {};
    }

    if (destination->mD3d11ConstantBuffer) {
        commandContext->GetD3D11DeviceContext4()->CopySubresourceRegion(
            destination->mD3d11ConstantBuffer.Get(), /*DstSubresource=*/0,
            /*DstX=*/destinationOffset,
            /*DstY=*/0,
            /*DstZ=*/0, d3d11SourceBuffer, /*SrcSubresource=*/0, &srcBox);
    }

    return {};
}

ResultOrError<Buffer::ScopedMap> Buffer::ScopedMap::Create(Buffer* buffer) {
    if (!IsMappable(buffer->GetUsage())) {
        return ScopedMap(nullptr, /*needsUnmap=*/false);
    }

    if (buffer->mMappedData) {
        return ScopedMap(buffer, /*needsUnmap=*/false);
    }

    DAWN_TRY(buffer->MapInternal());
    return ScopedMap(buffer, /*needsUnmap=*/true);
}

Buffer::ScopedMap::ScopedMap() = default;

Buffer::ScopedMap::ScopedMap(Buffer* buffer, bool needsUnmap)
    : mBuffer(buffer), mNeedsUnmap(needsUnmap) {}

Buffer::ScopedMap::~ScopedMap() {
    Reset();
}

Buffer::ScopedMap::ScopedMap(Buffer::ScopedMap&& other) {
    this->operator=(std::move(other));
}

Buffer::ScopedMap& Buffer::ScopedMap::operator=(Buffer::ScopedMap&& other) {
    Reset();
    mBuffer = other.mBuffer;
    mNeedsUnmap = other.mNeedsUnmap;
    other.mBuffer = nullptr;
    other.mNeedsUnmap = false;
    return *this;
}

void Buffer::ScopedMap::Reset() {
    if (mNeedsUnmap) {
        mBuffer->UnmapInternal();
    }
    mBuffer = nullptr;
    mNeedsUnmap = false;
}

uint8_t* Buffer::ScopedMap::GetMappedData() const {
    return mBuffer ? mBuffer->mMappedData : nullptr;
}

}  // namespace dawn::native::d3d11
