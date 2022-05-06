// Copyright 2017 The Dawn Authors
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

#include "dawn/native/d3d12/ShaderModuleD3D12.h"

#include <d3dcompiler.h>

#include <map>
#include <sstream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Log.h"
#include "dawn/common/WindowsUtils.h"
#include "dawn/native/CacheKey.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctions.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d12 {

namespace {
ResultOrError<uint64_t> GetDXCompilerVersion(ComPtr<IDxcValidator> dxcValidator) {
    ComPtr<IDxcVersionInfo> versionInfo;
    DAWN_TRY(CheckHRESULT(dxcValidator.As(&versionInfo),
                          "D3D12 QueryInterface IDxcValidator to IDxcVersionInfo"));

    uint32_t compilerMajor, compilerMinor;
    DAWN_TRY(CheckHRESULT(versionInfo->GetVersion(&compilerMajor, &compilerMinor),
                          "IDxcVersionInfo::GetVersion"));

    // Pack both into a single version number.
    return (uint64_t(compilerMajor) << uint64_t(32)) + compilerMinor;
}

uint64_t GetD3DCompilerVersion() {
    return D3D_COMPILER_VERSION;
}

struct CompareBindingPoint {
    constexpr bool operator()(const tint::transform::BindingPoint& lhs,
                              const tint::transform::BindingPoint& rhs) const {
        if (lhs.group != rhs.group) {
            return lhs.group < rhs.group;
        } else {
            return lhs.binding < rhs.binding;
        }
    }
};

void Serialize(std::stringstream& output, const tint::ast::Access& access) {
    output << access;
}

void Serialize(std::stringstream& output, const tint::transform::BindingPoint& binding_point) {
    output << "(BindingPoint";
    output << " group=" << binding_point.group;
    output << " binding=" << binding_point.binding;
    output << ")";
}

template <typename T, typename = typename std::enable_if<std::is_fundamental<T>::value>::type>
void Serialize(std::stringstream& output, const T& val) {
    output << val;
}

template <typename T>
void Serialize(std::stringstream& output,
               const std::unordered_map<tint::transform::BindingPoint, T>& map) {
    output << "(map";

    std::map<tint::transform::BindingPoint, T, CompareBindingPoint> sorted(map.begin(), map.end());
    for (auto& [bindingPoint, value] : sorted) {
        output << " ";
        Serialize(output, bindingPoint);
        output << "=";
        Serialize(output, value);
    }
    output << ")";
}

void Serialize(std::stringstream& output,
               const tint::writer::ArrayLengthFromUniformOptions& arrayLengthFromUniform) {
    output << "(ArrayLengthFromUniformOptions";
    output << " ubo_binding=";
    Serialize(output, arrayLengthFromUniform.ubo_binding);
    output << " bindpoint_to_size_index=";
    Serialize(output, arrayLengthFromUniform.bindpoint_to_size_index);
    output << ")";
}

// 32 bit float has 7 decimal digits of precision so setting n to 8 should be enough
std::string FloatToStringWithPrecision(float v, std::streamsize n = 8) {
    std::ostringstream out;
    out.precision(n);
    out << std::fixed << v;
    return out.str();
}

std::string GetHLSLValueString(EntryPointMetadata::OverridableConstant::Type dawnType,
                               const OverridableConstantScalar* entry,
                               double value = 0) {
    switch (dawnType) {
        case EntryPointMetadata::OverridableConstant::Type::Boolean:
            return std::to_string(entry ? entry->b : static_cast<int32_t>(value));
        case EntryPointMetadata::OverridableConstant::Type::Float32:
            return FloatToStringWithPrecision(entry ? entry->f32 : static_cast<float>(value));
        case EntryPointMetadata::OverridableConstant::Type::Int32:
            return std::to_string(entry ? entry->i32 : static_cast<int32_t>(value));
        case EntryPointMetadata::OverridableConstant::Type::Uint32:
            return std::to_string(entry ? entry->u32 : static_cast<uint32_t>(value));
        default:
            UNREACHABLE();
    }
}

constexpr char kSpecConstantPrefix[] = "WGSL_SPEC_CONSTANT_";

void GetOverridableConstantsDefines(
    std::vector<std::pair<std::string, std::string>>* defineStrings,
    const PipelineConstantEntries* pipelineConstantEntries,
    const EntryPointMetadata::OverridableConstantsMap* shaderEntryPointConstants) {
    std::unordered_set<std::string> overriddenConstants;

    // Set pipeline overridden values
    for (const auto& [name, value] : *pipelineConstantEntries) {
        overriddenConstants.insert(name);

        // This is already validated so `name` must exist
        const auto& moduleConstant = shaderEntryPointConstants->at(name);

        defineStrings->emplace_back(
            kSpecConstantPrefix + std::to_string(static_cast<int32_t>(moduleConstant.id)),
            GetHLSLValueString(moduleConstant.type, nullptr, value));
    }

    // Set shader initialized default values
    for (const auto& iter : *shaderEntryPointConstants) {
        const std::string& name = iter.first;
        if (overriddenConstants.count(name) != 0) {
            // This constant already has overridden value
            continue;
        }

        const auto& moduleConstant = shaderEntryPointConstants->at(name);

        // Uninitialized default values are okay since they ar only defined to pass
        // compilation but not used
        defineStrings->emplace_back(
            kSpecConstantPrefix + std::to_string(static_cast<int32_t>(moduleConstant.id)),
            GetHLSLValueString(moduleConstant.type, &moduleConstant.defaultValue));
    }
}

// The inputs to a shader compilation. These have been intentionally isolated from the
// device to help ensure that the pipeline cache key contains all inputs for compilation.
struct ShaderCompilationRequest {
    enum Compiler { FXC, DXC };

