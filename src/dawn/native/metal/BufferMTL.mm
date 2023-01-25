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

#include "dawn/native/metal/BufferMTL.h"

#include "dawn/common/Math.h"
#include "dawn/common/Platform.h"
#include "dawn/native/CommandBuffer.h"
#include "dawn/native/metal/CommandRecordingContext.h"
#include "dawn/native/metal/DeviceMTL.h"

#include <limits>

namespace dawn::native::metal {
// The size of uniform buffer and storage buffer need to be aligned to 16 bytes which is the
// largest alignment of supported data types
static constexpr uint32_t kMinUniformOrStorageBufferAlignment = 16u;

// static
ResultOrError<Ref<Buffer>> Buffer::Create(Device* device, const BufferDescriptor* descriptor) {
    Ref<Buffer> buffer = AcquireRef(new Buffer(device, descriptor));
    DAWN_TRY(buffer->Initialize(descriptor->mappedAtCreation));
    return std::move(buffer);
}

// static
uint64_t Buffer::QueryMaxBufferLength(id<MTLDevice> mtlDevice) {
    if (@available(iOS 12, tvOS 12, macOS 10.14, *)) {
        return [mtlDevice maxBufferLength];
    }

    // Earlier versions of Metal had maximums defined in the Metal feature set tables
    // https://metalbyexample.com/wp-content/uploads/Metal-Feature-Set-Tables-2018.pdf
#if DAWN_PLATFORM_IS(MACOS)
    // 10.12 and 10.13 have a 1Gb limit.
    if (@available(macOS 10.12, *)) {
        // |maxBufferLength| isn't always available on older systems. If available, use
        // |recommendedMaxWorkingSetSize| instead. We can probably allocate more than this,
        // but don't have a way to discover a better limit. MoltenVK also uses this heuristic.
        return 1024 * 1024 * 1024;
    }
    // 10.11 has a 256Mb limit
    if (@available(macOS 10.11, *)) {
        return 256 * 1024 * 1024;
    }
    // 256Mb for other platform if any. (Need to have a return for all branches).
    return 256 * 1024 * 1024;
#else
    // macOS / tvOS: 256Mb limit in versions without [MTLDevice maxBufferLength]
    return 256 * 1024 * 1024;
#endif
}

Buffer::Buffer(DeviceBase* dev, const BufferDescriptor* desc) : BufferBase(dev, desc) {}

MaybeError Buffer::Initialize(bool mappedAtCreation) {
    MTLResourceOptions storageMode;
    if (GetUsage() & kMappableBufferUsages) {
        storageMode = MTLResourceStorageModeShared;
    } else {
        storageMode = MTLResourceStorageModePrivate;
    }

    uint32_t alignment = 1;
#if DAWN_PLATFORM_IS(MACOS)
    // [MTLBlitCommandEncoder fillBuffer] requires the size to be a multiple of 4 on MacOS.
    alignment = 4;
#endif

    // Metal validation layer requires the size of uniform buffer and storage buffer to be no
    // less than the size of the buffer block defined in shader, and the overall size of the
    // buffer must be aligned to the largest alignment of its members.
    if (GetUsage() &
        (wgpu::BufferUsage::Uniform | wgpu::BufferUsage::Storage | kInternalStorageBuffer)) {
        ASSERT(IsAligned(kMinUniformOrStorageBufferAlignment, alignment));
        alignment = kMinUniformOrStorageBufferAlignment;
    }

    // The vertex pulling transform requires at least 4 bytes in the buffer.
    // 0-sized vertex buffer bindings are allowed, so we always need an additional 4 bytes
    // after the end.
    NSUInteger extraBytes = 0u;
    if ((GetUsage() & wgpu::BufferUsage::Vertex) != 0) {
        extraBytes = 4u;
    }

    if (GetSize() > std::numeric_limits<NSUInteger>::max() - extraBytes) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    NSUInteger currentSize =
        std::max(static_cast<NSUInteger>(GetSize()) + extraBytes, NSUInteger(4));

    if (currentSize > std::numeric_limits<NSUInteger>::max() - alignment) {
        // Alignment would overlow.
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }
    currentSize = Align(currentSize, alignment);

    uint64_t maxBufferSize = QueryMaxBufferLength(ToBackend(GetDevice())->GetMTLDevice());
    if (currentSize > maxBufferSize) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation is too large");
    }

