// Copyright 2017 The Dawn & Tint Authors
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
                            descriptor->mipmapFilter != wgpu::MipmapFilterMode::Linear,
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
    DAWN_TRY(ValidateMipmapFilterMode(descriptor->mipmapFilter));
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

SamplerBase::SamplerBase(DeviceBase* device, ObjectBase::ErrorTag tag, const char* label)
    : ApiObjectBase(device, tag, label) {}

SamplerBase::~SamplerBase() = default;

void SamplerBase::DestroyImpl() {
    Uncache();
}

// static
SamplerBase* SamplerBase::MakeError(DeviceBase* device, const char* label) {
    return new SamplerBase(device, ObjectBase::kError, label);
}

ObjectType SamplerBase::GetType() const {
    return ObjectType::Sampler;
}

bool SamplerBase::IsComparison() const {
    return mCompareFunction != wgpu::CompareFunction::Undefined;
}

bool SamplerBase::IsFiltering() const {
    return mMinFilter == wgpu::FilterMode::Linear || mMagFilter == wgpu::FilterMode::Linear ||
           mMipmapFilter == wgpu::MipmapFilterMode::Linear;
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

    DAWN_ASSERT(!std::isnan(a->mLodMinClamp));
    DAWN_ASSERT(!std::isnan(b->mLodMinClamp));
    DAWN_ASSERT(!std::isnan(a->mLodMaxClamp));
    DAWN_ASSERT(!std::isnan(b->mLodMaxClamp));

    return a->mAddressModeU == b->mAddressModeU && a->mAddressModeV == b->mAddressModeV &&
           a->mAddressModeW == b->mAddressModeW && a->mMagFilter == b->mMagFilter &&
           a->mMinFilter == b->mMinFilter && a->mMipmapFilter == b->mMipmapFilter &&
           a->mLodMinClamp == b->mLodMinClamp && a->mLodMaxClamp == b->mLodMaxClamp &&
           a->mCompareFunction == b->mCompareFunction && a->mMaxAnisotropy == b->mMaxAnisotropy;
}

}  // namespace dawn::native