    // Common inputs
    Compiler compiler;
    const tint::Program* program;
    const char* entryPointName;
    SingleShaderStage stage;
    uint32_t compileFlags;
    bool disableSymbolRenaming;
    tint::transform::BindingRemapper::BindingPoints remappedBindingPoints;
    tint::transform::BindingRemapper::AccessControls remappedAccessControls;
    bool isRobustnessEnabled;
    bool usesNumWorkgroups;
    uint32_t numWorkgroupsRegisterSpace;
    uint32_t numWorkgroupsShaderRegister;
    tint::writer::ArrayLengthFromUniformOptions arrayLengthFromUniform;
    std::vector<std::pair<std::string, std::string>> defineStrings;

    // FXC/DXC common inputs
    bool disableWorkgroupInit;

    // FXC inputs
    uint64_t fxcVersion;

    // DXC inputs
    uint64_t dxcVersion;
    const D3D12DeviceInfo* deviceInfo;
    bool hasShaderFloat16Feature;

    static ResultOrError<ShaderCompilationRequest> Create(
        const char* entryPointName,
        SingleShaderStage stage,
        const PipelineLayout* layout,
        uint32_t compileFlags,
        const Device* device,
        const tint::Program* program,
        const EntryPointMetadata& entryPoint,
        const ProgrammableStage& programmableStage) {
        Compiler compiler;
        uint64_t dxcVersion = 0;
        if (device->IsToggleEnabled(Toggle::UseDXC)) {
            compiler = Compiler::DXC;
            DAWN_TRY_ASSIGN(dxcVersion, GetDXCompilerVersion(device->GetDxcValidator()));
        } else {
            compiler = Compiler::FXC;
        }

        using tint::transform::BindingPoint;
        using tint::transform::BindingRemapper;

        BindingRemapper::BindingPoints remappedBindingPoints;
        BindingRemapper::AccessControls remappedAccessControls;

        tint::writer::ArrayLengthFromUniformOptions arrayLengthFromUniform;
        arrayLengthFromUniform.ubo_binding = {
            layout->GetDynamicStorageBufferLengthsRegisterSpace(),
            layout->GetDynamicStorageBufferLengthsShaderRegister()};

        const BindingInfoArray& moduleBindingInfo = entryPoint.bindings;
        for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
            const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
            const auto& groupBindingInfo = moduleBindingInfo[group];

            // d3d12::BindGroupLayout packs the bindings per HLSL register-space. We modify
            // the Tint AST to make the "bindings" decoration match the offset chosen by
            // d3d12::BindGroupLayout so that Tint produces HLSL with the correct registers
            // assigned to each interface variable.
            for (const auto& [binding, bindingInfo] : groupBindingInfo) {
                BindingIndex bindingIndex = bgl->GetBindingIndex(binding);
                BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                             static_cast<uint32_t>(binding)};
                BindingPoint dstBindingPoint{static_cast<uint32_t>(group),
                                             bgl->GetShaderRegister(bindingIndex)};
                if (srcBindingPoint != dstBindingPoint) {
                    remappedBindingPoints.emplace(srcBindingPoint, dstBindingPoint);
                }

                // Declaring a read-only storage buffer in HLSL but specifying a storage
                // buffer in the BGL produces the wrong output. Force read-only storage
                // buffer bindings to be treated as UAV instead of SRV. Internal storage
                // buffer is a storage buffer used in the internal pipeline.
                const bool forceStorageBufferAsUAV =
                    (bindingInfo.buffer.type == wgpu::BufferBindingType::ReadOnlyStorage &&
                     (bgl->GetBindingInfo(bindingIndex).buffer.type ==
                          wgpu::BufferBindingType::Storage ||
                      bgl->GetBindingInfo(bindingIndex).buffer.type ==
                          kInternalStorageBufferBinding));
                if (forceStorageBufferAsUAV) {
                    remappedAccessControls.emplace(srcBindingPoint, tint::ast::Access::kReadWrite);
                }
            }

