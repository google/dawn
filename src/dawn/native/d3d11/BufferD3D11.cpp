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

MaybeError ValidationUsage(wgpu::BufferUsage usage) {
    // https://learn.microsoft.com/en-us/windows/win32/api/d3d11/ne-d3d11-d3d11_bind_flag
    // D3D11 doesn't support constants buffers with other accelerated GPU usages.
    // TODO(dawn:1755): find a way to workaround this D3D11 limitation.
    constexpr wgpu::BufferUsage kAllowedUniformBufferUsages =
        wgpu::BufferUsage::CopyDst | wgpu::BufferUsage::Uniform;

    DAWN_INVALID_IF(
        usage & wgpu::BufferUsage::Uniform && !IsSubset(usage, kAllowedUniformBufferUsages),
        "Buffer usage can't be both uniform and other accelerated usages with D3D11");

    return {};
}

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
    constexpr wgpu::BufferUsage kMapUsages =
        wgpu::BufferUsage::MapRead | wgpu::BufferUsage::MapWrite;
    return usage & kMapUsages;
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
        bindFlags |= D3D11_BIND_FLAG::D3D11_BIND_VERTEX_BUFFER;
    }
    if (usage & wgpu::BufferUsage::Index) {
        bindFlags |= D3D11_BIND_FLAG::D3D11_BIND_INDEX_BUFFER;
    }
    if (usage & (wgpu::BufferUsage::Uniform)) {
        bindFlags |= D3D11_BIND_FLAG::D3D11_BIND_CONSTANT_BUFFER;
    }
    if (usage & (wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        bindFlags |= D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
    }
    if (usage & kReadOnlyStorageBuffer) {
        bindFlags |= D3D11_BIND_FLAG::D3D11_BIND_SHADER_RESOURCE;
    }

    constexpr wgpu::BufferUsage kCopyUsages =
        wgpu::BufferUsage::CopySrc | wgpu::BufferUsage::CopyDst;
    // If the buffer only has CopySrc and CopyDst usages are used as staging buffers for copy.
    // Because D3D11 doesn't allow copying between buffer and texture, we will use compute shader
    // to copy data between buffer and texture. So the buffer needs to be bound as unordered access
    // view.
    if (IsSubset(usage, kCopyUsages)) {
        bindFlags |= D3D11_BIND_FLAG::D3D11_BIND_UNORDERED_ACCESS;
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
        cpuAccessFlags = D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_READ |
                         D3D11_CPU_ACCESS_FLAG::D3D11_CPU_ACCESS_WRITE;
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
    DAWN_TRY(ValidationUsage(GetUsage()));

    // Allocate at least 4 bytes so clamped accesses are always in bounds.
    uint64_t size = std::max(GetSize(), uint64_t(4u));
    size_t alignment = D3D11BufferSizeAlignment(GetUsage());
    if (size > std::numeric_limits<uint64_t>::max() - alignment) {
        // Alignment would overlow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    mAllocatedSize = Align(size, alignment);

    // Create mD3d11Buffer
    D3D11_BUFFER_DESC bufferDescriptor;
    bufferDescriptor.ByteWidth = mAllocatedSize;
    bufferDescriptor.Usage = D3D11BufferUsage(GetUsage());
    bufferDescriptor.BindFlags = D3D11BufferBindFlags(GetUsage());
    bufferDescriptor.CPUAccessFlags = D3D11CpuAccessFlags(GetUsage());
    bufferDescriptor.MiscFlags = D3D11BufferMiscFlags(GetUsage());
    bufferDescriptor.StructureByteStride = 0;

    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device()
                              ->CreateBuffer(&bufferDescriptor, nullptr, &mD3d11Buffer),
                          "ID3D11Device::CreateBuffer"));

    SetLabelImpl();
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
    DAWN_TRY(CheckHRESULT(commandContext->GetD3D11DeviceContext()->Map(
                              mD3d11Buffer.Get(), /*Subresource=*/0, D3D11_MAP_READ_WRITE,
                              /*MapFlags=*/0, &mappedResource),
                          "ID3D11DeviceContext::Map"));
    mMappedData = reinterpret_cast<uint8_t*>(mappedResource.pData);

    return {};
}

void Buffer::UnmapInternal() {
    DAWN_ASSERT(mMappedData);

    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();
    commandContext->GetD3D11DeviceContext()->Unmap(mD3d11Buffer.Get(), /*Subresource=*/0);
    mMappedData = nullptr;
}

