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

#include "dawn/native/d3d11/CommandRecordingContextD3D11.h"

#include <string>
#include <utility>

#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/Forward.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

namespace dawn::native::d3d11 {

MaybeError CommandRecordingContext::Open(Device* device) {
    ASSERT(!IsOpen());
    ASSERT(device);

    if (!mD3D11DeviceContext4) {
        ID3D11Device* d3d11Device = device->GetD3D11Device();

        ComPtr<ID3D11DeviceContext> d3d11DeviceContext;
        device->GetD3D11Device()->GetImmediateContext(&d3d11DeviceContext);

        ComPtr<ID3D11DeviceContext4> d3d11DeviceContext4;
        DAWN_TRY(
            CheckHRESULT(d3d11DeviceContext.As(&d3d11DeviceContext4),
                         "D3D11 querying immediate context for ID3D11DeviceContext4 interface"));

        mD3D11Device = d3d11Device;
        mD3D11DeviceContext4 = std::move(d3d11DeviceContext4);
    }

    mIsOpen = true;
    mNeedsSubmit = false;

    return {};
}

MaybeError CommandRecordingContext::ExecuteCommandList(Device* device) {
    // Consider using deferred DeviceContext.
    return {};
}

ID3D11Device* CommandRecordingContext::GetD3D11Device() const {
    return mD3D11Device.Get();
}

ID3D11DeviceContext* CommandRecordingContext::GetD3D11DeviceContext() const {
    return mD3D11DeviceContext4.Get();
}

ID3D11DeviceContext1* CommandRecordingContext::GetD3D11DeviceContext1() const {
    return mD3D11DeviceContext4.Get();
}

ID3D11DeviceContext4* CommandRecordingContext::GetD3D11DeviceContext4() const {
    return mD3D11DeviceContext4.Get();
}

void CommandRecordingContext::Release() {
    if (mIsOpen) {
        mIsOpen = false;
        mNeedsSubmit = false;
        mD3D11DeviceContext4 = nullptr;
        mD3D11Device = nullptr;
    }
}

bool CommandRecordingContext::IsOpen() const {
    return mIsOpen;
}

bool CommandRecordingContext::NeedsSubmit() const {
    return mNeedsSubmit;
}

void CommandRecordingContext::SetNeedsSubmit() {
    mNeedsSubmit = true;
}

}  // namespace dawn::native::d3d11
