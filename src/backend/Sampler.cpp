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
#include "backend/ValidationUtils_autogen.h"

namespace backend {

    bool ValidateSamplerDescriptor(DeviceBase*, const nxt::SamplerDescriptor* descriptor) {
        return descriptor->nextInChain == nullptr && IsValidFilterMode(descriptor->magFilter) &&
               IsValidFilterMode(descriptor->minFilter) &&
               IsValidFilterMode(descriptor->mipmapFilter) &&
               IsValidAddressMode(descriptor->addressModeU) &&
               IsValidAddressMode(descriptor->addressModeV) &&
               IsValidAddressMode(descriptor->addressModeW);
    }

    // SamplerBase

    SamplerBase::SamplerBase(DeviceBase*, const nxt::SamplerDescriptor*) {
    }

}  // namespace backend