MaybeError Buffer::MapAtCreationImpl() {
    DAWN_ASSERT(IsMappable(GetUsage()));
    return MapInternal();
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    DAWN_ASSERT(mD3d11Buffer);

    // TODO(dawn:1705): make sure the map call is not blocked by the GPU operations.
    DAWN_TRY(MapInternal());

    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();
    DAWN_TRY(EnsureDataInitialized(commandContext));

    return {};
}

void Buffer::UnmapImpl() {
    DAWN_ASSERT(mD3d11Buffer);
    DAWN_ASSERT(mMappedData);
    UnmapInternal();
}

void* Buffer::GetMappedPointer() {
    // The frontend asks that the pointer returned is from the start of the resource
    // irrespective of the offset passed in MapAsyncImpl, which is what mMappedData is.
    return mMappedData;
}

void Buffer::DestroyImpl() {
    BufferBase::DestroyImpl();
    if (mMappedData) {
        UnmapInternal();
    }
    mD3d11Buffer = nullptr;
}

void Buffer::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mD3d11Buffer.Get(), "Dawn_Buffer", GetLabel());
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

ResultOrError<ComPtr<ID3D11ShaderResourceView>> Buffer::CreateD3D11ShaderResourceView(
    uint64_t offset,
    uint64_t size) const {
    DAWN_ASSERT(IsAligned(offset, 4u));
    DAWN_ASSERT(IsAligned(size, 4u));
    UINT firstElement = static_cast<UINT>(offset / 4);
    UINT numElements = static_cast<UINT>(size / 4);

    D3D11_SHADER_RESOURCE_VIEW_DESC desc;
    desc.Format = DXGI_FORMAT_R32_TYPELESS;
    desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
    desc.Buffer.FirstElement = firstElement;
    desc.Buffer.NumElements = numElements;

    ComPtr<ID3D11ShaderResourceView> srv;
    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device()
                              ->CreateShaderResourceView(mD3d11Buffer.Get(), &desc, &srv),
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
    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device5()
                              ->CreateUnorderedAccessView1(mD3d11Buffer.Get(), &desc, &uav),
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
    if (size == 0) {
        return {};
    }

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
        return {};
    }

    // UpdateSubresource can only be used to update non-mappable buffers.
    DAWN_ASSERT(!IsMappable(GetUsage()));

    ID3D11DeviceContext1* d3d11DeviceContext1 = commandContext->GetD3D11DeviceContext1();

    // For updating the full buffer, just pass nullptr as the pDstBox.
    if (offset == 0 && size == GetAllocatedSize()) {
        d3d11DeviceContext1->UpdateSubresource(GetD3D11Buffer(), /*DstSubresource=*/0,
                                               /*pDstBox=*/nullptr, data,
                                               /*SrcRowPitch=*/0,
                                               /*SrcDepthPitch*/ 0);
        return {};
    }

    D3D11_BOX dstBox;
    dstBox.left = offset;
    dstBox.right = offset + size;
    dstBox.top = 0;
    dstBox.bottom = 1;
    dstBox.front = 0;
    dstBox.back = 1;

    // TODO(dawn:1739): check whether driver supports partial update of uniform buffer.
    if ((GetUsage() & wgpu::BufferUsage::Uniform)) {
        d3d11DeviceContext1->UpdateSubresource1(GetD3D11Buffer(), /*DstSubresource=*/0, &dstBox,
                                                data,
                                                /*SrcRowPitch=*/0,
                                                /*SrcDepthPitch*/ 0, D3D11_COPY_NO_OVERWRITE);
    } else {
        d3d11DeviceContext1->UpdateSubresource(GetD3D11Buffer(), /*DstSubresource=*/0, &dstBox,
                                               data,
                                               /*SrcRowPitch=*/0,
                                               /*SrcDepthPitch*/ 0);
    }

    return {};
}

MaybeError Buffer::CopyFromBuffer(CommandRecordingContext* commandContext,
                                  uint64_t offset,
                                  size_t size,
                                  Buffer* source,
                                  uint64_t sourceOffset) {
    if (size == 0) {
        // Skip no-op copies.
        return {};
    }

    DAWN_TRY(source->EnsureDataInitialized(commandContext));
    DAWN_TRY(EnsureDataInitializedAsDestination(commandContext, offset, size));

    D3D11_BOX srcBox;
    srcBox.left = sourceOffset;
    srcBox.right = sourceOffset + size;
    srcBox.top = 0;
    srcBox.bottom = 1;
    srcBox.front = 0;
    srcBox.back = 1;
    commandContext->GetD3D11DeviceContext()->CopySubresourceRegion(
        GetD3D11Buffer(), /*DstSubresource=*/0, /*DstX=*/offset, /*DstY=*/0, /*DstZ=*/0,
        source->GetD3D11Buffer(), /*SrcSubresource=*/0, &srcBox);

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
