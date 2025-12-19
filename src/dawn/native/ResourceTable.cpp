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

namespace {

MaybeError ValidateBindingResource(const DeviceBase* device, const BindingResource* resource) {
    DAWN_INVALID_IF(resource->nextInChain != nullptr, "nextInChain is not null.");

    uint32_t resourceCount = uint32_t(resource->buffer != nullptr) +
                             uint32_t(resource->textureView != nullptr) +
                             uint32_t(resource->sampler != nullptr);
    DAWN_INVALID_IF(resourceCount != 1,
                    "%i resources are specified (when there must be exactly 1).", resourceCount);

    // TODO(https://issues.chromium.org/435317394): Support buffers in FullResourceTable.
    if (resource->buffer != nullptr) {
        return DAWN_VALIDATION_ERROR("Buffers are not supported.");
    }

    // TODO(https://issues.chromium.org/435317394): Support samplers in SamplingResourceTable.
    if (resource->sampler != nullptr) {
        return DAWN_VALIDATION_ERROR("Samplers are not supported.");
    }

    // TODO(https://issues.chromium.org/435317394): Support texel buffers in FullResourceTable.

    if (resource->textureView != nullptr) {
        TextureViewBase* view = resource->textureView;
        DAWN_TRY(device->ValidateObject(view));

        Aspect aspect = view->GetAspects();
        DAWN_INVALID_IF(!HasOneBit(aspect),
                        "Multiple aspects (%s) selected in %s. Expected only 1.", aspect, view);

        // TODO(https://issues.chromium.org/435317394): Support storage textures in
        // FullResourceTable
        DAWN_INVALID_IF(
            (view->GetUsage() & kTextureViewOnlyUsages) != wgpu::TextureUsage::TextureBinding,
            "%s's usages (%s) are not exactly %s.", view, view->GetUsage() & kTextureViewOnlyUsages,
            wgpu::TextureUsage::TextureBinding);

        DAWN_INVALID_IF(view->IsYCbCr(), "%s is YCbCr.", view);
    }

    return {};
}

}  // anonymous namespace

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
      mDynamicArray(AcquireRef(new DynamicArrayState(device, BindingIndex(descriptor->size)))) {
    GetObjectTrackingList()->Track(this);
}

ResourceTableBase::ResourceTableBase(DeviceBase* device,
                                     const ResourceTableDescriptor* descriptor,
                                     ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag, descriptor->label) {
    // Create a DynamicArrayState even for an error resource table because we need to do state
    // tracking used for the validation of synchronous errors. However skip creating it for tables
    // above the limit because that's caught on the content-timeline as well.
    if (descriptor->size <= device->GetLimits().resourceTableLimits.maxResourceTableSize) {
        mDynamicArray = AcquireRef(new DynamicArrayState(device, BindingIndex(descriptor->size)));
    }
}

// static
Ref<ResourceTableBase> ResourceTableBase::MakeError(DeviceBase* device,
                                                    const ResourceTableDescriptor* descriptor) {
    return AcquireRef(new ResourceTableBase(device, descriptor, ObjectBase::kError));
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
    if (!mDynamicArray->IsDestroyed()) {
        mDynamicArray->Destroy();
    }
}

void ResourceTableBase::APIDestroy() {
    // Don't just call Destroy() because it skips running on error objects.
    if (!mDynamicArray->IsDestroyed()) {
        mDynamicArray->Destroy();
    }
}

wgpu::Status ResourceTableBase::APIUpdate(uint32_t slotIn, const BindingResource* resource) {
    ResourceTableSlot slot = ResourceTableSlot(slotIn);
    if (!IsValidSlot(slot)) {
        return wgpu::Status::Error;
    }

    // Prevent replacing a slot that may be in use by the GPU.
    if (!mDynamicArray->CanBeUpdated(slot)) {
        return wgpu::Status::Error;
    }

    // Perform validation that produces a validation error, but unconditionally mark the slot as
    // used since we need to match client-side validation that doesn't perform these checks.
    if (GetDevice()->ConsumedError(  //
            ([&]() -> MaybeError {
                DAWN_TRY(GetDevice()->ValidateObject(this));
                return ValidateBindingResource(GetDevice(), resource);
            })(),
            "validating %s.Update()", this)) {
        BindingResource nothing = {};
        mDynamicArray->Update(slot, nothing);
    } else {
        mDynamicArray->Update(slot, *resource);
    }

    return wgpu::Status::Success;
}

uint32_t ResourceTableBase::APIInsertBinding(const BindingResource* resource) {
    if (mDynamicArray == nullptr || mDynamicArray->IsDestroyed()) {
        return wgpu::kInvalidBinding;
    }

    std::optional<ResourceTableSlot> freeSlot = mDynamicArray->GetFreeSlot();
    if (!freeSlot) {
        return wgpu::kInvalidBinding;
    }
    ResourceTableSlot slot = freeSlot.value();

    wgpu::Status updateStatus = APIUpdate(uint32_t(slot), resource);
    DAWN_ASSERT(updateStatus == wgpu::Status::Success);
    return uint32_t(slot);
}

wgpu::Status ResourceTableBase::APIRemoveBinding(uint32_t slotIn) {
    ResourceTableSlot slot = ResourceTableSlot(slotIn);
    if (!IsValidSlot(slot)) {
        return wgpu::Status::Error;
    }

    // Always remove the slot, even if a validation error happens, so that we match client-side
    // validation.
    mDynamicArray->Remove(slot);

    [[maybe_unused]] bool error = GetDevice()->ConsumedError(
        GetDevice()->ValidateObject(this), "validating %s.RemoveBinding(%u)", this, slot);
    return wgpu::Status::Success;
}

uint32_t ResourceTableBase::APIGetSize() const {
    if (mDynamicArray == nullptr) {
        return 0;
    }

    return uint32_t(mDynamicArray->GetAPISize());
}

MaybeError ResourceTableBase::ValidateCanUseInSubmitNow() const {
    DAWN_ASSERT(!IsError());
    DAWN_ASSERT(mDynamicArray != nullptr);
    DAWN_INVALID_IF(mDynamicArray->IsDestroyed(), "%s used while destroyed.", this);
    return {};
}

bool ResourceTableBase::IsValidSlot(ResourceTableSlot slot) const {
    // Some validation is required to return a synchronous error. It needs to be able to run even on
    // error ResourceTables because it must act the same way as an implementation running on top of
    // the wire client-side and doesn't know if objects are errors or not.
    if (mDynamicArray == nullptr || mDynamicArray->IsDestroyed()) {
        return false;
    }

    if (slot >= mDynamicArray->GetAPISize()) {
        return false;
    }

    return true;
}

}  // namespace dawn::native
