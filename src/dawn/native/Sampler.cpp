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

#include "dawn/native/Sampler.h"

#include <cmath>

#include "dawn/native/Device.h"
#include "dawn/native/ObjectContentHasher.h"
#include "dawn/native/ValidationUtils_autogen.h"

namespace dawn::native {

MaybeError ValidateSamplerDescriptor(DeviceBase*, const SamplerDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->nextInChain != nullptr, "nextInChain must be nullptr");

    DAWN_INVALID_IF(std::isnan(descriptor->lodMinClamp) || std::isnan(descriptor->lodMaxClamp),
                    "LOD clamp bounds [%f, %f] contain a NaN.", descriptor->lodMinClamp,
                    descriptor->lodMaxClamp);

    DAWN_INVALID_IF(descriptor->lodMinClamp < 0 || descriptor->lodMaxClamp < 0,
                    "LOD clamp bounds [%f, %f] contain contain a negative number.",
                    descriptor->lodMinClamp, descriptor->lodMaxClamp);

    DAWN_INVALID_IF(descriptor->lodMinClamp > descriptor->lodMaxClamp,
                    "LOD min clamp (%f) is larger than the max clamp (%f).",
                    descriptor->lodMinClamp, descriptor->lodMaxClamp);

    if (descriptor->maxAnisotropy > 1) {
        DAWN_INVALID_IF(descriptor->minFilter != wgpu::FilterMode::Linear ||
                            descriptor->magFilter != wgpu::FilterMode::Linear ||
                            descriptor->mipmapFilter != wgpu::FilterMode::Linear,
                        "One of minFilter (%s), magFilter (%s) or mipmapFilter (%s) is not %s "
                        "while using anisotropic filter (maxAnisotropy is %f)",
                        descriptor->magFilter, descriptor->minFilter, descriptor->mipmapFilter,
                        wgpu::FilterMode::Linear, descriptor->maxAnisotropy);
    } else if (descriptor->maxAnisotropy == 0u) {
        return DAWN_VALIDATION_ERROR("Max anisotropy (%f) is less than 1.",
                                     descriptor->maxAnisotropy);
    }

    DAWN_TRY(ValidateFilterMode(descriptor->minFilter));
    DAWN_TRY(ValidateFilterMode(descriptor->magFilter));
    DAWN_TRY(ValidateFilterMode(descriptor->mipmapFilter));
    DAWN_TRY(ValidateAddressMode(descriptor->addressModeU));
    DAWN_TRY(ValidateAddressMode(descriptor->addressModeV));
    DAWN_TRY(ValidateAddressMode(descriptor->addressModeW));

    // CompareFunction::Undefined is tagged as invalid because it can't be used, except for the
    // SamplerDescriptor where it is a special value that means the sampler is not a
    // comparison-sampler.
    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        DAWN_TRY(ValidateCompareFunction(descriptor->compare));
    }

    return {};
}

// SamplerBase

SamplerBase::SamplerBase(DeviceBase* device,
                         const SamplerDescriptor* descriptor,
                         ApiObjectBase::UntrackedByDeviceTag tag)
    : ApiObjectBase(device, descriptor->label),
      mAddressModeU(descriptor->addressModeU),
      mAddressModeV(descriptor->addressModeV),
      mAddressModeW(descriptor->addressModeW),
      mMagFilter(descriptor->magFilter),
      mMinFilter(descriptor->minFilter),
      mMipmapFilter(descriptor->mipmapFilter),
      mLodMinClamp(descriptor->lodMinClamp),
      mLodMaxClamp(descriptor->lodMaxClamp),
      mCompareFunction(descriptor->compare),
      mMaxAnisotropy(descriptor->maxAnisotropy) {}

SamplerBase::SamplerBase(DeviceBase* device, const SamplerDescriptor* descriptor)
    : SamplerBase(device, descriptor, kUntrackedByDevice) {
    GetObjectTrackingList()->Track(this);
}

SamplerBase::SamplerBase(DeviceBase* device, ObjectBase::ErrorTag tag)
    : ApiObjectBase(device, tag) {}

SamplerBase::~SamplerBase() = default;

void SamplerBase::DestroyImpl() {
    if (IsCachedReference()) {
        // Do not uncache the actual cached object if we are a blueprint.
        GetDevice()->UncacheSampler(this);
    }
}

// static
SamplerBase* SamplerBase::MakeError(DeviceBase* device) {
    return new SamplerBase(device, ObjectBase::kError);
}

ObjectType SamplerBase::GetType() const {
    return ObjectType::Sampler;
}

bool SamplerBase::IsComparison() const {
    return mCompareFunction != wgpu::CompareFunction::Undefined;
}

bool SamplerBase::IsFiltering() const {
    return mMinFilter == wgpu::FilterMode::Linear || mMagFilter == wgpu::FilterMode::Linear ||
           mMipmapFilter == wgpu::FilterMode::Linear;
}

size_t SamplerBase::ComputeContentHash() {
    ObjectContentHasher recorder;
    recorder.Record(mAddressModeU, mAddressModeV, mAddressModeW, mMagFilter, mMinFilter,
                    mMipmapFilter, mLodMinClamp, mLodMaxClamp, mCompareFunction, mMaxAnisotropy);
    return recorder.GetContentHash();
}

bool SamplerBase::EqualityFunc::operator()(const SamplerBase* a, const SamplerBase* b) const {
    if (a == b) {
        return true;
    }

    ASSERT(!std::isnan(a->mLodMinClamp));
    ASSERT(!std::isnan(b->mLodMinClamp));
    ASSERT(!std::isnan(a->mLodMaxClamp));
    ASSERT(!std::isnan(b->mLodMaxClamp));

    return a->mAddressModeU == b->mAddressModeU && a->mAddressModeV == b->mAddressModeV &&
           a->mAddressModeW == b->mAddressModeW && a->mMagFilter == b->mMagFilter &&
           a->mMinFilter == b->mMinFilter && a->mMipmapFilter == b->mMipmapFilter &&
           a->mLodMinClamp == b->mLodMinClamp && a->mLodMaxClamp == b->mLodMaxClamp &&
           a->mCompareFunction == b->mCompareFunction && a->mMaxAnisotropy == b->mMaxAnisotropy;
}

}  // namespace dawn::native
