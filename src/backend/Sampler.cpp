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

#include "backend/Sampler.h"

#include "backend/Device.h"

namespace backend {

    // SamplerBase

    SamplerBase::SamplerBase(SamplerBuilder*) {
    }

    // SamplerBuilder

    enum SamplerSetProperties {
        SAMPLER_PROPERTY_FILTER = 0x1,
        SAMPLER_PROPERTY_ADDRESS = 0x2,
    };
    SamplerBuilder::SamplerBuilder(DeviceBase* device) : Builder(device) {
    }

    nxt::FilterMode SamplerBuilder::GetMagFilter() const {
        return mMagFilter;
    }

    nxt::FilterMode SamplerBuilder::GetMinFilter() const {
        return mMinFilter;
    }

    nxt::FilterMode SamplerBuilder::GetMipMapFilter() const {
        return mMipMapFilter;
    }

    nxt::AddressMode SamplerBuilder::GetAddressModeU() const {
        return mAddressModeU;
    }

    nxt::AddressMode SamplerBuilder::GetAddressModeV() const {
        return mAddressModeV;
    }

    nxt::AddressMode SamplerBuilder::GetAddressModeW() const {
        return mAddressModeW;
    }

    void SamplerBuilder::SetFilterMode(nxt::FilterMode magFilter,
                                       nxt::FilterMode minFilter,
                                       nxt::FilterMode mipMapFilter) {
        if ((mPropertiesSet & SAMPLER_PROPERTY_FILTER) != 0) {
            HandleError("Sampler filter property set multiple times");
            return;
        }

        mMagFilter = magFilter;
        mMinFilter = minFilter;
        mMipMapFilter = mipMapFilter;
        mPropertiesSet |= SAMPLER_PROPERTY_FILTER;
    }

    void SamplerBuilder::SetAddressMode(nxt::AddressMode addressModeU,
                                        nxt::AddressMode addressModeV,
                                        nxt::AddressMode addressModeW) {
        if ((mPropertiesSet & SAMPLER_PROPERTY_ADDRESS) != 0) {
            HandleError("Sampler address property set multiple times");
            return;
        }

        mAddressModeU = addressModeU;
        mAddressModeV = addressModeV;
        mAddressModeW = addressModeW;
        mPropertiesSet |= SAMPLER_PROPERTY_ADDRESS;
    }

    SamplerBase* SamplerBuilder::GetResultImpl() {
        return mDevice->CreateSampler(this);
    }

}  // namespace backend
