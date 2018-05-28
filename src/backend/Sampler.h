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

#include "backend/Error.h"
#include "backend/RefCounted.h"

#include "nxt/nxtcpp.h"

namespace backend {

    class DeviceBase;

    MaybeError ValidateSamplerDescriptor(DeviceBase* device,
                                         const nxt::SamplerDescriptor* descriptor);

    class SamplerBase : public RefCounted {
      public:
        SamplerBase(DeviceBase* device, const nxt::SamplerDescriptor* descriptor);
    };

}  // namespace backend

#endif  // BACKEND_SAMPLER_H_
