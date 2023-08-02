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

#include "dawn/native/d3d/ShaderUtils.h"

#include <sstream>
#include <string>
#include <utility>
#include <vector>

#include "dawn/native/d3d/BlobD3D.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d/DeviceD3D.h"
#include "dawn/native/d3d/PlatformFunctions.h"
#include "dawn/native/d3d/UtilsD3D.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d {

namespace {

std::vector<const wchar_t*> GetDXCArguments(uint32_t compileFlags, bool enable16BitTypes) {
    std::vector<const wchar_t*> arguments;
    if (compileFlags & D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY) {
        arguments.push_back(L"/Gec");
    }
    if (compileFlags & D3DCOMPILE_IEEE_STRICTNESS) {
        arguments.push_back(L"/Gis");
    }
    constexpr uint32_t d3dCompileFlagsBits = D3DCOMPILE_OPTIMIZATION_LEVEL2;
    if (compileFlags & d3dCompileFlagsBits) {
        switch (compileFlags & D3DCOMPILE_OPTIMIZATION_LEVEL2) {
            case D3DCOMPILE_OPTIMIZATION_LEVEL0:
                arguments.push_back(L"/O0");
                break;
            case D3DCOMPILE_OPTIMIZATION_LEVEL2:
                arguments.push_back(L"/O2");
                break;
            case D3DCOMPILE_OPTIMIZATION_LEVEL3:
                arguments.push_back(L"/O3");
                break;
        }
    }
    if (compileFlags & D3DCOMPILE_DEBUG) {
        arguments.push_back(L"/Zi");
    }
    if (compileFlags & D3DCOMPILE_PACK_MATRIX_ROW_MAJOR) {
        arguments.push_back(L"/Zpr");
    }
    if (compileFlags & D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR) {
        arguments.push_back(L"/Zpc");
    }
    if (compileFlags & D3DCOMPILE_AVOID_FLOW_CONTROL) {
        arguments.push_back(L"/Gfa");
    }
    if (compileFlags & D3DCOMPILE_PREFER_FLOW_CONTROL) {
        arguments.push_back(L"/Gfp");
    }
    if (compileFlags & D3DCOMPILE_RESOURCES_MAY_ALIAS) {
        arguments.push_back(L"/res_may_alias");
    }

    if (enable16BitTypes) {
        // enable-16bit-types are only allowed in -HV 2018 (default)
        arguments.push_back(L"/enable-16bit-types");
    }

    arguments.push_back(L"-HV");
    arguments.push_back(L"2018");

    return arguments;
}

ResultOrError<ComPtr<IDxcBlob>> CompileShaderDXC(const d3d::D3DBytecodeCompilationRequest& r,
                                                 const std::string& entryPointName,
                                                 const std::string& hlslSource) {
    ComPtr<IDxcBlobEncoding> sourceBlob;
    DAWN_TRY(CheckHRESULT(r.dxcLibrary->CreateBlobWithEncodingFromPinned(
                              hlslSource.c_str(), hlslSource.length(), CP_UTF8, &sourceBlob),
                          "DXC create blob"));

    std::wstring entryPointW;
    DAWN_TRY_ASSIGN(entryPointW, d3d::ConvertStringToWstring(entryPointName));

    std::vector<const wchar_t*> arguments = GetDXCArguments(r.compileFlags, r.hasShaderF16Feature);

    ComPtr<IDxcOperationResult> result;
    DAWN_TRY(CheckHRESULT(r.dxcCompiler->Compile(sourceBlob.Get(), nullptr, entryPointW.c_str(),
                                                 r.dxcShaderProfile.data(), arguments.data(),
                                                 arguments.size(), nullptr, 0, nullptr, &result),
                          "DXC compile"));

    HRESULT hr;
    DAWN_TRY(CheckHRESULT(result->GetStatus(&hr), "DXC get status"));

    if (FAILED(hr)) {
        ComPtr<IDxcBlobEncoding> errors;
        DAWN_TRY(CheckHRESULT(result->GetErrorBuffer(&errors), "DXC get error buffer"));

        return DAWN_VALIDATION_ERROR("DXC compile failed with: %s",
                                     static_cast<char*>(errors->GetBufferPointer()));
    }

    ComPtr<IDxcBlob> compiledShader;
    DAWN_TRY(CheckHRESULT(result->GetResult(&compiledShader), "DXC get result"));
    return std::move(compiledShader);
}

ResultOrError<ComPtr<ID3DBlob>> CompileShaderFXC(const d3d::D3DBytecodeCompilationRequest& r,
                                                 const std::string& entryPointName,
                                                 const std::string& hlslSource) {
    ComPtr<ID3DBlob> compiledShader;
    ComPtr<ID3DBlob> errors;

    DAWN_INVALID_IF(FAILED(r.d3dCompile(hlslSource.c_str(), hlslSource.length(), nullptr, nullptr,
                                        nullptr, entryPointName.c_str(), r.fxcShaderProfile.data(),
                                        r.compileFlags, 0, &compiledShader, &errors)),
                    "D3D compile failed with: %s", static_cast<char*>(errors->GetBufferPointer()));

    return std::move(compiledShader);
}

MaybeError TranslateToHLSL(d3d::HlslCompilationRequest r,
                           CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*> tracePlatform,
                           std::string* remappedEntryPointName,
                           CompiledShader* compiledShader) {
    std::ostringstream errorStream;
    errorStream << "Tint HLSL failure:" << std::endl;

    tint::ast::transform::Manager transformManager;
    tint::ast::transform::DataMap transformInputs;

    // Run before the renamer so that the entry point name matches `entryPointName` still.
    transformManager.Add<tint::ast::transform::SingleEntryPoint>();
    transformInputs.Add<tint::ast::transform::SingleEntryPoint::Config>(r.entryPointName.data());

    // Needs to run before all other transforms so that they can use builtin names safely.
    transformManager.Add<tint::ast::transform::Renamer>();
    if (r.disableSymbolRenaming) {
        // We still need to rename HLSL reserved keywords
        transformInputs.Add<tint::ast::transform::Renamer::Config>(
            tint::ast::transform::Renamer::Target::kHlslKeywords);
    }

    if (r.stage == SingleShaderStage::Vertex) {
        transformManager.Add<tint::ast::transform::FirstIndexOffset>();
        transformInputs.Add<tint::ast::transform::FirstIndexOffset::BindingPoint>(
            r.firstIndexOffsetShaderRegister, r.firstIndexOffsetRegisterSpace);
    }

    if (r.substituteOverrideConfig) {
        // This needs to run after SingleEntryPoint transform which removes unused overrides for
        // current entry point.
        transformManager.Add<tint::ast::transform::SubstituteOverride>();
        transformInputs.Add<tint::ast::transform::SubstituteOverride::Config>(
            std::move(r.substituteOverrideConfig).value());
    }

    tint::Program transformedProgram;
    tint::ast::transform::DataMap transformOutputs;
    {
        TRACE_EVENT0(tracePlatform.UnsafeGetValue(), General, "RunTransforms");
        DAWN_TRY_ASSIGN(transformedProgram,
                        RunTransforms(&transformManager, r.inputProgram, transformInputs,
                                      &transformOutputs, nullptr));
    }

    if (auto* data = transformOutputs.Get<tint::ast::transform::Renamer::Data>()) {
        auto it = data->remappings.find(r.entryPointName.data());
        if (it != data->remappings.end()) {
            *remappedEntryPointName = it->second;
        } else {
            DAWN_INVALID_IF(!r.disableSymbolRenaming,
                            "Could not find remapped name for entry point.");

            *remappedEntryPointName = r.entryPointName;
        }
    } else {
        return DAWN_VALIDATION_ERROR("Transform output missing renamer data.");
    }

    if (r.stage == SingleShaderStage::Compute) {
        // Validate workgroup size after program runs transforms.
        Extent3D _;
        DAWN_TRY_ASSIGN(_, ValidateComputeStageWorkgroupSize(
                               transformedProgram, remappedEntryPointName->data(), r.limits));
    }

    bool usesVertexIndex = false;
    bool usesInstanceIndex = false;
    if (r.stage == SingleShaderStage::Vertex) {
        if (auto* data = transformOutputs.Get<tint::ast::transform::FirstIndexOffset::Data>()) {
            usesVertexIndex = data->has_vertex_index;
            usesInstanceIndex = data->has_instance_index;
        } else {
            return DAWN_VALIDATION_ERROR("Transform output missing first index offset data.");
        }
    }

    tint::hlsl::writer::Options options;
    options.disable_robustness = !r.isRobustnessEnabled;
    options.disable_workgroup_init = r.disableWorkgroupInit;
    options.binding_remapper_options = r.bindingRemapper;
    options.external_texture_options = r.externalTextureOptions;

    if (r.usesNumWorkgroups) {
        options.root_constant_binding_point =
            tint::BindingPoint{r.numWorkgroupsRegisterSpace, r.numWorkgroupsShaderRegister};
    }
    // TODO(dawn:549): HLSL generation outputs the indices into the
    // array_length_from_uniform buffer that were actually used. When the blob cache can
    // store more than compiled shaders, we should reflect these used indices and store
    // them as well. This would allow us to only upload root constants that are actually
    // read by the shader.
    options.array_length_from_uniform = r.arrayLengthFromUniform;

    if (r.stage == SingleShaderStage::Vertex) {
        // Now that only vertex shader can have interstage outputs.
        // Pass in the actually used interstage locations for tint to potentially truncate unused
        // outputs.
        options.interstage_locations = r.interstageLocations;
        options.truncate_interstage_variables = true;
    }

    options.polyfill_reflect_vec2_f32 = r.polyfillReflectVec2F32;

    options.binding_points_ignored_in_robustness_transform =
        std::move(r.bindingPointsIgnoredInRobustnessTransform);

    TRACE_EVENT0(tracePlatform.UnsafeGetValue(), General, "tint::hlsl::writer::Generate");
    auto result = tint::hlsl::writer::Generate(&transformedProgram, options);
    DAWN_INVALID_IF(!result, "An error occured while generating HLSL: %s", result.Failure());

    compiledShader->usesVertexIndex = usesVertexIndex;
    compiledShader->usesInstanceIndex = usesInstanceIndex;
    compiledShader->hlslSource = std::move(result->hlsl);
    return {};
}

std::string CompileFlagsToString(uint32_t compileFlags) {
    struct Flag {
        uint32_t value;
        const char* name;
    };
    constexpr Flag flags[] = {
    // Populated from d3dcompiler.h
#define F(f) Flag{f, #f}
        F(D3DCOMPILE_DEBUG),
        F(D3DCOMPILE_SKIP_VALIDATION),
        F(D3DCOMPILE_SKIP_OPTIMIZATION),
        F(D3DCOMPILE_PACK_MATRIX_ROW_MAJOR),
        F(D3DCOMPILE_PACK_MATRIX_COLUMN_MAJOR),
        F(D3DCOMPILE_PARTIAL_PRECISION),
        F(D3DCOMPILE_FORCE_VS_SOFTWARE_NO_OPT),
        F(D3DCOMPILE_FORCE_PS_SOFTWARE_NO_OPT),
        F(D3DCOMPILE_NO_PRESHADER),
        F(D3DCOMPILE_AVOID_FLOW_CONTROL),
        F(D3DCOMPILE_PREFER_FLOW_CONTROL),
        F(D3DCOMPILE_ENABLE_STRICTNESS),
        F(D3DCOMPILE_ENABLE_BACKWARDS_COMPATIBILITY),
        F(D3DCOMPILE_IEEE_STRICTNESS),
        F(D3DCOMPILE_RESERVED16),
        F(D3DCOMPILE_RESERVED17),
        F(D3DCOMPILE_WARNINGS_ARE_ERRORS),
        F(D3DCOMPILE_RESOURCES_MAY_ALIAS),
        F(D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES),
        F(D3DCOMPILE_ALL_RESOURCES_BOUND),
        F(D3DCOMPILE_DEBUG_NAME_FOR_SOURCE),
        F(D3DCOMPILE_DEBUG_NAME_FOR_BINARY),
#undef F
    };

    std::string result;
    for (const Flag& f : flags) {
        if ((compileFlags & f.value) != 0) {
            result += f.name + std::string("\n");
        }
    }

    // Optimization level must be handled separately as two bits are used, and the values
    // don't map neatly to 0-3.
    constexpr uint32_t d3dCompileFlagsBits = D3DCOMPILE_OPTIMIZATION_LEVEL2;
    switch (compileFlags & d3dCompileFlagsBits) {
        case D3DCOMPILE_OPTIMIZATION_LEVEL0:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL0";
            break;
        case D3DCOMPILE_OPTIMIZATION_LEVEL1:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL1";
            break;
        case D3DCOMPILE_OPTIMIZATION_LEVEL2:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL2";
            break;
        case D3DCOMPILE_OPTIMIZATION_LEVEL3:
            result += "D3DCOMPILE_OPTIMIZATION_LEVEL3";
            break;
    }
    result += std::string("\n");

    return result;
}

}  // anonymous namespace

