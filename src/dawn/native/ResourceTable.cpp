// Copyright 2025 The Dawn & Tint Authors
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

#include "dawn/native/ResourceTable.h"

#include <utility>

#include "dawn/native/Device.h"

namespace dawn::native {

MaybeError ValidateResourceTableDescriptor(const DeviceBase* device,
                                           const ResourceTableDescriptor* descriptor) {
    DAWN_ASSERT(descriptor);

    DAWN_INVALID_IF(!device->HasFeature(Feature::ChromiumExperimentalSamplingResourceTable),
                    "Resource table used without the %s feature enabled.",
                    wgpu::FeatureName::ChromiumExperimentalSamplingResourceTable);

    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain is not nullptr.");

    DAWN_INVALID_IF(descriptor->size > device->GetLimits().resourceTableLimits.maxResourceTableSize,
                    "Resource table size (%u) is larger than maxResourceTableSize (%u)",
                    descriptor->size, device->GetLimits().resourceTableLimits.maxResourceTableSize);

    return {};
}

ResourceTableBase::ResourceTableBase(DeviceBase* device, const ResourceTableDescriptor* descriptor)
    : ApiObjectBase(device, descriptor->label),
      mDynamicArray(AcquireRef(new DynamicArrayState(device,
                                                     BindingIndex(descriptor->size),
                                                     wgpu::DynamicBindingKind::SampledTexture))) {
    GetObjectTrackingList()->Track(this);
}

ResourceTableBase::ResourceTableBase(DeviceBase* device, ObjectBase::ErrorTag tag, StringView label)
    : ApiObjectBase(device, tag, label), mDestroyed(true) {}

// static
Ref<ResourceTableBase> ResourceTableBase::MakeError(DeviceBase* device, StringView label) {
    return AcquireRef(new ResourceTableBase(device, ObjectBase::kError, label));
}

ObjectType ResourceTableBase::GetType() const {
    return ObjectType::ResourceTable;
}

MaybeError ResourceTableBase::InitializeBase() {
    return mDynamicArray->Initialize();
}

DynamicArrayState* ResourceTableBase::GetDynamicArrayState() {
    return mDynamicArray.Get();
}

void ResourceTableBase::DestroyImpl() {
    if (mDynamicArray) {
        mDynamicArray->Destroy();
    }
    mDestroyed = true;
}

void ResourceTableBase::APIDestroy() {
    Destroy();  // Calls DestroyImpl
}

MaybeError ResourceTableBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_INVALID_IF(mDestroyed, "%s used while destroyed.", this);
    return {};
}

}  // namespace dawn::native
