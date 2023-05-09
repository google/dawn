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

#include "dawn/native/d3d11/FenceD3D11.h"

#include <utility>

#include "dawn/common/Log.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/DeviceD3D12.h"

namespace dawn::native::d3d11 {

// static
ResultOrError<Ref<Fence>> Fence::CreateFromHandle(ID3D11Device5* device,
                                                  HANDLE unownedHandle,
                                                  UINT64 fenceValue) {
    ASSERT(unownedHandle != nullptr);
    HANDLE ownedHandle = nullptr;
    if (!::DuplicateHandle(::GetCurrentProcess(), unownedHandle, ::GetCurrentProcess(),
                           &ownedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        return DAWN_DEVICE_LOST_ERROR("D3D11 fence dup handle");
    }
    ComPtr<ID3D11Fence> d3d11Fence;
    DAWN_TRY_WITH_CLEANUP(
        CheckHRESULT(device->OpenSharedFence(ownedHandle, IID_PPV_ARGS(&d3d11Fence)),
                     "D3D11 fence open handle"),
        { ::CloseHandle(ownedHandle); });
    return AcquireRef(new Fence(std::move(d3d11Fence), fenceValue, ownedHandle));
}

Fence::Fence(ComPtr<ID3D11Fence> d3d11Fence, UINT64 fenceValue, HANDLE sharedHandle)
    : Base(fenceValue, sharedHandle), mD3D11Fence(std::move(d3d11Fence)) {}

Fence::~Fence() = default;

ID3D11Fence* Fence::GetD3D11Fence() const {
    return mD3D11Fence.Get();
}

}  // namespace dawn::native::d3d11