    mAllocatedSize = currentSize;
    mMtlBuffer.Acquire([ToBackend(GetDevice())->GetMTLDevice() newBufferWithLength:currentSize
                                                                           options:storageMode]);
    if (mMtlBuffer == nullptr) {
        return DAWN_OUT_OF_MEMORY_ERROR("Buffer allocation failed");
    }

    // The buffers with mappedAtCreation == true will be initialized in
    // BufferBase::MapAtCreation().
    if (GetDevice()->IsToggleEnabled(Toggle::NonzeroClearResourcesOnCreationForTesting) &&
        !mappedAtCreation) {
        CommandRecordingContext* commandContext =
            ToBackend(GetDevice())->GetPendingCommandContext();
        ClearBuffer(commandContext, uint8_t(1u));
    }

    // Initialize the padding bytes to zero.
    if (GetDevice()->IsToggleEnabled(Toggle::LazyClearResourceOnFirstUse) && !mappedAtCreation) {
        uint32_t paddingBytes = GetAllocatedSize() - GetSize();
        if (paddingBytes > 0) {
            uint32_t clearSize = Align(paddingBytes, 4);
            uint64_t clearOffset = GetAllocatedSize() - clearSize;

            CommandRecordingContext* commandContext =
                ToBackend(GetDevice())->GetPendingCommandContext();
            ClearBuffer(commandContext, 0, clearOffset, clearSize);
        }
    }
    return {};
}

Buffer::~Buffer() = default;

id<MTLBuffer> Buffer::GetMTLBuffer() const {
    return mMtlBuffer.Get();
}

bool Buffer::IsCPUWritableAtCreation() const {
    // TODO(enga): Handle CPU-visible memory on UMA
    return GetUsage() & kMappableBufferUsages;
}

MaybeError Buffer::MapAtCreationImpl() {
    return {};
}

MaybeError Buffer::MapAsyncImpl(wgpu::MapMode mode, size_t offset, size_t size) {
    CommandRecordingContext* commandContext = ToBackend(GetDevice())->GetPendingCommandContext();
    EnsureDataInitialized(commandContext);

    return {};
}

void* Buffer::GetMappedPointer() {
    return [*mMtlBuffer contents];
}

void Buffer::UnmapImpl() {
    // Nothing to do, Metal StorageModeShared buffers are always mapped.
}

void Buffer::DestroyImpl() {
    BufferBase::DestroyImpl();
    mMtlBuffer = nullptr;
}

void Buffer::TrackUsage() {
    MarkUsedInPendingCommands();
}

bool Buffer::EnsureDataInitialized(CommandRecordingContext* commandContext) {
    if (!NeedsInitialization()) {
        return false;
    }

    InitializeToZero(commandContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                uint64_t offset,
                                                uint64_t size) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferRange(offset, size)) {
        SetIsDataInitialized();
        return false;
    }

    InitializeToZero(commandContext);
    return true;
}

bool Buffer::EnsureDataInitializedAsDestination(CommandRecordingContext* commandContext,
                                                const CopyTextureToBufferCmd* copy) {
    if (!NeedsInitialization()) {
        return false;
    }

    if (IsFullBufferOverwrittenInTextureToBufferCopy(copy)) {
        SetIsDataInitialized();
        return false;
    }

    InitializeToZero(commandContext);
    return true;
}

void Buffer::InitializeToZero(CommandRecordingContext* commandContext) {
    ASSERT(NeedsInitialization());

    ClearBuffer(commandContext, uint8_t(0u));

    SetIsDataInitialized();
    GetDevice()->IncrementLazyClearCountForTesting();
}

void Buffer::ClearBuffer(CommandRecordingContext* commandContext,
                         uint8_t clearValue,
                         uint64_t offset,
                         uint64_t size) {
    ASSERT(commandContext != nullptr);
    size = size > 0 ? size : GetAllocatedSize();
    ASSERT(size > 0);
    TrackUsage();
    [commandContext->EnsureBlit() fillBuffer:mMtlBuffer.Get()
                                       range:NSMakeRange(offset, size)
                                       value:clearValue];
}

}  // namespace dawn::native::metal