            // Add arrayLengthFromUniform options
            {
                for (const auto& bindingAndRegisterOffset :
                     layout->GetDynamicStorageBufferLengthInfo()[group].bindingAndRegisterOffsets) {
                    BindingNumber binding = bindingAndRegisterOffset.binding;
                    uint32_t registerOffset = bindingAndRegisterOffset.registerOffset;

                    BindingPoint bindingPoint{static_cast<uint32_t>(group),
                                              static_cast<uint32_t>(binding)};
                    // Get the renamed binding point if it was remapped.
                    auto it = remappedBindingPoints.find(bindingPoint);
                    if (it != remappedBindingPoints.end()) {
                        bindingPoint = it->second;
                    }

                    arrayLengthFromUniform.bindpoint_to_size_index.emplace(bindingPoint,
                                                                           registerOffset);
                }
            }
        }

        ShaderCompilationRequest request;
        request.compiler = compiler;
        request.program = program;
        request.entryPointName = entryPointName;
        request.stage = stage;
        request.compileFlags = compileFlags;
        request.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
        request.remappedBindingPoints = std::move(remappedBindingPoints);
        request.remappedAccessControls = std::move(remappedAccessControls);
        request.isRobustnessEnabled = device->IsRobustnessEnabled();
        request.disableWorkgroupInit = device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
        request.usesNumWorkgroups = entryPoint.usesNumWorkgroups;
        request.numWorkgroupsShaderRegister = layout->GetNumWorkgroupsShaderRegister();
        request.numWorkgroupsRegisterSpace = layout->GetNumWorkgroupsRegisterSpace();
        request.arrayLengthFromUniform = std::move(arrayLengthFromUniform);
        request.fxcVersion = compiler == Compiler::FXC ? GetD3DCompilerVersion() : 0;
        request.dxcVersion = compiler == Compiler::DXC ? dxcVersion : 0;
        request.deviceInfo = &device->GetDeviceInfo();
        request.hasShaderFloat16Feature = device->IsFeatureEnabled(Feature::ShaderFloat16);

        GetOverridableConstantsDefines(
            &request.defineStrings, &programmableStage.constants,
            &programmableStage.module->GetEntryPoint(programmableStage.entryPoint)
                 .overridableConstants);

