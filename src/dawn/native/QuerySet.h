// Copyright 2020 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_QUERYSET_H_
#define SRC_DAWN_NATIVE_QUERYSET_H_

#include <vector>

#include "dawn/native/Error.h"
#include "dawn/native/Forward.h"
#include "dawn/native/ObjectBase.h"

#include "dawn/native/dawn_platform.h"

namespace dawn::native {

MaybeError ValidateQuerySetDescriptor(DeviceBase* device, const QuerySetDescriptor* descriptor);

class QuerySetBase : public ApiObjectBase {
  public:
    static QuerySetBase* MakeError(DeviceBase* device, const QuerySetDescriptor* descriptor);

    ObjectType GetType() const override;

    wgpu::QueryType GetQueryType() const;
    uint32_t GetQueryCount() const;
    const std::vector<wgpu::PipelineStatisticName>& GetPipelineStatistics() const;

    const std::vector<bool>& GetQueryAvailability() const;
    void SetQueryAvailability(uint32_t index, bool available);

    MaybeError ValidateCanUseInSubmitNow() const;

    void APIDestroy();
    wgpu::QueryType APIGetType() const;
    uint32_t APIGetCount() const;

  protected:
    QuerySetBase(DeviceBase* device, const QuerySetDescriptor* descriptor);
    QuerySetBase(DeviceBase* device,
                 const QuerySetDescriptor* descriptor,
                 ObjectBase::ErrorTag tag);

    void DestroyImpl() override;

    ~QuerySetBase() override;

  private:
    wgpu::QueryType mQueryType;
    uint32_t mQueryCount;
    std::vector<wgpu::PipelineStatisticName> mPipelineStatistics;

    enum class QuerySetState { Unavailable, Available, Destroyed };
    QuerySetState mState = QuerySetState::Unavailable;

    // Indicates the available queries on the query set for resolving
    std::vector<bool> mQueryAvailability;
};

}  // namespace dawn::native

#endif  // SRC_DAWN_NATIVE_QUERYSET_H_
