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
        : mHasDepthStencilAttachment(descriptor->depthStencilFormat != nullptr),
          mSampleCount(descriptor->sampleCount) {
        for (uint32_t i = 0; i < descriptor->colorFormatsCount; ++i) {
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = descriptor->colorFormats[i];
        }
        if (mHasDepthStencilAttachment) {
            mDepthStencilFormat = *descriptor->depthStencilFormat;
        }
    }

    AttachmentStateBlueprint::AttachmentStateBlueprint(const RenderPipelineDescriptor* descriptor)
        : mHasDepthStencilAttachment(descriptor->depthStencilState != nullptr),
          mSampleCount(descriptor->sampleCount) {
        for (uint32_t i = 0; i < descriptor->colorStateCount; ++i) {
            ASSERT(descriptor->colorStates[i] != nullptr);
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = descriptor->colorStates[i]->format;
        }
        if (mHasDepthStencilAttachment) {
            mDepthStencilFormat = descriptor->depthStencilState->format;
        }
    }

    AttachmentStateBlueprint::AttachmentStateBlueprint(const RenderPassDescriptor* descriptor)
        : mHasDepthStencilAttachment(descriptor->depthStencilAttachment != nullptr) {
        for (uint32_t i = 0; i < descriptor->colorAttachmentCount; ++i) {
            TextureViewBase* attachment = descriptor->colorAttachments[i]->attachment;
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = attachment->GetFormat().format;
            if (mSampleCount == 0) {
                mSampleCount = attachment->GetTexture()->GetSampleCount();
            } else {
                ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
            }
        }
        if (mHasDepthStencilAttachment) {
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
        for (uint32_t i : IterateBitSet(attachmentState->mColorAttachmentsSet)) {
            HashCombine(&hash, attachmentState->mColorFormats[i]);
        }

        // Hash depth stencil attachments
        if (attachmentState->mHasDepthStencilAttachment) {
            HashCombine(&hash, attachmentState->mDepthStencilFormat);
        }

        // Hash sample count
        HashCombine(&hash, attachmentState->mSampleCount);

        return hash;
    }

    bool AttachmentStateBlueprint::EqualityFunc::operator()(
        const AttachmentStateBlueprint* a,
        const AttachmentStateBlueprint* b) const {
        // Check set attachments
        if (a->mColorAttachmentsSet != b->mColorAttachmentsSet ||
            a->mHasDepthStencilAttachment != b->mHasDepthStencilAttachment) {
            return false;
        }

        // Check color formats
        for (uint32_t i : IterateBitSet(a->mColorAttachmentsSet)) {
            if (a->mColorFormats[i] != b->mColorFormats[i]) {
                return false;
            }
        }

        // Check depth stencil format
        if (a->mHasDepthStencilAttachment) {
            if (a->mDepthStencilFormat != b->mDepthStencilFormat) {
                return false;
            }
        }

        // Check sample count
        if (a->mSampleCount != b->mSampleCount) {
            return false;
        }

        return true;
    }

    AttachmentState::AttachmentState(DeviceBase* device, const AttachmentStateBlueprint& blueprint)
        : AttachmentStateBlueprint(blueprint), RefCounted(), mDevice(device) {
    }

    AttachmentState::~AttachmentState() {
        mDevice->UncacheAttachmentState(this);
    }

    std::bitset<kMaxColorAttachments> AttachmentState::GetColorAttachmentsMask() const {
        return mColorAttachmentsSet;
    }

    dawn::TextureFormat AttachmentState::GetColorAttachmentFormat(uint32_t index) const {
        ASSERT(mColorAttachmentsSet[index]);
        return mColorFormats[index];
    }

    bool AttachmentState::HasDepthStencilAttachment() const {
        return mHasDepthStencilAttachment;
    }

    dawn::TextureFormat AttachmentState::GetDepthStencilFormat() const {
        ASSERT(mHasDepthStencilAttachment);
        return mDepthStencilFormat;
    }

    uint32_t AttachmentState::GetSampleCount() const {
        return mSampleCount;
    }

}  // namespace dawn_native
