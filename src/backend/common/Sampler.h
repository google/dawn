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

#ifndef BACKEND_COMMON_SAMPLER_H_
#define BACKEND_COMMON_SAMPLER_H_

#include "Forward.h"
#include "Buffer.h"
#include "RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {

    class SamplerBase : public RefCounted {
        public:
            SamplerBase(SamplerBuilder* builder);
    };

    class SamplerBuilder : public Builder {
        public:
            SamplerBuilder(DeviceBase* device);

            nxt::FilterMode GetMagFilter() const;
            nxt::FilterMode GetMinFilter() const;
            nxt::FilterMode GetMipMapFilter() const;

            // NXT API
            SamplerBase* GetResult();
            void SetFilterMode(nxt::FilterMode magFilter, nxt::FilterMode minFilter, nxt::FilterMode mipMapFilter);

        private:
            friend class SamplerBase;

            int propertiesSet = 0;

            nxt::FilterMode magFilter = nxt::FilterMode::Nearest;
            nxt::FilterMode minFilter = nxt::FilterMode::Nearest;
            nxt::FilterMode mipMapFilter = nxt::FilterMode::Nearest;
    };

}

#endif // BACKEND_COMMON_SAMPLER_H_
