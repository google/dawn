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

#include "Sampler.h"

#include "Device.h"

namespace backend {

    // SamplerBase

    SamplerBase::SamplerBase(SamplerBuilder* builder) {
    }

    // SamplerBuilder

    enum SamplerSetProperties {
        SAMPLER_PROPERTY_FILTER = 0x1,
    };
    SamplerBuilder::SamplerBuilder(DeviceBase* device) : Builder(device) {
    }

    nxt::FilterMode SamplerBuilder::GetMagFilter() const {
        return magFilter;
    }

    nxt::FilterMode SamplerBuilder::GetMinFilter() const {
        return minFilter;
    }

    nxt::FilterMode SamplerBuilder::GetMipMapFilter() const {
        return mipMapFilter;
    }

    void SamplerBuilder::SetFilterMode(nxt::FilterMode magFilter, nxt::FilterMode minFilter, nxt::FilterMode mipMapFilter) {
        if ((propertiesSet & SAMPLER_PROPERTY_FILTER) != 0) {
            HandleError("Sampler filter property set multiple times");
            return;
        }

        this->magFilter = magFilter;
        this->minFilter = minFilter;
        this->mipMapFilter = mipMapFilter;
        propertiesSet |= SAMPLER_PROPERTY_FILTER;
    }

    SamplerBase* SamplerBuilder::GetResultImpl() {
        return device->CreateSampler(this);
    }

}
