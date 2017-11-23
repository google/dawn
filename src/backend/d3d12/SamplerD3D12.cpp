// Copyright 2017 The NXT Authors
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

#include "backend/d3d12/SamplerD3D12.h"

#include "backend/d3d12/D3D12Backend.h"

namespace backend {
namespace d3d12 {

    Sampler::Sampler(SamplerBuilder* builder)
        : SamplerBase(builder) {

        // https://msdn.microsoft.com/en-us/library/windows/desktop/dn770367(v=vs.85).aspx
        // D3D12_FILTER_MIN_MAG_MIP_POINT                       = 0       0     0 0 0   // hex value, decimal value, min linear, mag linear, mip linear
        // D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR                = 0x1     1     0 0 1
        // D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT          = 0x4     4     0 1 0
        // D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR                = 0x5     5     0 1 1
        // D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT                = 0x10   16     1 0 0
        // D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR         = 0x11   17     1 0 1
        // D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT                = 0x14   20     1 1 0
        // D3D12_FILTER_MIN_MAG_MIP_LINEAR                      = 0x15   21     1 1 1

        // if mip mode is linear, add 1
        // if mag mode is linear, add 4
        // if min mode is linear, add 16

        uint8_t mode = 0;

        switch (builder->GetMinFilter()) {
            case nxt::FilterMode::Nearest:
                break;
            case nxt::FilterMode::Linear:
                mode += 16;
                break;
        }

        switch (builder->GetMagFilter()) {
            case nxt::FilterMode::Nearest:
                break;
            case nxt::FilterMode::Linear:
                mode += 4;
                break;
        }

        switch (builder->GetMipMapFilter()) {
            case nxt::FilterMode::Nearest:
                break;
            case nxt::FilterMode::Linear:
                mode += 1;
                break;
        }

        mSamplerDesc.Filter = static_cast<D3D12_FILTER>(mode);
        mSamplerDesc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        mSamplerDesc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        mSamplerDesc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        mSamplerDesc.MipLODBias = 0.f;
        mSamplerDesc.MaxAnisotropy = 1;
        mSamplerDesc.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        mSamplerDesc.BorderColor[0] = mSamplerDesc.BorderColor[1] = mSamplerDesc.BorderColor[2] = mSamplerDesc.BorderColor[3] = 0;
        mSamplerDesc.MinLOD = 0;
        mSamplerDesc.MaxLOD = D3D12_FLOAT32_MAX;

    }

    const D3D12_SAMPLER_DESC& Sampler::GetSamplerDescriptor() const {
        return mSamplerDesc;
    }

}
}
