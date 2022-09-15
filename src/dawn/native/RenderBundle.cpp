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

#include "dawn/native/RenderBundle.h"

#include <utility>

#include "dawn/common/BitSetIterator.h"
#include "dawn/native/Commands.h"
#include "dawn/native/Device.h"
#include "dawn/native/ObjectType_autogen.h"
#include "dawn/native/RenderBundleEncoder.h"

namespace dawn::native {

RenderBundleBase::RenderBundleBase(RenderBundleEncoder* encoder,
                                   const RenderBundleDescriptor* descriptor,
                                   Ref<AttachmentState> attachmentState,
                                   bool depthReadOnly,
                                   bool stencilReadOnly,
                                   RenderPassResourceUsage resourceUsage,
                                   IndirectDrawMetadata indirectDrawMetadata)
    : ApiObjectBase(encoder->GetDevice(), kLabelNotImplemented),
      mCommands(encoder->AcquireCommands()),
      mIndirectDrawMetadata(std::move(indirectDrawMetadata)),
      mAttachmentState(std::move(attachmentState)),
      mDepthReadOnly(depthReadOnly),
      mStencilReadOnly(stencilReadOnly),
      mDrawCount(encoder->GetDrawCount()),
      mResourceUsage(std::move(resourceUsage)) {
    GetObjectTrackingList()->Track(this);
}

void RenderBundleBase::DestroyImpl() {
    FreeCommands(&mCommands);

    // Remove reference to the attachment state so that we don't have lingering references to
    // it preventing it from being uncached in the device.
    mAttachmentState = nullptr;
}

// static
RenderBundleBase* RenderBundleBase::MakeError(DeviceBase* device) {
    return new RenderBundleBase(device, ObjectBase::kError);
}

RenderBundleBase::RenderBundleBase(DeviceBase* device, ErrorTag errorTag)
    : ApiObjectBase(device, errorTag), mIndirectDrawMetadata(device->GetLimits()) {}

ObjectType RenderBundleBase::GetType() const {
    return ObjectType::RenderBundle;
}

CommandIterator* RenderBundleBase::GetCommands() {
    return &mCommands;
}

const AttachmentState* RenderBundleBase::GetAttachmentState() const {
    ASSERT(!IsError());
    return mAttachmentState.Get();
}

bool RenderBundleBase::IsDepthReadOnly() const {
    ASSERT(!IsError());
    return mDepthReadOnly;
}

bool RenderBundleBase::IsStencilReadOnly() const {
    ASSERT(!IsError());
    return mStencilReadOnly;
}

uint64_t RenderBundleBase::GetDrawCount() const {
    ASSERT(!IsError());
    return mDrawCount;
}

const RenderPassResourceUsage& RenderBundleBase::GetResourceUsage() const {
    ASSERT(!IsError());
    return mResourceUsage;
}

const IndirectDrawMetadata& RenderBundleBase::GetIndirectDrawMetadata() {
    return mIndirectDrawMetadata;
}

}  // namespace dawn::native
