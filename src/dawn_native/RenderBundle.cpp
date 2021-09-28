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

#include "dawn_native/RenderBundle.h"

#include "common/BitSetIterator.h"
#include "dawn_native/Commands.h"
#include "dawn_native/Device.h"
#include "dawn_native/ObjectType_autogen.h"
#include "dawn_native/RenderBundleEncoder.h"

namespace dawn_native {

    RenderBundleBase::RenderBundleBase(RenderBundleEncoder* encoder,
                                       const RenderBundleDescriptor* descriptor,
                                       Ref<AttachmentState> attachmentState,
                                       RenderPassResourceUsage resourceUsage,
                                       IndirectDrawMetadata indirectDrawMetadata)
        : ApiObjectBase(encoder->GetDevice(), kLabelNotImplemented),
          mCommands(encoder->AcquireCommands()),
          mIndirectDrawMetadata(std::move(indirectDrawMetadata)),
          mAttachmentState(std::move(attachmentState)),
          mResourceUsage(std::move(resourceUsage)) {
    }

    RenderBundleBase::~RenderBundleBase() {
        FreeCommands(&mCommands);
    }

    // static
    RenderBundleBase* RenderBundleBase::MakeError(DeviceBase* device) {
        return new RenderBundleBase(device, ObjectBase::kError);
    }

    RenderBundleBase::RenderBundleBase(DeviceBase* device, ErrorTag errorTag)
        : ApiObjectBase(device, errorTag) {
    }

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

    const RenderPassResourceUsage& RenderBundleBase::GetResourceUsage() const {
        ASSERT(!IsError());
        return mResourceUsage;
    }

    const IndirectDrawMetadata& RenderBundleBase::GetIndirectDrawMetadata() {
        return mIndirectDrawMetadata;
    }

}  // namespace dawn_native
