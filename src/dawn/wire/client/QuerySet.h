// Copyright 2022 The Dawn Authors
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

#ifndef SRC_DAWN_WIRE_CLIENT_QUERYSET_H_
#define SRC_DAWN_WIRE_CLIENT_QUERYSET_H_

#include "dawn/webgpu.h"

#include "dawn/wire/client/ObjectBase.h"

namespace dawn::wire::client {

class Device;

class QuerySet final : public ObjectBase {
  public:
    static WGPUQuerySet Create(Device* device, const WGPUQuerySetDescriptor* descriptor);

    QuerySet(const ObjectBaseParams& params, const WGPUQuerySetDescriptor* descriptor);
    ~QuerySet() override;

    // Note that these values can be arbitrary since they aren't validated in the wire client.
    WGPUQueryType GetType() const;
    uint32_t GetCount() const;

  private:
    WGPUQueryType mType;
    uint32_t mCount;
};

}  // namespace dawn::wire::client

#endif  // SRC_DAWN_WIRE_CLIENT_QUERYSET_H_
