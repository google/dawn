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

#include "dawn/native/SharedFence.h"

#include "dawn/native/Device.h"
#include "dawn/native/dawn_platform.h"

namespace dawn::native {

namespace {

class ErrorSharedFence : public SharedFenceBase {
  public:
    ErrorSharedFence(DeviceBase* device, const SharedFenceDescriptor* descriptor)
        : SharedFenceBase(device, descriptor, ObjectBase::kError) {}

    MaybeError ExportInfoImpl(SharedFenceExportInfo* info) const override { UNREACHABLE(); }
};

}  // namespace

// static
SharedFenceBase* SharedFenceBase::MakeError(DeviceBase* device,
                                            const SharedFenceDescriptor* descriptor) {
    return new ErrorSharedFence(device, descriptor);
}

SharedFenceBase::SharedFenceBase(DeviceBase* device,
                                 const SharedFenceDescriptor* descriptor,
                                 ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label) {}

SharedFenceBase::SharedFenceBase(DeviceBase* device, const char* label)
    : ApiObjectBase(device, label) {}

ObjectType SharedFenceBase::GetType() const {
    return ObjectType::SharedFence;
}

void SharedFenceBase::APIExportInfo(SharedFenceExportInfo* info) const {
    DAWN_UNUSED(GetDevice()->ConsumedError(ExportInfo(info)));
}

void SharedFenceBase::DestroyImpl() {}

MaybeError SharedFenceBase::ExportInfo(SharedFenceExportInfo* info) const {
    // Set the type to undefined. It will be overwritten to the actual type
    // as long as no error occurs.
    info->type = wgpu::SharedFenceType::Undefined;

    DAWN_TRY(GetDevice()->ValidateObject(this));
    return ExportInfoImpl(info);
}

}  // namespace dawn::native
