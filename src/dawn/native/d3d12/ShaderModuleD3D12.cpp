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

#include <string>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Log.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/AdapterD3D12.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d12 {

namespace {
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

ResultOrError<d3d::CompiledShader> ShaderModule::Compile(
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

    d3d::D3DCompilationRequest req = {};
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
        d3d::DxcVersionInfo dxcVersionInfo =
            ToBackend(device->GetAdapter())->GetBackend()->GetDxcVersion();

        req.bytecode.compiler = d3d::Compiler::DXC;
        req.bytecode.dxcLibrary = device->GetDxcLibrary().Get();
        req.bytecode.dxcCompiler = device->GetDxcCompiler().Get();
        req.bytecode.compilerVersion = dxcVersionInfo.DxcCompilerVersion;
        req.bytecode.dxcShaderProfile = device->GetDeviceInfo().shaderProfiles[stage];
    } else {
        req.bytecode.compiler = d3d::Compiler::FXC;
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

    using tint::writer::BindingPoint;

    tint::writer::BindingRemapperOptions bindingRemapper;
    // D3D12 registers like `t3` and `c3` have the same bindingOffset number in
    // the remapping but should not be considered a collision because they have
    // different types.
    bindingRemapper.allow_collisions = true;

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
                bindingRemapper.binding_points.emplace(srcBindingPoint, dstBindingPoint);
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
                bindingRemapper.access_controls.emplace(srcBindingPoint,
                                                        tint::builtin::Access::kReadWrite);
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
                auto it = bindingRemapper.binding_points.find(bindingPoint);
                if (it != bindingRemapper.binding_points.end()) {
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
    req.hlsl.bindingRemapper = std::move(bindingRemapper);
    req.hlsl.externalTextureOptions = BuildExternalTextureTransformBindings(layout);
    req.hlsl.arrayLengthFromUniform = std::move(arrayLengthFromUniform);
    req.hlsl.substituteOverrideConfig = std::move(substituteOverrideConfig);

    req.hlsl.polyfillReflectVec2F32 = device->IsToggleEnabled(Toggle::D3D12PolyfillReflectVec2F32);

    const CombinedLimits& limits = device->GetLimits();
    req.hlsl.limits = LimitsForCompilationRequest::Create(limits.v1);

    CacheResult<d3d::CompiledShader> compiledShader;
    DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req), d3d::CompiledShader::FromBlob,
                         d3d::CompileShader);

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        std::ostringstream dumpedMsg;
        dumpedMsg << "/* Dumped generated HLSL */" << std::endl
                  << compiledShader->hlslSource << std::endl;

        if (device->IsToggleEnabled(Toggle::UseDXC)) {
            dumpedMsg << "/* Dumped disassembled DXIL */" << std::endl;
            const Blob& shaderBlob = compiledShader->shaderBlob;
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
                      << CompileFlagsToStringFXC(compileFlags) << std::endl;
            dumpedMsg << "/* Dumped disassembled DXBC */" << std::endl;
            ComPtr<ID3DBlob> disassembly;
            const Blob& shaderBlob = compiledShader->shaderBlob;
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
        device->EmitLog(WGPULoggingType_Info, dumpedMsg.str().c_str());
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    d3d::CompiledShader result = compiledShader.Acquire();
    result.hlslSource = "";
    return result;
}

}  // namespace dawn::native::d3d12
