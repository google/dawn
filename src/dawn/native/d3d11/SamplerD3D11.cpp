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

#include "dawn/native/d3d11/SamplerD3D11.h"

#include <algorithm>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {

namespace {

D3D11_TEXTURE_ADDRESS_MODE D3D11TextureAddressMode(wgpu::AddressMode mode) {
    switch (mode) {
        case wgpu::AddressMode::Repeat:
            return D3D11_TEXTURE_ADDRESS_WRAP;
        case wgpu::AddressMode::MirrorRepeat:
            return D3D11_TEXTURE_ADDRESS_MIRROR;
        case wgpu::AddressMode::ClampToEdge:
            return D3D11_TEXTURE_ADDRESS_CLAMP;
    }
}

D3D11_FILTER_TYPE D3D11FilterType(wgpu::FilterMode mode) {
    switch (mode) {
        case wgpu::FilterMode::Nearest:
            return D3D11_FILTER_TYPE_POINT;
        case wgpu::FilterMode::Linear:
            return D3D11_FILTER_TYPE_LINEAR;
    }
}

}  // namespace

// static
ResultOrError<Ref<Sampler>> Sampler::Create(Device* device, const SamplerDescriptor* descriptor) {
    auto sampler = AcquireRef(new Sampler(device, descriptor));
    DAWN_TRY(sampler->Initialize(descriptor));
    return sampler;
}

MaybeError Sampler::Initialize(const SamplerDescriptor* descriptor) {
    D3D11_SAMPLER_DESC samplerDesc = {};
    D3D11_FILTER_TYPE minFilter = D3D11FilterType(descriptor->minFilter);
    D3D11_FILTER_TYPE magFilter = D3D11FilterType(descriptor->magFilter);
    D3D11_FILTER_TYPE mipmapFilter = D3D11FilterType(descriptor->mipmapFilter);

    D3D11_FILTER_REDUCTION_TYPE reduction = descriptor->compare == wgpu::CompareFunction::Undefined
                                                ? D3D11_FILTER_REDUCTION_TYPE_STANDARD
                                                : D3D11_FILTER_REDUCTION_TYPE_COMPARISON;

    // https://docs.microsoft.com/en-us/windows/win32/api/d3d11/ns-d3d11-d3d11_sampler_desc
    samplerDesc.MaxAnisotropy = std::min<uint16_t>(GetMaxAnisotropy(), 16u);

    if (samplerDesc.MaxAnisotropy > 1) {
        samplerDesc.Filter = D3D11_ENCODE_ANISOTROPIC_FILTER(reduction);
    } else {
        samplerDesc.Filter =
            D3D11_ENCODE_BASIC_FILTER(minFilter, magFilter, mipmapFilter, reduction);
    }

    samplerDesc.AddressU = D3D11TextureAddressMode(descriptor->addressModeU);
    samplerDesc.AddressV = D3D11TextureAddressMode(descriptor->addressModeV);
    samplerDesc.AddressW = D3D11TextureAddressMode(descriptor->addressModeW);
    samplerDesc.MipLODBias = 0.f;

    if (descriptor->compare != wgpu::CompareFunction::Undefined) {
        samplerDesc.ComparisonFunc = ToD3D11ComparisonFunc(descriptor->compare);
    } else {
        // Still set the function so it's not garbage.
        samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    }
    samplerDesc.MinLOD = descriptor->lodMinClamp;
    samplerDesc.MaxLOD = descriptor->lodMaxClamp;

    DAWN_TRY(CheckHRESULT(ToBackend(GetDevice())
                              ->GetD3D11Device()
                              ->CreateSamplerState(&samplerDesc, &mD3d11SamplerState),
                          "ID3D11Device::CreateSamplerState"));

    SetLabelImpl();
    return {};
}

ID3D11SamplerState* Sampler::GetD3D11SamplerState() const {
    return mD3d11SamplerState.Get();
}

void Sampler::SetLabelImpl() {
    SetDebugName(ToBackend(GetDevice()), mD3d11SamplerState.Get(), "Dawn_Sampler", GetLabel());
}

}  // namespace dawn::native::d3d11
