// Copyright 2019 The Dawn & Tint Authors
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

#include "dawn/native/PassResourceUsageTracker.h"

#include <utility>

#include "dawn/native/BindGroup.h"
#include "dawn/native/Buffer.h"
#include "dawn/native/EnumMaskIterator.h"
#include "dawn/native/ExternalTexture.h"
#include "dawn/native/Format.h"
#include "dawn/native/QuerySet.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

SyncScopeUsageTracker::SyncScopeUsageTracker() = default;

SyncScopeUsageTracker::SyncScopeUsageTracker(SyncScopeUsageTracker&&) = default;

SyncScopeUsageTracker::~SyncScopeUsageTracker() = default;

SyncScopeUsageTracker& SyncScopeUsageTracker::operator=(SyncScopeUsageTracker&&) = default;

void SyncScopeUsageTracker::BufferUsedAs(BufferBase* buffer, wgpu::BufferUsage usage) {
    // std::map's operator[] will create the key and return 0 if the key didn't exist
    // before.
    mBufferUsages[buffer] |= usage;
}

void SyncScopeUsageTracker::TextureViewUsedAs(TextureViewBase* view, wgpu::TextureUsage usage) {
    TextureRangeUsedAs(view->GetTexture(), view->GetSubresourceRange(), usage);
}

void SyncScopeUsageTracker::TextureRangeUsedAs(TextureBase* texture,
                                               const SubresourceRange& range,
                                               wgpu::TextureUsage usage) {
    // Get or create a new TextureSubresourceUsage for that texture (initially filled with
    // wgpu::TextureUsage::None)
    auto it = mTextureUsages.emplace(
        std::piecewise_construct, std::forward_as_tuple(texture),
        std::forward_as_tuple(texture->GetFormat().aspects, texture->GetArrayLayers(),
                              texture->GetNumMipLevels(), wgpu::TextureUsage::None));
    TextureSubresourceUsage& textureUsage = it.first->second;

    textureUsage.Update(range, [usage](const SubresourceRange&, wgpu::TextureUsage* storedUsage) {
        // TODO(crbug.com/dawn/1001): Consider optimizing to have fewer branches.

        // Using the same subresource for two different attachments is a write-write or read-write
        // hazard. Add an internal kAgainAsAttachment usage to fail the later check that a
        // subresource with a writable usage has a single usage.
        constexpr wgpu::TextureUsage kAgainAsAttachment =
            kReservedTextureUsage | static_cast<wgpu::TextureUsage>(1);
        constexpr wgpu::TextureUsage kWritableAttachmentUsages =
            wgpu::TextureUsage::RenderAttachment | wgpu::TextureUsage::StorageAttachment;
        if ((usage & kWritableAttachmentUsages) && (*storedUsage & kWritableAttachmentUsages)) {
            *storedUsage |= kAgainAsAttachment;
        }

        *storedUsage |= usage;
    });
}

void SyncScopeUsageTracker::AddRenderBundleTextureUsage(
    TextureBase* texture,
    const TextureSubresourceUsage& textureUsage) {
    // Get or create a new TextureSubresourceUsage for that texture (initially filled with
    // wgpu::TextureUsage::None)
    auto it = mTextureUsages.emplace(
        std::piecewise_construct, std::forward_as_tuple(texture),
        std::forward_as_tuple(texture->GetFormat().aspects, texture->GetArrayLayers(),
                              texture->GetNumMipLevels(), wgpu::TextureUsage::None));
    TextureSubresourceUsage* passTextureUsage = &it.first->second;

    passTextureUsage->Merge(
        textureUsage, [](const SubresourceRange&, wgpu::TextureUsage* storedUsage,
                         const wgpu::TextureUsage& addedUsage) {
            DAWN_ASSERT((addedUsage & wgpu::TextureUsage::RenderAttachment) == 0);
            *storedUsage |= addedUsage;
        });
}

void SyncScopeUsageTracker::AddBindGroup(BindGroupBase* group) {
    for (BindingIndex bindingIndex{0}; bindingIndex < group->GetLayout()->GetBindingCount();
         ++bindingIndex) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(bindingIndex);

        switch (bindingInfo.bindingType) {
            case BindingInfoType::Buffer: {
                BufferBase* buffer = group->GetBindingAsBufferBinding(bindingIndex).buffer;
                switch (bindingInfo.buffer.type) {
                    case wgpu::BufferBindingType::Uniform:
                        BufferUsedAs(buffer, wgpu::BufferUsage::Uniform);
                        break;
                    case wgpu::BufferBindingType::Storage:
                        BufferUsedAs(buffer, wgpu::BufferUsage::Storage);
                        break;
                    case kInternalStorageBufferBinding:
                        BufferUsedAs(buffer, kInternalStorageBuffer);
                        break;
                    case wgpu::BufferBindingType::ReadOnlyStorage:
                        BufferUsedAs(buffer, kReadOnlyStorageBuffer);
                        break;
                    case wgpu::BufferBindingType::Undefined:
                        DAWN_UNREACHABLE();
                }
                break;
            }

            case BindingInfoType::Texture: {
                TextureViewBase* view = group->GetBindingAsTextureView(bindingIndex);
                switch (bindingInfo.texture.sampleType) {
                    case kInternalResolveAttachmentSampleType:
                        TextureViewUsedAs(view, kResolveAttachmentLoadingUsage);
                        break;
                    default:
                        TextureViewUsedAs(view, wgpu::TextureUsage::TextureBinding);
                        break;
                }
                break;
            }

            case BindingInfoType::StorageTexture: {
                TextureViewBase* view = group->GetBindingAsTextureView(bindingIndex);
                switch (bindingInfo.storageTexture.access) {
                    case wgpu::StorageTextureAccess::WriteOnly:
                        TextureViewUsedAs(view, kWriteOnlyStorageTexture);
                        break;
                    case wgpu::StorageTextureAccess::ReadWrite:
                        TextureViewUsedAs(view, wgpu::TextureUsage::StorageBinding);
                        break;
                    case wgpu::StorageTextureAccess::ReadOnly:
                        TextureViewUsedAs(view, kReadOnlyStorageTexture);
                        break;
                    case wgpu::StorageTextureAccess::Undefined:
                        DAWN_UNREACHABLE();
                }
                break;
            }

            case BindingInfoType::ExternalTexture:
                DAWN_UNREACHABLE();
                break;

            case BindingInfoType::Sampler:
                break;
        }
    }

    for (const Ref<ExternalTextureBase>& externalTexture : group->GetBoundExternalTextures()) {
        mExternalTextureUsages.insert(externalTexture.Get());
    }
}

