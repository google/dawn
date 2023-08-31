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

#include "dawn/native/d3d11/SharedFenceD3D11.h"

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"

namespace dawn::native::d3d11 {

// static
ResultOrError<Ref<SharedFence>> SharedFence::Create(
    Device* device,
    const char* label,
    const SharedFenceDXGISharedHandleDescriptor* descriptor) {
    DAWN_INVALID_IF(descriptor->handle == nullptr, "shared HANDLE is missing.");

    HANDLE ownedHandle;
    if (!::DuplicateHandle(::GetCurrentProcess(), descriptor->handle, ::GetCurrentProcess(),
                           &ownedHandle, 0, FALSE, DUPLICATE_SAME_ACCESS)) {
        return DAWN_INTERNAL_ERROR("Failed to DuplicateHandle");
    }

    Ref<SharedFence> fence = AcquireRef(new SharedFence(device, label, ownedHandle));
    DAWN_TRY(CheckHRESULT(
        device->GetD3D11Device5()->OpenSharedFence(ownedHandle, IID_PPV_ARGS(&fence->mFence)),
        "D3D11 fence open shared handle"));
    return fence;
}

void SharedFence::DestroyImpl() {
    mFence = nullptr;
}

ID3D11Fence* SharedFence::GetD3DFence() const {
    return mFence.Get();
}

}  // namespace dawn::native::d3d11