ResultOrError<CompiledShader> CompileShader(d3d::D3DCompilationRequest r) {
    CompiledShader compiledShader;
    bool shouldDumpShader = r.hlsl.dumpShaders;
    // Compile the source shader to HLSL.
    std::string remappedEntryPoint;
    DAWN_TRY(
        TranslateToHLSL(std::move(r.hlsl), r.tracePlatform, &remappedEntryPoint, &compiledShader));

    switch (r.bytecode.compiler) {
        case d3d::Compiler::DXC: {
            TRACE_EVENT0(r.tracePlatform.UnsafeGetValue(), General, "CompileShaderDXC");
            ComPtr<IDxcBlob> compiledDXCShader;
            DAWN_TRY_ASSIGN(compiledDXCShader, CompileShaderDXC(r.bytecode, remappedEntryPoint,
                                                                compiledShader.hlslSource));
            compiledShader.shaderBlob = CreateBlob(std::move(compiledDXCShader));
            break;
        }
        case d3d::Compiler::FXC: {
            TRACE_EVENT0(r.tracePlatform.UnsafeGetValue(), General, "CompileShaderFXC");
            ComPtr<ID3DBlob> compiledFXCShader;
            DAWN_TRY_ASSIGN(compiledFXCShader, CompileShaderFXC(r.bytecode, remappedEntryPoint,
                                                                compiledShader.hlslSource));
            compiledShader.shaderBlob = CreateBlob(std::move(compiledFXCShader));
            break;
        }
    }

    // If dumpShaders is false, we don't need the HLSL for logging. Clear the contents so it
    // isn't stored into the cache.
    if (!shouldDumpShader) {
        compiledShader.hlslSource = "";
    }
    return compiledShader;
}

