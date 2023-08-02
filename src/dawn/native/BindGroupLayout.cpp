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

#include "dawn/native/BindGroupLayout.h"

#include "dawn/native/ObjectType_autogen.h"

namespace dawn::native {

BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                         const char* label,
                                         Ref<BindGroupLayoutInternalBase> internal,
                                         PipelineCompatibilityToken pipelineCompatibilityToken)
    : ApiObjectBase(device, label),
      mInternalLayout(internal),
      mPipelineCompatibilityToken(pipelineCompatibilityToken) {
    GetObjectTrackingList()->Track(this);
}

BindGroupLayoutBase::BindGroupLayoutBase(DeviceBase* device,
                                         ObjectBase::ErrorTag tag,
                                         const char* label)
    : ApiObjectBase(device, tag, label) {}

ObjectType BindGroupLayoutBase::GetType() const {
    return ObjectType::BindGroupLayout;
}

// static
BindGroupLayoutBase* BindGroupLayoutBase::MakeError(DeviceBase* device, const char* label) {
    return new BindGroupLayoutBase(device, ObjectBase::kError, label);
}

BindGroupLayoutInternalBase* BindGroupLayoutBase::GetInternalBindGroupLayout() const {
    return mInternalLayout.Get();
}

bool BindGroupLayoutBase::IsLayoutEqual(const BindGroupLayoutBase* other,
                                        bool excludePipelineCompatibiltyToken) const {
    if (!excludePipelineCompatibiltyToken &&
        GetPipelineCompatibilityToken() != other->GetPipelineCompatibilityToken()) {
        return false;
    }
    return GetInternalBindGroupLayout() == other->GetInternalBindGroupLayout();
}

void BindGroupLayoutBase::DestroyImpl() {}

}  // namespace dawn::native
