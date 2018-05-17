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

#include "backend/metal/SamplerMTL.h"

#include "backend/metal/MetalBackend.h"

namespace backend { namespace metal {

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

        MTLSamplerAddressMode AddressMode(nxt::AddressMode mode) {
            switch (mode) {
                case nxt::AddressMode::Repeat:
                    return MTLSamplerAddressModeRepeat;
                case nxt::AddressMode::MirroredRepeat:
                    return MTLSamplerAddressModeMirrorRepeat;
                case nxt::AddressMode::ClampToEdge:
                    return MTLSamplerAddressModeClampToEdge;
            }
        }
    }

    Sampler::Sampler(Device* device, const nxt::SamplerDescriptor* descriptor)
        : SamplerBase(device, descriptor) {
        MTLSamplerDescriptor* mtlDesc = [MTLSamplerDescriptor new];
        [mtlDesc autorelease];
        mtlDesc.minFilter = FilterModeToMinMagFilter(descriptor->minFilter);
        mtlDesc.magFilter = FilterModeToMinMagFilter(descriptor->magFilter);
        mtlDesc.mipFilter = FilterModeToMipFilter(descriptor->mipmapFilter);

        mtlDesc.sAddressMode = AddressMode(descriptor->addressModeU);
        mtlDesc.tAddressMode = AddressMode(descriptor->addressModeV);
        mtlDesc.rAddressMode = AddressMode(descriptor->addressModeW);

        mMtlSamplerState = [device->GetMTLDevice() newSamplerStateWithDescriptor:mtlDesc];
    }

    Sampler::~Sampler() {
        [mMtlSamplerState release];
    }

    id<MTLSamplerState> Sampler::GetMTLSamplerState() {
        return mMtlSamplerState;
    }

}}  // namespace backend::metal
