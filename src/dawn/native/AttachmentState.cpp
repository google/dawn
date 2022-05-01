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

#include "dawn/native/AttachmentState.h"

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/Texture.h"

namespace dawn::native {

AttachmentStateBlueprint::AttachmentStateBlueprint(const RenderBundleEncoderDescriptor* descriptor)
    : mSampleCount(descriptor->sampleCount) {
    ASSERT(descriptor->colorFormatsCount <= kMaxColorAttachments);
    for (ColorAttachmentIndex i(uint8_t(0));
         i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->colorFormatsCount)); ++i) {
        wgpu::TextureFormat format = descriptor->colorFormats[static_cast<uint8_t>(i)];
        if (format != wgpu::TextureFormat::Undefined) {
            mColorAttachmentsSet.set(i);
            mColorFormats[i] = format;
        }
    }
    mDepthStencilFormat = descriptor->depthStencilFormat;
}

AttachmentStateBlueprint::AttachmentStateBlueprint(const RenderPipelineDescriptor* descriptor)
    : mSampleCount(descriptor->multisample.count) {
    if (descriptor->fragment != nullptr) {
        ASSERT(descriptor->fragment->targetCount <= kMaxColorAttachments);
        for (ColorAttachmentIndex i(uint8_t(0));
             i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->fragment->targetCount));
             ++i) {
            wgpu::TextureFormat format =
                descriptor->fragment->targets[static_cast<uint8_t>(i)].format;
            if (format != wgpu::TextureFormat::Undefined) {
                mColorAttachmentsSet.set(i);
                mColorFormats[i] = format;
            }
        }
    }
    if (descriptor->depthStencil != nullptr) {
        mDepthStencilFormat = descriptor->depthStencil->format;
    }
}

AttachmentStateBlueprint::AttachmentStateBlueprint(const RenderPassDescriptor* descriptor) {
    for (ColorAttachmentIndex i(uint8_t(0));
         i < ColorAttachmentIndex(static_cast<uint8_t>(descriptor->colorAttachmentCount)); ++i) {
        TextureViewBase* attachment = descriptor->colorAttachments[static_cast<uint8_t>(i)].view;
        if (attachment == nullptr) {
            continue;
        }
        mColorAttachmentsSet.set(i);
        mColorFormats[i] = attachment->GetFormat().format;
        if (mSampleCount == 0) {
            mSampleCount = attachment->GetTexture()->GetSampleCount();
        } else {
            ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
        }
    }
    if (descriptor->depthStencilAttachment != nullptr) {
        TextureViewBase* attachment = descriptor->depthStencilAttachment->view;
        mDepthStencilFormat = attachment->GetFormat().format;
        if (mSampleCount == 0) {
            mSampleCount = attachment->GetTexture()->GetSampleCount();
        } else {
            ASSERT(mSampleCount == attachment->GetTexture()->GetSampleCount());
        }
    }
    ASSERT(mSampleCount > 0);
}

AttachmentStateBlueprint::AttachmentStateBlueprint(const AttachmentStateBlueprint& rhs) = default;

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

bool AttachmentStateBlueprint::EqualityFunc::operator()(const AttachmentStateBlueprint* a,
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
    : AttachmentStateBlueprint(blueprint), ObjectBase(device) {}

AttachmentState::~AttachmentState() {
    GetDevice()->UncacheAttachmentState(this);
}

size_t AttachmentState::ComputeContentHash() {
    // TODO(dawn:549): skip this traversal and reuse the blueprint.
    return AttachmentStateBlueprint::HashFunc()(this);
}

ityp::bitset<ColorAttachmentIndex, kMaxColorAttachments> AttachmentState::GetColorAttachmentsMask()
    const {
    return mColorAttachmentsSet;
}

wgpu::TextureFormat AttachmentState::GetColorAttachmentFormat(ColorAttachmentIndex index) const {
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

}  // namespace dawn::native