        return std::move(request);
    }

    // TODO(dawn:1341): Move to use CacheKey instead of the vector.
    ResultOrError<std::vector<uint8_t>> CreateCacheKey() const {
        // Generate the WGSL from the Tint program so it's normalized.
        // TODO(tint:1180): Consider using a binary serialization of the tint AST for a more
        // compact representation.
        auto result = tint::writer::wgsl::Generate(program, tint::writer::wgsl::Options{});
        if (!result.success) {
            std::ostringstream errorStream;
            errorStream << "Tint WGSL failure:" << std::endl;
            errorStream << "Generator: " << result.error << std::endl;
            return DAWN_INTERNAL_ERROR(errorStream.str().c_str());
        }

        std::stringstream stream;

        // Prefix the key with the type to avoid collisions from another type that could
        // have the same key.
        stream << static_cast<uint32_t>(CacheKey::Type::Shader);
        stream << "\n";

        stream << result.wgsl.length();
        stream << "\n";

        stream << result.wgsl;
        stream << "\n";

        stream << "(ShaderCompilationRequest";
        stream << " compiler=" << compiler;
        stream << " entryPointName=" << entryPointName;
        stream << " stage=" << uint32_t(stage);
        stream << " compileFlags=" << compileFlags;
        stream << " disableSymbolRenaming=" << disableSymbolRenaming;

        stream << " remappedBindingPoints=";
        Serialize(stream, remappedBindingPoints);

        stream << " remappedAccessControls=";
        Serialize(stream, remappedAccessControls);

        stream << " useNumWorkgroups=" << usesNumWorkgroups;
        stream << " numWorkgroupsRegisterSpace=" << numWorkgroupsRegisterSpace;
        stream << " numWorkgroupsShaderRegister=" << numWorkgroupsShaderRegister;

        stream << " arrayLengthFromUniform=";
        Serialize(stream, arrayLengthFromUniform);

        stream << " shaderModel=" << deviceInfo->shaderModel;
        stream << " disableWorkgroupInit=" << disableWorkgroupInit;
        stream << " isRobustnessEnabled=" << isRobustnessEnabled;
        stream << " fxcVersion=" << fxcVersion;
        stream << " dxcVersion=" << dxcVersion;
        stream << " hasShaderFloat16Feature=" << hasShaderFloat16Feature;

        stream << " defines={";
        for (const auto& [name, value] : defineStrings) {
            stream << " <" << name << "," << value << ">";
        }
        stream << " }";

        stream << ")";
        stream << "\n";

        return std::vector<uint8_t>(std::istreambuf_iterator<char>{stream},
                                    std::istreambuf_iterator<char>{});
    }
};

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

ResultOrError<ComPtr<IDxcBlob>> CompileShaderDXC(IDxcLibrary* dxcLibrary,
                                                 IDxcCompiler* dxcCompiler,
                                                 const ShaderCompilationRequest& request,
                                                 const std::string& hlslSource) {
    ComPtr<IDxcBlobEncoding> sourceBlob;
    DAWN_TRY(CheckHRESULT(dxcLibrary->CreateBlobWithEncodingOnHeapCopy(
                              hlslSource.c_str(), hlslSource.length(), CP_UTF8, &sourceBlob),
                          "DXC create blob"));

    std::wstring entryPointW;
    DAWN_TRY_ASSIGN(entryPointW, ConvertStringToWstring(request.entryPointName));

    std::vector<const wchar_t*> arguments =
        GetDXCArguments(request.compileFlags, request.hasShaderFloat16Feature);

    // Build defines for overridable constants
    std::vector<std::pair<std::wstring, std::wstring>> defineStrings;
    defineStrings.reserve(request.defineStrings.size());
    for (const auto& [name, value] : request.defineStrings) {
        defineStrings.emplace_back(UTF8ToWStr(name.c_str()), UTF8ToWStr(value.c_str()));
    }

    std::vector<DxcDefine> dxcDefines;
    dxcDefines.reserve(defineStrings.size());
    for (const auto& [name, value] : defineStrings) {
        dxcDefines.push_back({name.c_str(), value.c_str()});
    }

    ComPtr<IDxcOperationResult> result;
    DAWN_TRY(
        CheckHRESULT(dxcCompiler->Compile(sourceBlob.Get(), nullptr, entryPointW.c_str(),
                                          request.deviceInfo->shaderProfiles[request.stage].c_str(),
                                          arguments.data(), arguments.size(), dxcDefines.data(),
                                          dxcDefines.size(), nullptr, &result),
                     "DXC compile"));

    HRESULT hr;
    DAWN_TRY(CheckHRESULT(result->GetStatus(&hr), "DXC get status"));

    if (FAILED(hr)) {
        ComPtr<IDxcBlobEncoding> errors;
        DAWN_TRY(CheckHRESULT(result->GetErrorBuffer(&errors), "DXC get error buffer"));

        return DAWN_FORMAT_VALIDATION_ERROR("DXC compile failed with: %s",
                                            static_cast<char*>(errors->GetBufferPointer()));
    }

    ComPtr<IDxcBlob> compiledShader;
    DAWN_TRY(CheckHRESULT(result->GetResult(&compiledShader), "DXC get result"));
    return std::move(compiledShader);
}

