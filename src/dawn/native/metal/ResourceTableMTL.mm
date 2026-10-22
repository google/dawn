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

#include "src/dawn/native/metal/ResourceTableMTL.h"

#include "src/dawn/common/Enumerator.h"
#include "src/dawn/common/MatchVariant.h"
#include "src/dawn/native/DynamicUploader.h"
#include "src/dawn/native/metal/BufferMTL.h"
#include "src/dawn/native/metal/DeviceMTL.h"
#include "src/dawn/native/metal/SamplerMTL.h"
#include "src/dawn/native/metal/UtilsMetal.h"

namespace dawn::native::metal {

// static
ResultOrError<Ref<ResourceTable>> ResourceTable::Create(Device* device,
                                                        const ResourceTableDescriptor* descriptor) {
    Ref<ResourceTable> table = AcquireRef(new ResourceTable(device, descriptor));
    DAWN_TRY(table->Initialize());
    return table;
}

ResourceTable::~ResourceTable() = default;

MaybeError ResourceTable::Initialize() {
    DAWN_TRY(ResourceTableBase::InitializeBase());

    Device* device = ToBackend(GetDevice());
    NSUInteger argBufferLength = size_t(GetSizeWithDefaultResources()) * sizeof(MTLResourceID);

    // TODO(crbug.com/473444514): This should use `ModePrivate` over `ModeShared` as private would
    // be faster.
    // TODO(crbug.com/473444514): Investigate turning this into a WebGPU Buffer, then below we can
    // use the `GetMappedSpan` and remove the unsafe buffer.
    mArgBuffer =
        AcquireNSPRef([device->GetMTLDevice() newBufferWithLength:argBufferLength
                                                          options:MTLResourceStorageModeShared]);
    if (mArgBuffer == nil) {
        return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate resource table argument buffer.");
    }

    mTextureViews.resize(GetSizeWithDefaultResources());

    SetLabelImpl();

    return {};
}

MaybeError ResourceTable::ApplyPendingUpdates(
    CommandRecordingContext* recordingContext,
    const absl::flat_hash_set<TextureBase*>& writableTextures) {
    DAWN_TRY(ApplyDirtySlotUpdatesWith(
        writableTextures,
        [&](const std::vector<MetadataUpdate>& metadataUpdates,
            const std::vector<ResourceDiff>& resourceDiffs,
            const absl::flat_hash_set<raw_ptr<TextureBase>>& texturesToTransition) -> MaybeError {
            // Transition and initialize all required textures
            if (!texturesToTransition.empty()) {
                DAWN_TRY(TransitionResources(recordingContext, texturesToTransition));
            }
            if (!metadataUpdates.empty()) {
                DAWN_TRY(UpdateMetadataBuffer(recordingContext, metadataUpdates));
            }
            if (!resourceDiffs.empty()) {
                DAWN_TRY(UpdateResourceBindings(resourceDiffs));
            }
            return {};
        }));
    return {};
}

MaybeError ResourceTable::TransitionResources(
    CommandRecordingContext* recordingContext,
    const absl::flat_hash_set<raw_ptr<TextureBase>>& textures) {
    for (const auto& texture : textures) {
        Texture* textureBackend = ToBackend(texture);
        textureBackend->SynchronizeTextureBeforeUse(recordingContext);
        DAWN_TRY(textureBackend->EnsureSubresourceContentInitialized(
            recordingContext, texture->GetAllSubresources()));
    }
    return {};
}

MaybeError ResourceTable::UpdateMetadataBuffer(CommandRecordingContext* recordingContext,
                                               const std::vector<MetadataUpdate>& updates) {
    DAWN_ASSERT(!updates.empty());

    Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());

