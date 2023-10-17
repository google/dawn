// Copyright 2023 The Dawn & Tint Authors
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
    DAWN_ASSERT(unownedHandle != nullptr);
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
