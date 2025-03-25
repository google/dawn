// Copyright 2018 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"

#include <array>
#include <sstream>
#include <string>
#include <utility>

#include "dawn/common/SystemUtils.h"

namespace dawn::native::d3d12 {

PlatformFunctions::PlatformFunctions() = default;
PlatformFunctions::~PlatformFunctions() = default;

MaybeError PlatformFunctions::LoadFunctions() {
    DAWN_TRY(Base::LoadFunctions());

    const auto modulePath = GetModuleDirectory();
    const std::string& pathToPrepend = modulePath.value_or("");

    LoadDXCLibraries(pathToPrepend);
    DAWN_TRY(LoadD3D12());
    DAWN_TRY(LoadD3D11());
    LoadPIXRuntime(pathToPrepend);
    return {};
}

MaybeError PlatformFunctions::LoadD3D12() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3d12CreateDevice = &D3D12CreateDevice;
    d3d12GetDebugInterface = &D3D12GetDebugInterface;
    d3d12SerializeRootSignature = &D3D12SerializeRootSignature;
    d3d12CreateRootSignatureDeserializer = &D3D12CreateRootSignatureDeserializer;
    d3d12SerializeVersionedRootSignature = &D3D12SerializeVersionedRootSignature;
    d3d12CreateVersionedRootSignatureDeserializer = &D3D12CreateVersionedRootSignatureDeserializer;
#else
    std::string error;
    if (!mD3D12Lib.OpenSystemLibrary(L"d3d12.dll", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateDevice, "D3D12CreateDevice", &error) ||
        !mD3D12Lib.GetProc(&d3d12GetDebugInterface, "D3D12GetDebugInterface", &error) ||
        !mD3D12Lib.GetProc(&d3d12SerializeRootSignature, "D3D12SerializeRootSignature", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateRootSignatureDeserializer,
                           "D3D12CreateRootSignatureDeserializer", &error) ||
        !mD3D12Lib.GetProc(&d3d12SerializeVersionedRootSignature,
                           "D3D12SerializeVersionedRootSignature", &error) ||
        !mD3D12Lib.GetProc(&d3d12CreateVersionedRootSignatureDeserializer,
                           "D3D12CreateVersionedRootSignatureDeserializer", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif

    return {};
}

MaybeError PlatformFunctions::LoadD3D11() {
#if DAWN_PLATFORM_IS(WINUWP)
    d3d11on12CreateDevice = &D3D11On12CreateDevice;
#else
    std::string error;
    if (!mD3D11Lib.OpenSystemLibrary(L"d3d11.dll", &error) ||
        !mD3D11Lib.GetProc(&d3d11on12CreateDevice, "D3D11On12CreateDevice", &error)) {
        return DAWN_INTERNAL_ERROR(error.c_str());
    }
#endif

    return {};
}

bool PlatformFunctions::IsPIXEventRuntimeLoaded() const {
    return mPIXEventRuntimeLib.Valid();
}

void PlatformFunctions::LoadPIXRuntime(const std::string& pathToPrepend) {
    // TODO(dawn:766):
    // In UWP PIX should be statically linked WinPixEventRuntime_UAP.lib
    // So maybe we should put WinPixEventRuntime as a third party package
    // Currently PIX is not going to be loaded in UWP since the following
    // mPIXEventRuntimeLib.Open will fail.
    if (!mPIXEventRuntimeLib.Open(pathToPrepend + "WinPixEventRuntime.dll") ||
        !mPIXEventRuntimeLib.GetProc(&pixBeginEventOnCommandList, "PIXBeginEventOnCommandList") ||
        !mPIXEventRuntimeLib.GetProc(&pixEndEventOnCommandList, "PIXEndEventOnCommandList") ||
        !mPIXEventRuntimeLib.GetProc(&pixSetMarkerOnCommandList, "PIXSetMarkerOnCommandList")) {
        mPIXEventRuntimeLib.Close();
    }
}

void PlatformFunctions::LoadDXCLibraries(const std::string& pathToPrepend) {
    // TODO(dawn:766)
    // Statically linked with dxcompiler.lib in UWP
    // currently linked with dxcompiler.lib making CoreApp unable to activate
    // LoadDXIL and LoadDXCompiler will fail in UWP, but LoadFunctions() can still be
    // successfully executed.

    DynamicLib dxilLib;
    // DXIL must be loaded before DXC, otherwise shader signing is unavailable
    if (!dxilLib.Open(pathToPrepend + "dxil.dll")) {
        return;
    }

    DynamicLib dxCompilerLib;
    if (!dxCompilerLib.Open(pathToPrepend + "dxcompiler.dll") ||
        !dxCompilerLib.GetProc(&dxcCreateInstance, "DxcCreateInstance", nullptr)) {
        return;
    }

    mDXCompilerLib = std::move(dxCompilerLib);
    mDXILLib = std::move(dxilLib);
}

// Use Backend::IsDXCAvailable if possible, which also check the DXC is no older than a given
// version
bool PlatformFunctions::IsDXCBinaryAvailable() const {
    return mDXILLib.Valid() && mDXCompilerLib.Valid();
}

}  // namespace dawn::native::d3d12
