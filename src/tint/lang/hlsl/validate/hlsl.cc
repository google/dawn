// Copyright 2021 The Dawn & Tint Authors
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

#include <string>

#include "src/tint/lang/hlsl/validate/val.h"

#include "src/tint/utils/command/command.h"
#include "src/tint/utils/file/tmpfile.h"
#include "src/tint/utils/text/string.h"

#ifdef _WIN32
#include <Windows.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>

#include <wrl.h>
using Microsoft::WRL::ComPtr;
#endif  // _WIN32

namespace tint::hlsl::validate {

Result UsingDXC(const std::string& dxc_path,
                const std::string& source,
                const EntryPointList& entry_points,
                bool require_16bit_types) {
    Result result;

    auto dxc = tint::Command(dxc_path);
    if (!dxc.Found()) {
        result.output = "DXC not found at '" + std::string(dxc_path) + "'";
        result.failed = true;
        return result;
    }

    // Native 16-bit types, e.g. float16_t, require SM6.2. Otherwise we use SM6.0.
    const char* shader_model_version = require_16bit_types ? "6_2" : "6_0";

    tint::TmpFile file;
    file << source;

    for (auto ep : entry_points) {
        const char* stage_prefix = "";

        switch (ep.second) {
            case ast::PipelineStage::kNone:
                result.output = "Invalid PipelineStage";
                result.failed = true;
                return result;
            case ast::PipelineStage::kVertex:
                stage_prefix = "vs";
                break;
            case ast::PipelineStage::kFragment:
                stage_prefix = "ps";
                break;
            case ast::PipelineStage::kCompute:
                stage_prefix = "cs";
                break;
        }

        // Match Dawn's compile flags
        // See dawn\src\dawn_native\d3d12\RenderPipelineD3D12.cpp
        // and dawn_native\d3d\ShaderUtils.cpp (GetDXCArguments)
        auto res = dxc(
            "-T " + std::string(stage_prefix) + "_" + std::string(shader_model_version),  // Profile
            "-HV 2018",                                        // Use HLSL 2018
            "-E " + ep.first,                                  // Entry point
            "/Zpr",                                            // D3DCOMPILE_PACK_MATRIX_ROW_MAJOR
            "/Gis",                                            // D3DCOMPILE_IEEE_STRICTNESS
            require_16bit_types ? "-enable-16bit-types" : "",  // Enable 16-bit if required
            file.Path());
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

        // Remove the temporary file name from the output to keep output deterministic
        result.output = tint::ReplaceAll(result.output, file.Path(), "shader.hlsl");
    }

    if (entry_points.empty()) {
        result.output = "No entrypoint found";
        result.failed = true;
        return result;
    }

    return result;
}

#ifdef _WIN32
Result UsingFXC(const std::string& fxc_path,
                const std::string& source,
                const EntryPointList& entry_points) {
    Result result;

    // This library leaks if an error happens in this function, but it is ok
    // because it is loaded at most once, and the executables using UsingFXC
    // are short-lived.
    HMODULE fxcLib = LoadLibraryA(fxc_path.c_str());
    if (fxcLib == nullptr) {
        result.output = "Couldn't load FXC";
        result.failed = true;
        return result;
    }

    auto* d3dCompile = reinterpret_cast<pD3DCompile>(
        reinterpret_cast<void*>(GetProcAddress(fxcLib, "D3DCompile")));
    auto* d3dDisassemble = reinterpret_cast<pD3DDisassemble>(
        reinterpret_cast<void*>(GetProcAddress(fxcLib, "D3DDisassemble")));

    if (d3dCompile == nullptr) {
        result.output = "Couldn't load D3DCompile from FXC";
        result.failed = true;
        return result;
    }
    if (d3dDisassemble == nullptr) {
        result.output = "Couldn't load D3DDisassemble from FXC";
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
        HRESULT res = d3dCompile(source.c_str(),    // pSrcData
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
        if (FAILED(res)) {
            result.output = static_cast<char*>(errors->GetBufferPointer());
            result.failed = true;
            return result;
        } else {
            ComPtr<ID3DBlob> disassembly;
            res = d3dDisassemble(compiledShader->GetBufferPointer(),
                                 compiledShader->GetBufferSize(), 0, "", &disassembly);
            if (FAILED(res)) {
                result.output = "failed to disassemble shader";
            } else {
                result.output = static_cast<char*>(disassembly->GetBufferPointer());
            }
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

}  // namespace tint::hlsl::validate
