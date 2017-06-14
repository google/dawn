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

#include "SamplerMTL.h"

#include "MetalBackend.h"

namespace backend {
namespace metal {

    namespace {
        MTLSamplerMinMagFilter FilterModeToMinMagFilter(nxt::FilterMode mode) {
            switch (mode) {
                case nxt::FilterMode::Nearest:
                    return MTLSamplerMinMagFilterNearest;
                case nxt::FilterMode::Linear:
                    return MTLSamplerMinMagFilterLinear;
            }
        }

        MTLSamplerMipFilter FilterModeToMipFilter(nxt::FilterMode mode) {
            switch (mode) {
                case nxt::FilterMode::Nearest:
                    return MTLSamplerMipFilterNearest;
                case nxt::FilterMode::Linear:
                    return MTLSamplerMipFilterLinear;
            }
        }
    }

    Sampler::Sampler(SamplerBuilder* builder)
        : SamplerBase(builder) {
        auto desc = [MTLSamplerDescriptor new];
        [desc autorelease];
        desc.minFilter = FilterModeToMinMagFilter(builder->GetMinFilter());
        desc.magFilter = FilterModeToMinMagFilter(builder->GetMagFilter());
        desc.mipFilter = FilterModeToMipFilter(builder->GetMipMapFilter());

        // TODO(kainino@chromium.org): wrap modes
        auto mtlDevice = ToBackend(builder->GetDevice())->GetMTLDevice();
        mtlSamplerState = [mtlDevice newSamplerStateWithDescriptor:desc];
    }

    Sampler::~Sampler() {
        [mtlSamplerState release];
    }

    id<MTLSamplerState> Sampler::GetMTLSamplerState() {
        return mtlSamplerState;
    }

}
}
