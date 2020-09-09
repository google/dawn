// Copyright 2019 The Dawn Authors
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

#include "dawn_native/AttachmentState.h"

#include "common/BitSetIterator.h"
#include "common/HashUtils.h"
#include "dawn_native/Device.h"
#include "dawn_native/Texture.h"

namespace dawn_native {

    AttachmentStateBlueprint::AttachmentStateBlueprint(
        const RenderBundleEncoderDescriptor* descriptor)
        : mSampleCount(descriptor->sampleCount) {
        ASSERT(descriptor->colorFormatsCount <= kMaxColorAttachments);
        for (ColorAttachmentIndex i(uint8_t(0));
             i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->colorFormatsCount)); ++i) {
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = descriptor->colorFormats[static_cast<uint8_t>(i)];
        }
        mDepthStencilFormat = descriptor->depthStencilFormat;
    }

    AttachmentStateBlueprint::AttachmentStateBlueprint(const RenderPipelineDescriptor* descriptor)
        : mSampleCount(descriptor->sampleCount) {
        ASSERT(descriptor->colorStateCount <= kMaxColorAttachments);
        for (ColorAttachmentIndex i(uint8_t(0));
             i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->colorStateCount)); ++i) {
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = descriptor->colorStates[static_cast<uint8_t>(i)].format;
        }
        if (descriptor->depthStencilState != nullptr) {
            mDepthStencilFormat = descriptor->depthStencilState->format;
        }
    }

    AttachmentStateBlueprint::AttachmentStateBlueprint(const RenderPassDescriptor* descriptor) {
        for (ColorAttachmentIndex i(uint8_t(0));
             i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->colorAttachmentCount));
             ++i) {
            TextureViewBase* attachment =
                descriptor->colorAttachments[static_cast<uint8_t>(i)].attachment;
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = attachment->GetFormat().format;
            if (mSampleCount == 0) {
                mSampleCount = attachment->GetTexture()->GetSampleCount();
            } else {
                ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
            }
        }
        if (descriptor->depthStencilAttachment != nullptr) {
            TextureViewBase* attachment = descriptor->depthStencilAttachment->attachment;
            mDepthStencilFormat = attachment->GetFormat().format;
            if (mSampleCount == 0) {
                mSampleCount = attachment->GetTexture()->GetSampleCount();
            } else {
                ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
            }
        }
        ASSERT(mSampleCount > 0);
    }

    AttachmentStateBlueprint::AttachmentStateBlueprint(const AttachmentStateBlueprint& rhs) =
        default;

    size_t AttachmentStateBlueprint::HashFunc::operator()(
        const AttachmentStateBlueprint* attachmentState) const {
        size_t hash = 0;

        // Hash color formats
        HashCombine(&hash, attachmentState->mColorAttachmentsSet);
        for (ColorAttachmentIndex i : IterateBitSet(attachmentState->mColorAttachmentsSet)) {
            HashCombine(&hash, attachmentState->mColorFormats[i]);
        }

        // Hash depth stencil attachment
        HashCombine(&hash, attachmentState->mDepthStencilFormat);

        // Hash sample count
        HashCombine(&hash, attachmentState->mSampleCount);

        return hash;
    }

    bool AttachmentStateBlueprint::EqualityFunc::operator()(
        const AttachmentStateBlueprint* a,
        const AttachmentStateBlueprint* b) const {
        // Check set attachments
        if (a->mColorAttachmentsSet != b->mColorAttachmentsSet) {
            return false;
        }

        // Check color formats
        for (ColorAttachmentIndex i : IterateBitSet(a->mColorAttachmentsSet)) {
            if (a->mColorFormats[i] != b->mColorFormats[i]) {
                return false;
            }
        }

        // Check depth stencil format
        if (a->mDepthStencilFormat != b->mDepthStencilFormat) {
            return false;
        }

        // Check sample count
        if (a->mSampleCount != b->mSampleCount) {
            return false;
        }

        return true;
    }

    AttachmentState::AttachmentState(DeviceBase* device, const AttachmentStateBlueprint& blueprint)
        : AttachmentStateBlueprint(blueprint), CachedObject(device) {
    }

    AttachmentState::~AttachmentState() {
        GetDevice()->UncacheAttachmentState(this);
    }

    ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments>
    AttachmentState::GetColorAttachmentsMask() const {
        return mColorAttachmentsSet;
    }

    wgpu::TextureFormat AttachmentState::GetColorAttachmentFormat(
        ColorAttachmentIndex index) const {
        ASSERT(mColorAttachmentsSet[index]);
        return mColorFormats[index];
    }

    bool AttachmentState::HasDepthStencilAttachment() const {
        return mDepthStencilFormat != wgpu::TextureFormat::Undefined;
    }

    wgpu::TextureFormat AttachmentState::GetDepthStencilFormat() const {
        ASSERT(HasDepthStencilAttachment());
        return mDepthStencilFormat;
    }

    uint32_t AttachmentState::GetSampleCount() const {
        return mSampleCount;
    }

}  // namespace dawn_native