SyncScopeResourceUsage SyncScopeUsageTracker::AcquireSyncScopeUsage() {
    SyncScopeResourceUsage result;
    result.buffers.reserve(mBufferUsages.size());
    result.bufferUsages.reserve(mBufferUsages.size());
    result.textures.reserve(mTextureUsages.size());
    result.textureUsages.reserve(mTextureUsages.size());

    for (auto& [buffer, usage] : mBufferUsages) {
        result.buffers.push_back(buffer);
        result.bufferUsages.push_back(usage);
    }

    for (auto& [texture, usage] : mTextureUsages) {
        result.textures.push_back(texture);
        result.textureUsages.push_back(std::move(usage));
    }

    for (auto* const it : mExternalTextureUsages) {
        result.externalTextures.push_back(it);
    }

    mBufferUsages.clear();
    mTextureUsages.clear();
    mExternalTextureUsages.clear();

    return result;
}

ComputePassResourceUsageTracker::ComputePassResourceUsageTracker() = default;

ComputePassResourceUsageTracker::~ComputePassResourceUsageTracker() = default;

void ComputePassResourceUsageTracker::AddDispatch(SyncScopeResourceUsage scope) {
    mUsage.dispatchUsages.push_back(std::move(scope));
}

void ComputePassResourceUsageTracker::AddReferencedBuffer(BufferBase* buffer) {
    mUsage.referencedBuffers.insert(buffer);
}

void ComputePassResourceUsageTracker::AddResourcesReferencedByBindGroup(BindGroupBase* group) {
    for (BindingIndex index{0}; index < group->GetLayout()->GetBindingCount(); ++index) {
        const BindingInfo& bindingInfo = group->GetLayout()->GetBindingInfo(index);

        switch (bindingInfo.bindingType) {
            case BindingInfoType::Buffer: {
                mUsage.referencedBuffers.insert(group->GetBindingAsBufferBinding(index).buffer);
                break;
            }

            case BindingInfoType::Texture: {
                mUsage.referencedTextures.insert(
                    group->GetBindingAsTextureView(index)->GetTexture());
                break;
            }

            case BindingInfoType::ExternalTexture:
                DAWN_UNREACHABLE();
            case BindingInfoType::StorageTexture:
            case BindingInfoType::Sampler:
                break;
        }
    }

    for (const Ref<ExternalTextureBase>& externalTexture : group->GetBoundExternalTextures()) {
        mUsage.referencedExternalTextures.insert(externalTexture.Get());
    }
}

ComputePassResourceUsage ComputePassResourceUsageTracker::AcquireResourceUsage() {
    return std::move(mUsage);
}

RenderPassResourceUsageTracker::RenderPassResourceUsageTracker() = default;

RenderPassResourceUsageTracker::RenderPassResourceUsageTracker(RenderPassResourceUsageTracker&&) =
    default;

RenderPassResourceUsageTracker::~RenderPassResourceUsageTracker() = default;

RenderPassResourceUsageTracker& RenderPassResourceUsageTracker::operator=(
    RenderPassResourceUsageTracker&&) = default;

RenderPassResourceUsage RenderPassResourceUsageTracker::AcquireResourceUsage() {
    RenderPassResourceUsage result;
    *static_cast<SyncScopeResourceUsage*>(&result) = AcquireSyncScopeUsage();

    result.querySets.reserve(mQueryAvailabilities.size());
    result.queryAvailabilities.reserve(mQueryAvailabilities.size());

    for (auto& it : mQueryAvailabilities) {
        result.querySets.push_back(it.first);
        result.queryAvailabilities.push_back(std::move(it.second));
    }

    mQueryAvailabilities.clear();

    return result;
}

void RenderPassResourceUsageTracker::TrackQueryAvailability(QuerySetBase* querySet,
                                                            uint32_t queryIndex) {
    // The query availability only needs to be tracked again on render passes for checking
    // query overwrite on render pass and resetting query sets on the Vulkan backend.
    DAWN_ASSERT(querySet != nullptr);

    // Gets the iterator for that querySet or create a new vector of bool set to false
    // if the querySet wasn't registered.
    auto it = mQueryAvailabilities.emplace(querySet, querySet->GetQueryCount()).first;
    it->second[queryIndex] = true;
}

const QueryAvailabilityMap& RenderPassResourceUsageTracker::GetQueryAvailabilityMap() const {
    return mQueryAvailabilities;
}

}  // namespace dawn::native
