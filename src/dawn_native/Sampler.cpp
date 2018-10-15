// Copyright 2017 The Dawn Authors
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

#include "dawn_native/Sampler.h"

#include "dawn_native/Device.h"
#include "dawn_native/ValidationUtils_autogen.h"

namespace dawn_native {

    MaybeError ValidateSamplerDescriptor(DeviceBase*, const SamplerDescriptor* descriptor) {
        if (descriptor->nextInChain != nullptr) {
            return DAWN_VALIDATION_ERROR("nextInChain must be nullptr");
        }
        DAWN_TRY(ValidateFilterMode(descriptor->minFilter));
        DAWN_TRY(ValidateFilterMode(descriptor->magFilter));
        DAWN_TRY(ValidateFilterMode(descriptor->mipmapFilter));
        DAWN_TRY(ValidateAddressMode(descriptor->addressModeU));
        DAWN_TRY(ValidateAddressMode(descriptor->addressModeV));
        DAWN_TRY(ValidateAddressMode(descriptor->addressModeW));
        return {};
    }

    // SamplerBase

    SamplerBase::SamplerBase(DeviceBase* device, const SamplerDescriptor*) : ObjectBase(device) {
    }

}  // namespace dawn_native
