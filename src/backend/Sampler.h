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

#ifndef BACKEND_SAMPLER_H_
#define BACKEND_SAMPLER_H_

#include "backend/Buffer.h"
#include "backend/Forward.h"
#include "backend/RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {

    class SamplerBase : public RefCounted {
      public:
        SamplerBase(SamplerBuilder* builder);
    };

    class SamplerBuilder : public Builder<SamplerBase> {
      public:
        SamplerBuilder(DeviceBase* device);

        nxt::FilterMode GetMagFilter() const;
        nxt::FilterMode GetMinFilter() const;
        nxt::FilterMode GetMipMapFilter() const;

        nxt::AddressMode GetAddressModeU() const;
        nxt::AddressMode GetAddressModeV() const;
        nxt::AddressMode GetAddressModeW() const;

        // NXT API
        void SetFilterMode(nxt::FilterMode magFilter,
                           nxt::FilterMode minFilter,
                           nxt::FilterMode mipMapFilter);

        void SetAddressMode(nxt::AddressMode addressModeU,
                            nxt::AddressMode addressModeV,
                            nxt::AddressMode addressModeW);

      private:
        friend class SamplerBase;

        SamplerBase* GetResultImpl() override;

        int mPropertiesSet = 0;

        nxt::FilterMode mMagFilter = nxt::FilterMode::Nearest;
        nxt::FilterMode mMinFilter = nxt::FilterMode::Nearest;
        nxt::FilterMode mMipMapFilter = nxt::FilterMode::Nearest;

        nxt::AddressMode mAddressModeU = nxt::AddressMode::ClampToEdge;
        nxt::AddressMode mAddressModeV = nxt::AddressMode::ClampToEdge;
        nxt::AddressMode mAddressModeW = nxt::AddressMode::ClampToEdge;
    };

}  // namespace backend

#endif  // BACKEND_SAMPLER_H_