void DumpCompiledShader(Device* device,
                        const CompiledShader& compiledShader,
                        uint32_t compileFlags) {
    std::ostringstream dumpedMsg;
    // The HLSL may be empty if compilation failed.
    if (!compiledShader.hlslSource.empty()) {
        dumpedMsg << "/* Dumped generated HLSL */" << std::endl
                  << compiledShader.hlslSource << std::endl;
    }

    // The blob may be empty if FXC/DXC compilation failed.
    const Blob& shaderBlob = compiledShader.shaderBlob;
    if (!shaderBlob.Empty()) {
        if (device->IsToggleEnabled(Toggle::UseDXC)) {
            dumpedMsg << "/* DXC compile flags */ " << std::endl
                      << CompileFlagsToString(compileFlags) << std::endl;
            dumpedMsg << "/* Dumped disassembled DXIL */" << std::endl;
            ComPtr<IDxcBlobEncoding> dxcBlob;
            ComPtr<IDxcBlobEncoding> disassembly;
            if (FAILED(device->GetDxcLibrary()->CreateBlobWithEncodingFromPinned(
                    shaderBlob.Data(), shaderBlob.Size(), 0, &dxcBlob)) ||
                FAILED(device->GetDxcCompiler()->Disassemble(dxcBlob.Get(), &disassembly))) {
                dumpedMsg << "DXC disassemble failed" << std::endl;
            } else {
                dumpedMsg << std::string_view(
                    static_cast<const char*>(disassembly->GetBufferPointer()),
                    disassembly->GetBufferSize());
            }
        } else {
            dumpedMsg << "/* FXC compile flags */ " << std::endl
                      << CompileFlagsToString(compileFlags) << std::endl;
            dumpedMsg << "/* Dumped disassembled DXBC */" << std::endl;
            ComPtr<ID3DBlob> disassembly;
            UINT flags =
                // Some literals are printed as floats with precision(6) which is not enough
                // precision for values very close to 0, so always print literals as hex values.
                D3D_DISASM_PRINT_HEX_LITERALS;
            if (FAILED(device->GetFunctions()->d3dDisassemble(shaderBlob.Data(), shaderBlob.Size(),
                                                              flags, nullptr, &disassembly))) {
                dumpedMsg << "D3D disassemble failed" << std::endl;
            } else {
                dumpedMsg << std::string_view(
                    static_cast<const char*>(disassembly->GetBufferPointer()),
                    disassembly->GetBufferSize());
            }
        }
    }

    std::string logMessage = dumpedMsg.str();
    if (!logMessage.empty()) {
        device->EmitLog(WGPULoggingType_Info, logMessage.c_str());
    }
}

}  // namespace dawn::native::d3d
