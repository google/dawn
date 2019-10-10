// Copyright 2019 The Dawn Authors
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
#include "dawn_native/d3d12/CommandRecordingContext.h"
#include "dawn_native/d3d12/CommandAllocatorManager.h"

namespace dawn_native { namespace d3d12 {

    MaybeError CommandRecordingContext::Open(ID3D12Device* d3d12Device,
                                             CommandAllocatorManager* commandAllocationManager) {
        ASSERT(!IsOpen());
        if (mD3d12CommandList != nullptr) {
            const HRESULT hr = mD3d12CommandList->Reset(
                commandAllocationManager->ReserveCommandAllocator().Get(), nullptr);
            if (FAILED(hr)) {
                mD3d12CommandList.Reset();
                return DAWN_DEVICE_LOST_ERROR("Error resetting command list.");
            }
        } else {
            ComPtr<ID3D12GraphicsCommandList> d3d12GraphicsCommandList;
            const HRESULT hr = d3d12Device->CreateCommandList(
                0, D3D12_COMMAND_LIST_TYPE_DIRECT,
                commandAllocationManager->ReserveCommandAllocator().Get(), nullptr,
                IID_PPV_ARGS(&d3d12GraphicsCommandList));
            if (FAILED(hr)) {
                return DAWN_DEVICE_LOST_ERROR("Error creating a direct command list.");
            }
            mD3d12CommandList = std::move(d3d12GraphicsCommandList);
        }

        mIsOpen = true;

        return {};
    }

    ResultOrError<ID3D12GraphicsCommandList*> CommandRecordingContext::Close() {
        ASSERT(IsOpen());
        mIsOpen = false;
        const HRESULT hr = mD3d12CommandList->Close();
        if (FAILED(hr)) {
            mD3d12CommandList.Reset();
            return DAWN_DEVICE_LOST_ERROR("Error closing pending command list.");
        }
        return mD3d12CommandList.Get();
    }

    ID3D12GraphicsCommandList* CommandRecordingContext::GetCommandList() const {
        ASSERT(mD3d12CommandList != nullptr);
        ASSERT(IsOpen());
        return mD3d12CommandList.Get();
    }

    void CommandRecordingContext::Release() {
        mD3d12CommandList.Reset();
        mIsOpen = false;
    }

    bool CommandRecordingContext::IsOpen() const {
        return mIsOpen;
    }

}}  // namespace dawn_native::d3d12
