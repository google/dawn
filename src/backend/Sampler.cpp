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

    SamplerBase* SamplerBuilder::GetResultImpl() {
        return mDevice->CreateSampler(this);
    }

}  // namespace backend
