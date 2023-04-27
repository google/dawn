// Copyright 2018 The Dawn Authors
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

#ifndef SRC_DAWN_NATIVE_D3D12_PLATFORMFUNCTIONS_H_
#define SRC_DAWN_NATIVE_D3D12_PLATFORMFUNCTIONS_H_

#include "dawn/native/d3d/PlatformFunctions.h"
#include "dawn/native/d3d12/d3d12_platform.h"

#include "dawn/common/DynamicLib.h"

namespace dawn::native::d3d12 {

class PlatformFunctions final : public d3d::PlatformFunctions {
  public:
    PlatformFunctions();
    ~PlatformFunctions() override;

    MaybeError LoadFunctions();
    bool IsPIXEventRuntimeLoaded() const;

    // Functions from d3d12.dll
    PFN_D3D12_CREATE_DEVICE d3d12CreateDevice = nullptr;
    PFN_D3D12_GET_DEBUG_INTERFACE d3d12GetDebugInterface = nullptr;

    PFN_D3D12_SERIALIZE_ROOT_SIGNATURE d3d12SerializeRootSignature = nullptr;
    PFN_D3D12_CREATE_ROOT_SIGNATURE_DESERIALIZER d3d12CreateRootSignatureDeserializer = nullptr;
    PFN_D3D12_SERIALIZE_VERSIONED_ROOT_SIGNATURE d3d12SerializeVersionedRootSignature = nullptr;
    PFN_D3D12_CREATE_VERSIONED_ROOT_SIGNATURE_DESERIALIZER
    d3d12CreateVersionedRootSignatureDeserializer = nullptr;

    // Functions from WinPixEventRuntime.dll
    using PFN_PIX_END_EVENT_ON_COMMAND_LIST =
        HRESULT(WINAPI*)(ID3D12GraphicsCommandList* commandList);

    PFN_PIX_END_EVENT_ON_COMMAND_LIST pixEndEventOnCommandList = nullptr;

    using PFN_PIX_BEGIN_EVENT_ON_COMMAND_LIST = HRESULT(
        WINAPI*)(ID3D12GraphicsCommandList* commandList, UINT64 color, _In_ PCSTR formatString);

    PFN_PIX_BEGIN_EVENT_ON_COMMAND_LIST pixBeginEventOnCommandList = nullptr;

    using PFN_SET_MARKER_ON_COMMAND_LIST = HRESULT(WINAPI*)(ID3D12GraphicsCommandList* commandList,
                                                            UINT64 color,
                                                            _In_ PCSTR formatString);

    PFN_SET_MARKER_ON_COMMAND_LIST pixSetMarkerOnCommandList = nullptr;

  private:
    using Base = d3d::PlatformFunctions;

    MaybeError LoadD3D12();
    void LoadPIXRuntime();

    DynamicLib mD3D12Lib;
    DynamicLib mD3D11Lib;
    DynamicLib mPIXEventRuntimeLib;
};

}  // namespace dawn::native::d3d12

#endif  // SRC_DAWN_NATIVE_D3D12_PLATFORMFUNCTIONS_H_
