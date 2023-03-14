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

#ifndef SRC_DAWN_NATIVE_D3D_PLATFORMFUNCTIONS_H_
#define SRC_DAWN_NATIVE_D3D_PLATFORMFUNCTIONS_H_

#include <d3dcompiler.h>

#include <string>

#include "dawn/common/DynamicLib.h"
#include "dawn/native/Error.h"
#include "dawn/native/d3d/d3d_platform.h"

namespace dawn::native::d3d {

// Loads the functions required from the platform dynamically so that we don't need to rely on
// them being present in the system. For example linking against d3d12.lib would prevent
// dawn_native from loading on Windows 7 system where d3d12.dll doesn't exist.
class PlatformFunctions {
  public:
    PlatformFunctions();
    virtual ~PlatformFunctions();

    MaybeError LoadFunctions();
    bool IsPIXEventRuntimeLoaded() const;
    bool IsDXCBinaryAvailable() const;

    // Functions from dxgi.dll
    using PFN_DXGI_GET_DEBUG_INTERFACE1 = HRESULT(WINAPI*)(UINT Flags,
                                                           REFIID riid,
                                                           _COM_Outptr_ void** pDebug);
    PFN_DXGI_GET_DEBUG_INTERFACE1 dxgiGetDebugInterface1 = nullptr;

    using PFN_CREATE_DXGI_FACTORY2 = HRESULT(WINAPI*)(UINT Flags,
                                                      REFIID riid,
                                                      _COM_Outptr_ void** ppFactory);
    PFN_CREATE_DXGI_FACTORY2 createDxgiFactory2 = nullptr;

    // Functions from dxcompiler.dll
    using PFN_DXC_CREATE_INSTANCE = HRESULT(WINAPI*)(REFCLSID rclsid,
                                                     REFIID riid,
                                                     _COM_Outptr_ void** ppCompiler);
    PFN_DXC_CREATE_INSTANCE dxcCreateInstance = nullptr;

    // Functions from d3d3compiler.dll
    pD3DCompile d3dCompile = nullptr;
    pD3DDisassemble d3dDisassemble = nullptr;

  private:
    MaybeError LoadDXGI();
    void LoadDXCLibraries();
    void LoadDXIL(const std::string& baseWindowsSDKPath);
    void LoadDXCompiler(const std::string& baseWindowsSDKPath);
    MaybeError LoadFXCompiler();
    void LoadPIXRuntime();

    DynamicLib mDXGILib;
    DynamicLib mDXILLib;
    DynamicLib mDXCompilerLib;
    DynamicLib mFXCompilerLib;
};

}  // namespace dawn::native::d3d

#endif  // SRC_DAWN_NATIVE_D3D_PLATFORMFUNCTIONS_H_