std::string CompileFlagsToStringFXC(uint32_t compileFlags) {
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

ResultOrError<ComPtr<ID3DBlob>> CompileShaderFXC(const PlatformFunctions* functions,
                                                 const ShaderCompilationRequest& request,
                                                 const std::string& hlslSource) {
    const char* targetProfile = nullptr;
    switch (request.stage) {
        case SingleShaderStage::Vertex:
            targetProfile = "vs_5_1";
            break;
        case SingleShaderStage::Fragment:
            targetProfile = "ps_5_1";
            break;
        case SingleShaderStage::Compute:
            targetProfile = "cs_5_1";
            break;
    }

    ComPtr<ID3DBlob> compiledShader;
    ComPtr<ID3DBlob> errors;

    // Build defines for overridable constants
    const D3D_SHADER_MACRO* pDefines = nullptr;
    std::vector<D3D_SHADER_MACRO> fxcDefines;
    if (request.defineStrings.size() > 0) {
        fxcDefines.reserve(request.defineStrings.size() + 1);
        for (const auto& [name, value] : request.defineStrings) {
            fxcDefines.push_back({name.c_str(), value.c_str()});
        }
        // d3dCompile D3D_SHADER_MACRO* pDefines is a nullptr terminated array
        fxcDefines.push_back({nullptr, nullptr});
        pDefines = fxcDefines.data();
    }

    DAWN_INVALID_IF(
        FAILED(functions->d3dCompile(hlslSource.c_str(), hlslSource.length(), nullptr, pDefines,
                                     nullptr, request.entryPointName, targetProfile,
                                     request.compileFlags, 0, &compiledShader, &errors)),
        "D3D compile failed with: %s", static_cast<char*>(errors->GetBufferPointer()));

    return std::move(compiledShader);
}

ResultOrError<std::string> TranslateToHLSL(dawn::platform::Platform* platform,
                                           const ShaderCompilationRequest& request,
                                           std::string* remappedEntryPointName) {
    std::ostringstream errorStream;
    errorStream << "Tint HLSL failure:" << std::endl;

    tint::transform::Manager transformManager;
    tint::transform::DataMap transformInputs;

    if (request.isRobustnessEnabled) {
        transformManager.Add<tint::transform::Robustness>();
    }

    transformManager.Add<tint::transform::BindingRemapper>();

    transformManager.Add<tint::transform::SingleEntryPoint>();
    transformInputs.Add<tint::transform::SingleEntryPoint::Config>(request.entryPointName);

    transformManager.Add<tint::transform::Renamer>();

    if (request.disableSymbolRenaming) {
        // We still need to rename HLSL reserved keywords
        transformInputs.Add<tint::transform::Renamer::Config>(
            tint::transform::Renamer::Target::kHlslKeywords);
    }

    // D3D12 registers like `t3` and `c3` have the same bindingOffset number in
    // the remapping but should not be considered a collision because they have
    // different types.
    const bool mayCollide = true;
    transformInputs.Add<tint::transform::BindingRemapper::Remappings>(
        std::move(request.remappedBindingPoints), std::move(request.remappedAccessControls),
        mayCollide);

    tint::Program transformedProgram;
    tint::transform::DataMap transformOutputs;
    {
        TRACE_EVENT0(platform, General, "RunTransforms");
        DAWN_TRY_ASSIGN(transformedProgram,
                        RunTransforms(&transformManager, request.program, transformInputs,
                                      &transformOutputs, nullptr));
    }

    if (auto* data = transformOutputs.Get<tint::transform::Renamer::Data>()) {
        auto it = data->remappings.find(request.entryPointName);
        if (it != data->remappings.end()) {
            *remappedEntryPointName = it->second;
        } else {
            DAWN_INVALID_IF(!request.disableSymbolRenaming,
                            "Could not find remapped name for entry point.");

            *remappedEntryPointName = request.entryPointName;
        }
    } else {
        return DAWN_FORMAT_VALIDATION_ERROR("Transform output missing renamer data.");
    }

    tint::writer::hlsl::Options options;
    options.disable_workgroup_init = request.disableWorkgroupInit;
    if (request.usesNumWorkgroups) {
        options.root_constant_binding_point.group = request.numWorkgroupsRegisterSpace;
        options.root_constant_binding_point.binding = request.numWorkgroupsShaderRegister;
    }
    // TODO(dawn:549): HLSL generation outputs the indices into the
    // array_length_from_uniform buffer that were actually used. When the blob cache can
    // store more than compiled shaders, we should reflect these used indices and store
    // them as well. This would allow us to only upload root constants that are actually
    // read by the shader.
    options.array_length_from_uniform = request.arrayLengthFromUniform;
    TRACE_EVENT0(platform, General, "tint::writer::hlsl::Generate");
    auto result = tint::writer::hlsl::Generate(&transformedProgram, options);
    DAWN_INVALID_IF(!result.success, "An error occured while generating HLSL: %s", result.error);

    return std::move(result.hlsl);
}

template <typename F>
MaybeError CompileShader(dawn::platform::Platform* platform,
                         const PlatformFunctions* functions,
                         IDxcLibrary* dxcLibrary,
                         IDxcCompiler* dxcCompiler,
                         ShaderCompilationRequest&& request,
                         bool dumpShaders,
                         F&& DumpShadersEmitLog,
                         CompiledShader* compiledShader) {
    // Compile the source shader to HLSL.
    std::string hlslSource;
    std::string remappedEntryPoint;
    DAWN_TRY_ASSIGN(hlslSource, TranslateToHLSL(platform, request, &remappedEntryPoint));
    if (dumpShaders) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated HLSL */" << std::endl << hlslSource;
        DumpShadersEmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }
    request.entryPointName = remappedEntryPoint.c_str();
    switch (request.compiler) {
        case ShaderCompilationRequest::Compiler::DXC: {
            TRACE_EVENT0(platform, General, "CompileShaderDXC");
            DAWN_TRY_ASSIGN(compiledShader->compiledDXCShader,
                            CompileShaderDXC(dxcLibrary, dxcCompiler, request, hlslSource));
            break;
        }
        case ShaderCompilationRequest::Compiler::FXC: {
            TRACE_EVENT0(platform, General, "CompileShaderFXC");
            DAWN_TRY_ASSIGN(compiledShader->compiledFXCShader,
                            CompileShaderFXC(functions, request, hlslSource));
            break;
        }
    }

    if (dumpShaders && request.compiler == ShaderCompilationRequest::Compiler::FXC) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* FXC compile flags */ " << std::endl
                  << CompileFlagsToStringFXC(request.compileFlags) << std::endl;

        dumpedMsg << "/* Dumped disassembled DXBC */" << std::endl;

        ComPtr<ID3DBlob> disassembly;
        if (FAILED(functions->d3dDisassemble(compiledShader->compiledFXCShader->GetBufferPointer(),
                                             compiledShader->compiledFXCShader->GetBufferSize(), 0,
                                             nullptr, &disassembly))) {
            dumpedMsg << "D3D disassemble failed" << std::endl;
        } else {
            dumpedMsg << reinterpret_cast<const char*>(disassembly->GetBufferPointer());
        }
        DumpShadersEmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    return {};
}

}  // anonymous namespace

