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
#include "dawn/native/CacheRequest.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/BlobD3D12.h"
#include "dawn/native/d3d12/D3D12Error.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctions.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/native/stream/BlobSource.h"
#include "dawn/native/stream/ByteVectorSink.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::stream {

// Define no-op serializations for pD3DCompile, IDxcLibrary, and IDxcCompiler.
// These are output-only interfaces used to generate bytecode.
template <>
void Stream<IDxcLibrary*>::Write(Sink*, IDxcLibrary* const&) {}
template <>
void Stream<IDxcCompiler*>::Write(Sink*, IDxcCompiler* const&) {}
template <>
void Stream<pD3DCompile>::Write(Sink*, pD3DCompile const&) {}

}  // namespace dawn::native::stream

namespace dawn::native::d3d12 {

namespace {

enum class Compiler { FXC, DXC };

#define HLSL_COMPILATION_REQUEST_MEMBERS(X)                                                 \
    X(const tint::Program*, inputProgram)                                                   \
    X(std::string_view, entryPointName)                                                     \
    X(SingleShaderStage, stage)                                                             \
    X(uint32_t, shaderModel)                                                                \
    X(uint32_t, compileFlags)                                                               \
    X(Compiler, compiler)                                                                   \
    X(uint64_t, compilerVersion)                                                            \
    X(std::wstring_view, dxcShaderProfile)                                                  \
    X(std::string_view, fxcShaderProfile)                                                   \
    X(pD3DCompile, d3dCompile)                                                              \
    X(IDxcLibrary*, dxcLibrary)                                                             \
    X(IDxcCompiler*, dxcCompiler)                                                           \
    X(uint32_t, firstIndexOffsetShaderRegister)                                             \
    X(uint32_t, firstIndexOffsetRegisterSpace)                                              \
    X(bool, usesNumWorkgroups)                                                              \
    X(uint32_t, numWorkgroupsShaderRegister)                                                \
    X(uint32_t, numWorkgroupsRegisterSpace)                                                 \
    X(tint::transform::MultiplanarExternalTexture::BindingsMap, newBindingsMap)             \
    X(tint::writer::ArrayLengthFromUniformOptions, arrayLengthFromUniform)                  \
    X(tint::transform::BindingRemapper::BindingPoints, remappedBindingPoints)               \
    X(tint::transform::BindingRemapper::AccessControls, remappedAccessControls)             \
    X(std::optional<tint::transform::SubstituteOverride::Config>, substituteOverrideConfig) \
    X(std::bitset<kMaxInterStageShaderVariables>, interstageLocations)                      \
    X(LimitsForCompilationRequest, limits)                                                  \
    X(bool, disableSymbolRenaming)                                                          \
    X(bool, isRobustnessEnabled)                                                            \
    X(bool, disableWorkgroupInit)                                                           \
    X(bool, dumpShaders)

#define D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS(X) \
    X(bool, hasShaderF16Feature)                    \
    X(uint32_t, compileFlags)                       \
    X(Compiler, compiler)                           \
    X(uint64_t, compilerVersion)                    \
    X(std::wstring_view, dxcShaderProfile)          \
    X(std::string_view, fxcShaderProfile)           \
    X(pD3DCompile, d3dCompile)                      \
    X(IDxcLibrary*, dxcLibrary)                     \
    X(IDxcCompiler*, dxcCompiler)

DAWN_SERIALIZABLE(struct, HlslCompilationRequest, HLSL_COMPILATION_REQUEST_MEMBERS){};
#undef HLSL_COMPILATION_REQUEST_MEMBERS

DAWN_SERIALIZABLE(struct,
                  D3DBytecodeCompilationRequest,
                  D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS){};
#undef D3D_BYTECODE_COMPILATION_REQUEST_MEMBERS

#define D3D_COMPILATION_REQUEST_MEMBERS(X)     \
    X(HlslCompilationRequest, hlsl)            \
    X(D3DBytecodeCompilationRequest, bytecode) \
    X(CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*>, tracePlatform)

DAWN_MAKE_CACHE_REQUEST(D3DCompilationRequest, D3D_COMPILATION_REQUEST_MEMBERS);
#undef D3D_COMPILATION_REQUEST_MEMBERS

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

ResultOrError<ComPtr<IDxcBlob>> CompileShaderDXC(const D3DBytecodeCompilationRequest& r,
                                                 const std::string& entryPointName,
                                                 const std::string& hlslSource) {
    ComPtr<IDxcBlobEncoding> sourceBlob;
    DAWN_TRY(CheckHRESULT(r.dxcLibrary->CreateBlobWithEncodingFromPinned(
                              hlslSource.c_str(), hlslSource.length(), CP_UTF8, &sourceBlob),
                          "DXC create blob"));

    std::wstring entryPointW;
    DAWN_TRY_ASSIGN(entryPointW, ConvertStringToWstring(entryPointName));

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

ResultOrError<ComPtr<ID3DBlob>> CompileShaderFXC(const D3DBytecodeCompilationRequest& r,
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

ResultOrError<std::string> TranslateToHLSL(
    HlslCompilationRequest r,
    CacheKey::UnsafeUnkeyedValue<dawn::platform::Platform*> tracePlatform,
    std::string* remappedEntryPointName,
    bool* usesVertexOrInstanceIndex) {
    std::ostringstream errorStream;
    errorStream << "Tint HLSL failure:" << std::endl;

    tint::transform::Manager transformManager;
    tint::transform::DataMap transformInputs;

    // Run before the renamer so that the entry point name matches `entryPointName` still.
    transformManager.Add<tint::transform::SingleEntryPoint>();
    transformInputs.Add<tint::transform::SingleEntryPoint::Config>(r.entryPointName.data());

    // Needs to run before all other transforms so that they can use builtin names safely.
    transformManager.Add<tint::transform::Renamer>();
    if (r.disableSymbolRenaming) {
        // We still need to rename HLSL reserved keywords
        transformInputs.Add<tint::transform::Renamer::Config>(
            tint::transform::Renamer::Target::kHlslKeywords);
    }

    if (!r.newBindingsMap.empty()) {
        transformManager.Add<tint::transform::MultiplanarExternalTexture>();
        transformInputs.Add<tint::transform::MultiplanarExternalTexture::NewBindingPoints>(
            std::move(r.newBindingsMap));
    }

    if (r.stage == SingleShaderStage::Vertex) {
        transformManager.Add<tint::transform::FirstIndexOffset>();
        transformInputs.Add<tint::transform::FirstIndexOffset::BindingPoint>(
            r.firstIndexOffsetShaderRegister, r.firstIndexOffsetRegisterSpace);
    }

    if (r.substituteOverrideConfig) {
        // This needs to run after SingleEntryPoint transform which removes unused overrides for
        // current entry point.
        transformManager.Add<tint::transform::SubstituteOverride>();
        transformInputs.Add<tint::transform::SubstituteOverride::Config>(
            std::move(r.substituteOverrideConfig).value());
    }

    if (r.isRobustnessEnabled) {
        transformManager.Add<tint::transform::Robustness>();
    }

    transformManager.Add<tint::transform::BindingRemapper>();

    // D3D12 registers like `t3` and `c3` have the same bindingOffset number in
    // the remapping but should not be considered a collision because they have
    // different types.
    const bool mayCollide = true;
    transformInputs.Add<tint::transform::BindingRemapper::Remappings>(
        std::move(r.remappedBindingPoints), std::move(r.remappedAccessControls), mayCollide);

    tint::Program transformedProgram;
    tint::transform::DataMap transformOutputs;
    {
        TRACE_EVENT0(tracePlatform.UnsafeGetValue(), General, "RunTransforms");
        DAWN_TRY_ASSIGN(transformedProgram,
                        RunTransforms(&transformManager, r.inputProgram, transformInputs,
                                      &transformOutputs, nullptr));
    }

    if (auto* data = transformOutputs.Get<tint::transform::Renamer::Data>()) {
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

    if (r.stage == SingleShaderStage::Vertex) {
        if (auto* data = transformOutputs.Get<tint::transform::FirstIndexOffset::Data>()) {
            *usesVertexOrInstanceIndex = data->has_vertex_or_instance_index;
        } else {
            return DAWN_VALIDATION_ERROR("Transform output missing first index offset data.");
        }
    }

    tint::writer::hlsl::Options options;
    options.disable_workgroup_init = r.disableWorkgroupInit;
    if (r.usesNumWorkgroups) {
        options.root_constant_binding_point =
            tint::sem::BindingPoint{r.numWorkgroupsRegisterSpace, r.numWorkgroupsShaderRegister};
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
    }

    TRACE_EVENT0(tracePlatform.UnsafeGetValue(), General, "tint::writer::hlsl::Generate");
    auto result = tint::writer::hlsl::Generate(&transformedProgram, options);
    DAWN_INVALID_IF(!result.success, "An error occured while generating HLSL: %s", result.error);

    return std::move(result.hlsl);
}

ResultOrError<CompiledShader> CompileShader(D3DCompilationRequest r) {
    CompiledShader compiledShader;
    // Compile the source shader to HLSL.
    std::string remappedEntryPoint;
    DAWN_TRY_ASSIGN(compiledShader.hlslSource,
                    TranslateToHLSL(std::move(r.hlsl), r.tracePlatform, &remappedEntryPoint,
                                    &compiledShader.usesVertexOrInstanceIndex));

    switch (r.bytecode.compiler) {
        case Compiler::DXC: {
            TRACE_EVENT0(r.tracePlatform.UnsafeGetValue(), General, "CompileShaderDXC");
            ComPtr<IDxcBlob> compiledDXCShader;
            DAWN_TRY_ASSIGN(compiledDXCShader, CompileShaderDXC(r.bytecode, remappedEntryPoint,
                                                                compiledShader.hlslSource));
            compiledShader.shaderBlob = CreateBlob(std::move(compiledDXCShader));
            break;
        }
        case Compiler::FXC: {
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
    if (!r.hlsl.dumpShaders) {
        compiledShader.hlslSource = "";
    }
    return compiledShader;
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

ResultOrError<CompiledShader> ShaderModule::Compile(
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    const PipelineLayout* layout,
    uint32_t compileFlags,
    const std::bitset<kMaxInterStageShaderVariables>* usedInterstageVariables) {
    Device* device = ToBackend(GetDevice());
    TRACE_EVENT0(device->GetPlatform(), General, "ShaderModuleD3D12::Compile");
    ASSERT(!IsError());

    ScopedTintICEHandler scopedICEHandler(device);
    const EntryPointMetadata& entryPoint = GetEntryPoint(programmableStage.entryPoint);

    D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnkeyedValue(device->GetPlatform());
    req.hlsl.shaderModel = device->GetDeviceInfo().shaderModel;
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.isRobustnessEnabled = device->IsRobustnessEnabled();
    req.hlsl.disableWorkgroupInit = device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);

    if (usedInterstageVariables) {
        req.hlsl.interstageLocations = *usedInterstageVariables;
    }

    req.bytecode.hasShaderF16Feature = device->HasFeature(Feature::ShaderF16);
    req.bytecode.compileFlags = compileFlags;

    if (device->IsToggleEnabled(Toggle::UseDXC)) {
        // If UseDXC toggle are not forced to be disable, DXC should have been validated to be
        // available.
        ASSERT(ToBackend(device->GetAdapter())->GetBackend()->IsDXCAvailable());
        // We can get the DXC version information since IsDXCAvailable() is true.
        DxcVersionInfo dxcVersionInfo =
            ToBackend(device->GetAdapter())->GetBackend()->GetDxcVersion();

        req.bytecode.compiler = Compiler::DXC;
        req.bytecode.dxcLibrary = device->GetDxcLibrary().Get();
        req.bytecode.dxcCompiler = device->GetDxcCompiler().Get();
        req.bytecode.compilerVersion = dxcVersionInfo.DxcCompilerVersion;
        req.bytecode.dxcShaderProfile = device->GetDeviceInfo().shaderProfiles[stage];
    } else {
        req.bytecode.compiler = Compiler::FXC;
        req.bytecode.d3dCompile = device->GetFunctions()->d3dCompile;
        req.bytecode.compilerVersion = D3D_COMPILER_VERSION;
        switch (stage) {
            case SingleShaderStage::Vertex:
                req.bytecode.fxcShaderProfile = "vs_5_1";
                break;
            case SingleShaderStage::Fragment:
                req.bytecode.fxcShaderProfile = "ps_5_1";
                break;
            case SingleShaderStage::Compute:
                req.bytecode.fxcShaderProfile = "cs_5_1";
                break;
        }
    }

    using tint::transform::BindingPoint;
    using tint::transform::BindingRemapper;

    BindingRemapper::BindingPoints remappedBindingPoints;
    BindingRemapper::AccessControls remappedAccessControls;

    tint::writer::ArrayLengthFromUniformOptions arrayLengthFromUniform;
    arrayLengthFromUniform.ubo_binding = {layout->GetDynamicStorageBufferLengthsRegisterSpace(),
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
                  bgl->GetBindingInfo(bindingIndex).buffer.type == kInternalStorageBufferBinding));
            if (forceStorageBufferAsUAV) {
                remappedAccessControls.emplace(srcBindingPoint, tint::builtin::Access::kReadWrite);
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

    std::optional<tint::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    req.hlsl.inputProgram = GetTintProgram();
    req.hlsl.entryPointName = programmableStage.entryPoint.c_str();
    req.hlsl.stage = stage;
    req.hlsl.firstIndexOffsetShaderRegister = layout->GetFirstIndexOffsetShaderRegister();
    req.hlsl.firstIndexOffsetRegisterSpace = layout->GetFirstIndexOffsetRegisterSpace();
    req.hlsl.usesNumWorkgroups = entryPoint.usesNumWorkgroups;
    req.hlsl.numWorkgroupsShaderRegister = layout->GetNumWorkgroupsShaderRegister();
    req.hlsl.numWorkgroupsRegisterSpace = layout->GetNumWorkgroupsRegisterSpace();
    req.hlsl.remappedBindingPoints = std::move(remappedBindingPoints);
    req.hlsl.remappedAccessControls = std::move(remappedAccessControls);
    req.hlsl.newBindingsMap = BuildExternalTextureTransformBindings(layout);
    req.hlsl.arrayLengthFromUniform = std::move(arrayLengthFromUniform);
    req.hlsl.substituteOverrideConfig = std::move(substituteOverrideConfig);

    const CombinedLimits& limits = device->GetLimits();
    req.hlsl.limits = LimitsForCompilationRequest::Create(limits.v1);

    CacheResult<CompiledShader> compiledShader;
    DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req), CompiledShader::FromBlob,
                         CompileShader);

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated HLSL */" << std::endl
                  << compiledShader->hlslSource << std::endl;

        if (device->IsToggleEnabled(Toggle::UseDXC)) {
            dumpedMsg << "/* Dumped disassembled DXIL */" << std::endl;
            D3D12_SHADER_BYTECODE code = compiledShader->GetD3D12ShaderBytecode();
            ComPtr<IDxcBlobEncoding> dxcBlob;
            ComPtr<IDxcBlobEncoding> disassembly;
            if (FAILED(device->GetDxcLibrary()->CreateBlobWithEncodingFromPinned(
                    code.pShaderBytecode, code.BytecodeLength, 0, &dxcBlob)) ||
                FAILED(device->GetDxcCompiler()->Disassemble(dxcBlob.Get(), &disassembly))) {
                dumpedMsg << "DXC disassemble failed" << std::endl;
            } else {
                dumpedMsg << std::string_view(
                    static_cast<const char*>(disassembly->GetBufferPointer()),
                    disassembly->GetBufferSize());
            }
        } else {
            dumpedMsg << "/* FXC compile flags */ " << std::endl
                      << CompileFlagsToStringFXC(compileFlags) << std::endl;
            dumpedMsg << "/* Dumped disassembled DXBC */" << std::endl;
            ComPtr<ID3DBlob> disassembly;
            D3D12_SHADER_BYTECODE code = compiledShader->GetD3D12ShaderBytecode();
            UINT flags =
                // Some literals are printed as floats with precision(6) which is not enough
                // precision for values very close to 0, so always print literals as hex values.
                D3D_DISASM_PRINT_HEX_LITERALS;
            if (FAILED(device->GetFunctions()->d3dDisassemble(
                    code.pShaderBytecode, code.BytecodeLength, flags, nullptr, &disassembly))) {
                dumpedMsg << "D3D disassemble failed" << std::endl;
            } else {
                dumpedMsg << std::string_view(
                    static_cast<const char*>(disassembly->GetBufferPointer()),
                    disassembly->GetBufferSize());
            }
        }
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    CompiledShader result = compiledShader.Acquire();
    result.hlslSource = "";
    return result;
}

D3D12_SHADER_BYTECODE CompiledShader::GetD3D12ShaderBytecode() const {
    return {shaderBlob.Data(), shaderBlob.Size()};
}

}  // namespace dawn::native::d3d12
