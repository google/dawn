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

#include "dawn/native/metal/SamplerMTL.h"

#include "dawn/native/metal/DeviceMTL.h"
#include "dawn/native/metal/UtilsMetal.h"

namespace dawn::native::metal {

namespace {
MTLSamplerMinMagFilter FilterModeToMinMagFilter(wgpu::FilterMode mode) {
    switch (mode) {
        case wgpu::FilterMode::Nearest:
            return MTLSamplerMinMagFilterNearest;
        case wgpu::FilterMode::Linear:
            return MTLSamplerMinMagFilterLinear;
    }
}

MTLSamplerMipFilter FilterModeToMipFilter(wgpu::FilterMode mode) {
    switch (mode) {
        case wgpu::FilterMode::Nearest:
            return MTLSamplerMipFilterNearest;
        case wgpu::FilterMode::Linear:
            return MTLSamplerMipFilterLinear;
    }
}

MTLSamplerAddressMode AddressMode(wgpu::AddressMode mode) {
    switch (mode) {
        case wgpu::AddressMode::Repeat:
            return MTLSamplerAddressModeRepeat;
        case wgpu::AddressMode::MirrorRepeat:
            return MTLSamplerAddressModeMirrorRepeat;
        case wgpu::AddressMode::ClampToEdge:
            return MTLSamplerAddressModeClampToEdge;
    }
}
}  // namespace

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    DAWN_INVALID_IF(
        descriptor->compare != wgpu::CompareFunction::Undefined &&
            device->IsToggleEnabled(Toggle::MetalDisableSamplerCompare),
        "Sampler compare function (%s) not supported. Compare functions are disabled with the "
        "Metal backend.",
        descriptor->compare);

    Ref<Sampler> sampler = AcquireRef(new Sampler(device, descriptor));
    DAWN_TRY(sampler->Initialize(descriptor));
    return sampler;
}

Sampler::Sampler(DeviceBase* dev, const SamplerDescriptor* desc) : SamplerBase(dev, desc) {}

Sampler::~Sampler() = default;

MaybeError Sampler::Initialize(const SamplerDescriptor* descriptor) {
    NSRef<MTLSamplerDescriptor> mtlDescRef = AcquireNSRef([MTLSamplerDescriptor new]);
    MTLSamplerDescriptor* mtlDesc = mtlDescRef.Get();

    mtlDesc.minFilter = FilterModeToMinMagFilter(descriptor->minFilter);
    mtlDesc.magFilter = FilterModeToMinMagFilter(descriptor->magFilter);
    mtlDesc.mipFilter = FilterModeToMipFilter(descriptor->mipmapFilter);

    mtlDesc.sAddressMode = AddressMode(descriptor->addressModeU);
    mtlDesc.tAddressMode = AddressMode(descriptor->addressModeV);
    mtlDesc.rAddressMode = AddressMode(descriptor->addressModeW);

    mtlDesc.lodMinClamp = descriptor->lodMinClamp;
    mtlDesc.lodMaxClamp = descriptor->lodMaxClamp;
    // https://developer.apple.com/documentation/metal/mtlsamplerdescriptor/1516164-maxanisotropy
    mtlDesc.maxAnisotropy = std::min<uint16_t>(GetMaxAnisotropy(), 16u);

    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        // Sampler compare is unsupported before A9, which we validate in
        // Sampler::Create.
        mtlDesc.compareFunction = ToMetalCompareFunction(descriptor->compare);
        // The value is default-initialized in the else-case, and we don't set it or the
        // Metal debug device errors.
    }

    mMtlSamplerState = AcquireNSPRef(
        [ToBackend(GetDevice())->GetMTLDevice() newSamplerStateWithDescriptor:mtlDesc]);

    if (mMtlSamplerState == nil) {
        return DAWN_OUT_OF_MEMORY_ERROR("Failed to allocate sampler.");
    }
    return {};
}

id<MTLSamplerState> Sampler::GetMTLSamplerState() {
    return mMtlSamplerState.Get();
}

}  // namespace dawn::native::metal
