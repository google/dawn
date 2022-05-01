// Copyright 2021 The Tint Authors.
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

#include "src/tint/val/val.h"

#include "src/tint/utils/io/command.h"
#include "src/tint/utils/io/tmpfile.h"

#ifdef _WIN32
#include <Windows.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif  // _WIN32

namespace tint::val {

Result HlslUsingDXC(const std::string& dxc_path,
                    const std::string& source,
                    const EntryPointList& entry_points) {
    Result result;

    auto dxc = utils::Command(dxc_path);
    if (!dxc.Found()) {
        result.output = "DXC not found at '" + std::string(dxc_path) + "'";
        result.failed = true;
        return result;
    }

    utils::TmpFile file;
    file << source;

    for (auto ep : entry_points) {
        const char* profile = "";

        switch (ep.second) {
            case ast::PipelineStage::kNone:
                result.output = "Invalid PipelineStage";
                result.failed = true;
                return result;
            case ast::PipelineStage::kVertex:
                profile = "-T vs_6_0";
                break;
            case ast::PipelineStage::kFragment:
                profile = "-T ps_6_0";
                break;
            case ast::PipelineStage::kCompute:
                profile = "-T cs_6_0";
                break;
        }

        // Match Dawn's compile flags
        // See dawn\src\dawn_native\d3d12\RenderPipelineD3D12.cpp
        // and dawn_native\d3d12\ShaderModuleD3D12.cpp (GetDXCArguments)
        const char* compileFlags =
            "/Zpr "  // D3DCOMPILE_PACK_MATRIX_ROW_MAJOR
            "/Gis";  // D3DCOMPILE_IEEE_STRICTNESS

        auto res = dxc(profile, "-E " + ep.first, compileFlags, file.Path());
        if (!res.out.empty()) {
            if (!result.output.empty()) {
                result.output += "\n";
            }
            result.output += res.out;
        }
        if (!res.err.empty()) {
            if (!result.output.empty()) {
                result.output += "\n";
            }
            result.output += res.err;
        }
        result.failed = (res.error_code != 0);
    }

    if (entry_points.empty()) {
        result.output = "No entrypoint found";
        result.failed = true;
        return result;
    }

    return result;
}

#ifdef _WIN32
Result HlslUsingFXC(const std::string& source, const EntryPointList& entry_points) {
    Result result;

    // This library leaks if an error happens in this function, but it is ok
    // because it is loaded at most once, and the executables using HlslUsingFXC
    // are short-lived.
    HMODULE fxcLib = LoadLibraryA("d3dcompiler_47.dll");
    if (fxcLib == nullptr) {
        result.output = "Couldn't load FXC";
        result.failed = true;
        return result;
    }

    pD3DCompile d3dCompile = reinterpret_cast<pD3DCompile>(
        reinterpret_cast<void*>(GetProcAddress(fxcLib, "D3DCompile")));
    if (d3dCompile == nullptr) {
        result.output = "Couldn't load D3DCompile from FXC";
        result.failed = true;
        return result;
    }

    for (auto ep : entry_points) {
        const char* profile = "";
        switch (ep.second) {
            case ast::PipelineStage::kNone:
                result.output = "Invalid PipelineStage";
                result.failed = true;
                return result;
            case ast::PipelineStage::kVertex:
                profile = "vs_5_1";
                break;
            case ast::PipelineStage::kFragment:
                profile = "ps_5_1";
                break;
            case ast::PipelineStage::kCompute:
                profile = "cs_5_1";
                break;
        }

        // Match Dawn's compile flags
        // See dawn\src\dawn_native\d3d12\RenderPipelineD3D12.cpp
        UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL0 | D3DCOMPILE_PACK_MATRIX_ROW_MAJOR |
                            D3DCOMPILE_IEEE_STRICTNESS;

        ComPtr<ID3DBlob> compiledShader;
        ComPtr<ID3DBlob> errors;
        HRESULT cr = d3dCompile(source.c_str(),    // pSrcData
                                source.length(),   // SrcDataSize
                                nullptr,           // pSourceName
                                nullptr,           // pDefines
                                nullptr,           // pInclude
                                ep.first.c_str(),  // pEntrypoint
                                profile,           // pTarget
                                compileFlags,      // Flags1
                                0,                 // Flags2
                                &compiledShader,   // ppCode
                                &errors);          // ppErrorMsgs
        if (FAILED(cr)) {
            result.output = static_cast<char*>(errors->GetBufferPointer());
            result.failed = true;
            return result;
        }
    }

    FreeLibrary(fxcLib);

    if (entry_points.empty()) {
        result.output = "No entrypoint found";
        result.failed = true;
        return result;
    }

    return result;
}
#endif  // _WIN32

}  // namespace tint::val
