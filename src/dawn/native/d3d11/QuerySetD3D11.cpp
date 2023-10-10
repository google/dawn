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

#include "dawn/native/d3d11/QuerySetD3D11.h"

#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/BufferD3D11.h"
#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"

namespace dawn::native::d3d11 {

ResultOrError<Ref<QuerySet>> QuerySet::Create(Device* device,
                                              const QuerySetDescriptor* descriptor) {
    Ref<QuerySet> querySet = AcquireRef(new QuerySet(device, descriptor));
    DAWN_TRY(querySet->Initialize());
    return querySet;
}

MaybeError QuerySet::Initialize() {
    DAWN_ASSERT(GetQueryType() == wgpu::QueryType::Occlusion);
    D3D11_QUERY_DESC queryDesc = {};
    queryDesc.Query = D3D11_QUERY_OCCLUSION_PREDICATE;

    for (uint32_t i = 0; i < GetQueryCount(); ++i) {
        ComPtr<ID3D11Predicate> d3d11Predicate;
        DAWN_TRY(CheckHRESULT(
            ToBackend(GetDevice())->GetD3D11Device()->CreatePredicate(&queryDesc, &d3d11Predicate),
            "ID3D11Device::CreateQuery"));
        mPredicates.push_back(std::move(d3d11Predicate));
    }
    SetLabelImpl();
    return {};
}

void QuerySet::DestroyImpl() {
    // TODO(crbug.com/dawn/831): DestroyImpl is called from two places.
    // - It may be called if the query set is explicitly destroyed with APIDestroy.
    //   This case is NOT thread-safe and needs proper synchronization with other
    //   simultaneous uses of the query set.
    // - It may be called when the last ref to the query set is dropped and it
    //   is implicitly destroyed. This case is thread-safe because there are no
    //   other threads using the query set since there are no other live refs.
    QuerySetBase::DestroyImpl();
    mPredicates.clear();
}

void QuerySet::SetLabelImpl() {
    for (const auto& predicate : mPredicates) {
        SetDebugName(ToBackend(GetDevice()), predicate.Get(), "Dawn_QuerySet", GetLabel());
    }
}

void QuerySet::BeginQuery(ID3D11DeviceContext* d3d11DeviceContext, uint32_t query) {
    d3d11DeviceContext->Begin(mPredicates[query].Get());
}

void QuerySet::EndQuery(ID3D11DeviceContext* d3d11DeviceContext, uint32_t query) {
    d3d11DeviceContext->End(mPredicates[query].Get());
}

MaybeError QuerySet::Resolve(CommandRecordingContext* commandContext,
                             uint32_t firstQuery,
                             uint32_t queryCount,
                             Buffer* destination,
                             uint64_t offset) {
    DAWN_TRY(destination->Clear(commandContext, 0, offset, queryCount * sizeof(uint64_t)));
    const auto& queryAvailability = GetQueryAvailability();
    ID3D11DeviceContext* d3d11DeviceContext = commandContext->GetD3D11DeviceContext();
    for (uint32_t i = 0; i < queryCount; ++i) {
        uint32_t queryIndex = i + firstQuery;
        if (queryAvailability[queryIndex]) {
            auto& predicate = mPredicates[queryIndex];
            d3d11DeviceContext->SetPredication(predicate.Get(), false);
            DAWN_TRY(destination->Clear(commandContext, 1, offset + i * sizeof(uint64_t),
                                        sizeof(uint64_t)));
            d3d11DeviceContext->SetPredication(nullptr, false);
        }
    }
    return {};
}

}  // namespace dawn::native::d3d11