// static
ResultOrError<Ref<ShaderModule>> ShaderModule::Create(
    Device* device,
    const ShaderModuleDescriptor* descriptor,
    ShaderModuleParseResult* parseResult,
    OwnedCompilationMessages* compilationMessages) {
    Ref<ShaderModule> module = AcquireRef(new ShaderModule(device, descriptor));
    DAWN_TRY(module->Initialize(parseResult, compilationMessages));
    return module;
}

ShaderModule::ShaderModule(Device* device, const ShaderModuleDescriptor* descriptor)
    : ShaderModuleBase(device, descriptor) {}

MaybeError ShaderModule::Initialize(ShaderModuleParseResult* parseResult,
                                    OwnedCompilationMessages* compilationMessages) {
    ScopedTintICEHandler scopedICEHandler(GetDevice());
    return InitializeBase(parseResult, compilationMessages);
}

ResultOrError<CompiledShader> ShaderModule::Compile(const ProgrammableStage& programmableStage,
                                                    SingleShaderStage stage,
                                                    const PipelineLayout* layout,
                                                    uint32_t compileFlags) {
    TRACE_EVENT0(GetDevice()->GetPlatform(), General, "ShaderModuleD3D12::Compile");
    ASSERT(!IsError());

    ScopedTintICEHandler scopedICEHandler(GetDevice());

    Device* device = ToBackend(GetDevice());

    CompiledShader compiledShader = {};

    tint::transform::Manager transformManager;
    tint::transform::DataMap transformInputs;

    const tint::Program* program = GetTintProgram();
    tint::Program programAsValue;

    AddExternalTextureTransform(layout, &transformManager, &transformInputs);

    if (stage == SingleShaderStage::Vertex) {
        transformManager.Add<tint::transform::FirstIndexOffset>();
        transformInputs.Add<tint::transform::FirstIndexOffset::BindingPoint>(
            layout->GetFirstIndexOffsetShaderRegister(),
            layout->GetFirstIndexOffsetRegisterSpace());
    }

    tint::transform::DataMap transformOutputs;
    DAWN_TRY_ASSIGN(programAsValue, RunTransforms(&transformManager, program, transformInputs,
                                                  &transformOutputs, nullptr));
    program = &programAsValue;

    if (stage == SingleShaderStage::Vertex) {
        if (auto* data = transformOutputs.Get<tint::transform::FirstIndexOffset::Data>()) {
            // TODO(dawn:549): Consider adding this information to the pipeline cache once we
            // can store more than the shader blob in it.
            compiledShader.usesVertexOrInstanceIndex = data->has_vertex_or_instance_index;
        }
    }

    ShaderCompilationRequest request;
    DAWN_TRY_ASSIGN(request,
                    ShaderCompilationRequest::Create(
                        programmableStage.entryPoint.c_str(), stage, layout, compileFlags, device,
                        program, GetEntryPoint(programmableStage.entryPoint), programmableStage));

    // TODO(dawn:1341): Add shader cache key generation and caching for the compiled shader.
    DAWN_TRY(CompileShader(
        device->GetPlatform(), device->GetFunctions(),
        device->IsToggleEnabled(Toggle::UseDXC) ? device->GetDxcLibrary().Get() : nullptr,
        device->IsToggleEnabled(Toggle::UseDXC) ? device->GetDxcCompiler().Get() : nullptr,
        std::move(request), device->IsToggleEnabled(Toggle::DumpShaders),
        [&](WGPULoggingType loggingType, const char* message) {
            GetDevice()->EmitLog(loggingType, message);
        },
        &compiledShader));
    return std::move(compiledShader);
}

D3D12_SHADER_BYTECODE CompiledShader::GetD3D12ShaderBytecode() const {
    if (compiledFXCShader != nullptr) {
        return {compiledFXCShader->GetBufferPointer(), compiledFXCShader->GetBufferSize()};
    } else if (compiledDXCShader != nullptr) {
        return {compiledDXCShader->GetBufferPointer(), compiledDXCShader->GetBufferSize()};
    }
    UNREACHABLE();
    return {};
}
}  // namespace dawn::native::d3d12
