// Copyright 2023 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D11_QUERYSETD3D11_H_
#define SRC_DAWN_NATIVE_D3D11_QUERYSETD3D11_H_

#include <vector>

#include "dawn/native/QuerySet.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d11 {

class Device;
class Buffer;
class CommandRecordingContext;

class QuerySet final : public QuerySetBase {
  public:
    static ResultOrError<Ref<QuerySet>> Create(Device* device,
                                               const QuerySetDescriptor* descriptor);

    void BeginQuery(ID3D11DeviceContext* d3d11DeviceContext, uint32_t query);
    void EndQuery(ID3D11DeviceContext* d3d11DeviceContext, uint32_t query);
    MaybeError Resolve(CommandRecordingContext* commandContext,
                       uint32_t firstQuery,
                       uint32_t queryCount,
                       Buffer* destination,
                       uint64_t offset);

  private:
    using QuerySetBase::QuerySetBase;

    ~QuerySet() override = default;
    MaybeError Initialize();

    // Dawn API
    void DestroyImpl() override;
    void SetLabelImpl() override;

    std::vector<ComPtr<ID3D11Predicate>> mPredicates;
};

}  // namespace dawn::native::d3d11

#endif  // SRC_DAWN_NATIVE_D3D11_QUERYSETD3D11_H_