    return GetDevice()->GetDynamicUploader()->WithUploadReservation(
        sizeof(uint32_t) * updates.size(), kCopyBufferToBufferOffsetAlignment,
        [&](UploadReservation reservation) -> MaybeError {
            Span<uint32_t> stagedData = ReinterpretSpan<uint32_t>(reservation.mappedData);
            id<MTLBuffer> uploadBuffer = ToBackend(reservation.buffer)->GetMTLBuffer();
            id<MTLBuffer> dstBuffer = metadataBuffer->GetMTLBuffer();
            id<MTLBlitCommandEncoder> blit = recordingContext->EnsureBlit();

            for (auto [i, update] : Enumerate(updates)) {
                DAWN_ASSERT((update.data & 0xFFFF0000) == 0);  // Only low 16 bits used for type id

                stagedData[i] = update.data;

                [blit copyFromBuffer:uploadBuffer
                         sourceOffset:reservation.offsetInBuffer + i * sizeof(uint32_t)
                             toBuffer:dstBuffer
                    destinationOffset:update.offset
                                 size:sizeof(uint32_t)];
            }
            return {};
        });
}

MaybeError ResourceTable::UpdateResourceBindings(const std::vector<ResourceDiff>& diffs) {
    // SAFETY: The mArgBuffer was created sized to the correct number of slots.
    Span<MTLResourceID> contents = DAWN_UNSAFE_BUFFERS(  //
        {static_cast<MTLResourceID*>([*mArgBuffer contents]),
         size_t(GetSizeWithDefaultResources())});

    for (const ResourceDiff& diff : diffs) {
        auto slot = uint32_t(diff.slot);

        MatchVariant(
            diff.added,
            [&](std::monostate) {
                contents[slot] = MTLResourceID{0};
                mTextureViews[diff.slot] = nullptr;
            },
            [&](Ref<TextureViewBase> textureView) {
                // Skip updating the descriptor if the texture is already destroyed; the slot is
                // marked as empty in the metadata buffer and the partially-bound descriptor array
                // does not require a valid descriptor for unused slots.
                if (textureView->GetTexture()->IsDestroyed()) {
                    return;
                }

                Ref<TextureView> handle = ToBackend(textureView);
                DAWN_CHECK(handle != nullptr);

                id<MTLTexture> texture = handle->GetMTLTexture();
                contents[slot] = [texture gpuResourceID];

                mTextureViews[diff.slot] = std::move(handle);
            },
            [&](Ref<SamplerBase> sampler) {
                mTextureViews[diff.slot] = nullptr;

                Ref<Sampler> handle = ToBackend(sampler);
                DAWN_CHECK(handle != nullptr);

                id<MTLSamplerState> samplerState = handle->GetMTLSamplerState();
                contents[slot] = [samplerState gpuResourceID];
            });
    }

    return {};
}

void ResourceTable::UseResources(id<MTLComputeCommandEncoder> encoder) {
    Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());
    auto scopedUse = metadataBuffer->UseInternal();
    metadataBuffer->TrackUsage();

    for (const Ref<TextureView>& view : mTextureViews) {
        if (view != nullptr && !view->GetTexture()->IsDestroyed() &&
            view->GetTexture()->HasAccess()) {
            [encoder useResource:view->GetMTLTexture() usage:MTLResourceUsageRead];
        }
    }
}

void ResourceTable::UseResources(id<MTLRenderCommandEncoder> encoder) {
    Buffer* metadataBuffer = ToBackend(GetMetadataBuffer());
    auto scopedUse = metadataBuffer->UseInternal();
    metadataBuffer->TrackUsage();

    for (const Ref<TextureView>& view : mTextureViews) {
        if (view != nullptr && !view->GetTexture()->IsDestroyed() &&
            view->GetTexture()->HasAccess()) {
            [encoder useResource:view->GetMTLTexture()
                           usage:MTLResourceUsageRead
                          stages:MTLRenderStageVertex | MTLRenderStageFragment];
        }
    }
}

id<MTLBuffer> ResourceTable::GetArgBuffer() const {
    return mArgBuffer.Get();
}

void ResourceTable::DestroyImpl(DestroyReason reason) {
    mArgBuffer = nullptr;
    mTextureViews.clear();
    ResourceTableBase::DestroyImpl(reason);
}

void ResourceTable::SetLabelImpl() {
    SetDebugName(GetDevice(), mArgBuffer.Get(), "Dawn_ResourceTable", GetLabel());
}

}  // namespace dawn::native::metal
