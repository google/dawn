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

#include "dawn/native/d3d11/ShaderModuleD3D11.h"

#include <string>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Log.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d11/AdapterD3D11.h"
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/BindGroupLayoutD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d11 {

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
    TRACE_EVENT0(device->GetPlatform(), General, "ShaderModuleD3D11::Compile");
    ASSERT(!IsError());

    ScopedTintICEHandler scopedICEHandler(device);
    const EntryPointMetadata& entryPoint = GetEntryPoint(programmableStage.entryPoint);

    d3d::D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnkeyedValue(device->GetPlatform());
    req.hlsl.shaderModel = 50;
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.isRobustnessEnabled = device->IsRobustnessEnabled();
    req.hlsl.disableWorkgroupInit = device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);

    if (usedInterstageVariables) {
        req.hlsl.interstageLocations = *usedInterstageVariables;
    }

    req.bytecode.hasShaderF16Feature = false;
    req.bytecode.compileFlags = compileFlags;

    // D3D11 only supports FXC.
    req.bytecode.compiler = d3d::Compiler::FXC;
    req.bytecode.d3dCompile = device->GetFunctions()->d3dCompile;
    req.bytecode.compilerVersion = D3D_COMPILER_VERSION;
    ASSERT(device->GetDeviceInfo().shaderModel == 50);
    switch (stage) {
        case SingleShaderStage::Vertex:
            req.bytecode.fxcShaderProfile = "vs_5_0";
            break;
        case SingleShaderStage::Fragment:
            req.bytecode.fxcShaderProfile = "ps_5_0";
            break;
        case SingleShaderStage::Compute:
            req.bytecode.fxcShaderProfile = "cs_5_0";
            break;
    }

    tint::writer::BindingRemapperOptions bindingRemapper;
    // D3D11 registers like `t3` and `c3` have the same bindingOffset number in
    // the remapping but should not be considered a collision because they have
    // different types.
    bindingRemapper.allow_collisions = true;

    const BindingInfoArray& moduleBindingInfo = entryPoint.bindings;

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* groupLayout = ToBackend(layout->GetBindGroupLayout(group));
        const auto& indices = layout->GetBindingIndexInfo()[group];
        const auto& groupBindingInfo = moduleBindingInfo[group];

        for (const auto& [binding, bindingInfo] : groupBindingInfo) {
            BindingIndex bindingIndex = groupLayout->GetBindingIndex(binding);
            tint::writer::BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                                       static_cast<uint32_t>(binding)};
            tint::writer::BindingPoint dstBindingPoint{0u, indices[bindingIndex]};
            if (srcBindingPoint != dstBindingPoint) {
                bindingRemapper.binding_points.emplace(srcBindingPoint, dstBindingPoint);
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
    // D3D11 (HLSL SM5.0) doesn't support spaces, so we have to put the firstIndex in the default
    // space(0)
    req.hlsl.firstIndexOffsetRegisterSpace = 0;
    req.hlsl.firstIndexOffsetShaderRegister = PipelineLayout::kNumWorkgroupsConstantBufferSlot;

    req.hlsl.usesNumWorkgroups = entryPoint.usesNumWorkgroups;
    // D3D11 (HLSL SM5.0) doesn't support spaces, so we have to put the numWorkgroups in the default
    // space(0)
    req.hlsl.numWorkgroupsRegisterSpace = 0;
    req.hlsl.numWorkgroupsShaderRegister = PipelineLayout::kNumWorkgroupsConstantBufferSlot;

    req.hlsl.bindingRemapper = std::move(bindingRemapper);

    req.hlsl.externalTextureOptions = BuildExternalTextureTransformBindings(layout);
    req.hlsl.substituteOverrideConfig = std::move(substituteOverrideConfig);

    // TODO(dawn:1705): do we need to support it?
    req.hlsl.polyfillReflectVec2F32 = false;

    const CombinedLimits& limits = device->GetLimits();
    req.hlsl.limits = LimitsForCompilationRequest::Create(limits.v1);

    CacheResult<d3d::CompiledShader> compiledShader;
    DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req), d3d::CompiledShader::FromBlob,
                         d3d::CompileShader);

    if (req.hlsl.dumpShaders) {
        d3d::DumpCompiledShader(device, *compiledShader, compileFlags);
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    d3d::CompiledShader result = compiledShader.Acquire();
    result.hlslSource = std::string();

    return result;
}

}  // namespace dawn::native::d3d11
