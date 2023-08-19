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

#include "dawn/native/metal/SharedFenceMTL.h"

#include "dawn/native/ChainUtils_autogen.h"
#include "dawn/native/metal/DeviceMTL.h"

namespace dawn::native::metal {

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(
    Device* device,
    const char* label,
    const SharedFenceMTLSharedEventDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->sharedEvent == nullptr, "MTLSharedEvent is missing.");
    return AcquireRef(
        new SharedFence(device, label, static_cast<id<MTLSharedEvent>>(descriptor->sharedEvent)));
}

SharedFence::SharedFence(Device* device, const char* label, id<MTLSharedEvent> sharedEvent)
    : SharedFenceBase(device, label), mSharedEvent(sharedEvent) {}

id<MTLSharedEvent> SharedFence::GetMTLSharedEvent() const {
    return mSharedEvent.Get();
}

MaybeError SharedFence::ExportInfoImpl(SharedFenceExportInfo* info) const {
    info->type = wgpu::SharedFenceType::MTLSharedEvent;

    DAWN_TRY(
        ValidateSingleSType(info->nextInChain, wgpu::SType::SharedFenceMTLSharedEventExportInfo));

    SharedFenceMTLSharedEventExportInfo* exportInfo = nullptr;
    FindInChain(info->nextInChain, &exportInfo);

    if (exportInfo != nullptr) {
        exportInfo->sharedEvent = mSharedEvent.Get();
    }
    return {};
}

}  // namespace dawn::native::metal
