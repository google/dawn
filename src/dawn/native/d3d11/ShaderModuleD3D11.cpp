// Copyright 2023 The Dawn & Tint Authors
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
#include "dawn/native/d3d11/BackendD3D11.h"
#include "dawn/native/d3d11/BindGroupLayoutD3D11.h"
#include "dawn/native/d3d11/DeviceD3D11.h"
#include "dawn/native/d3d11/PhysicalDeviceD3D11.h"
#include "dawn/native/d3d11/PipelineLayoutD3D11.h"
#include "dawn/native/d3d11/PlatformFunctionsD3D11.h"
#include "dawn/native/d3d11/UtilsD3D11.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
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
    const std::optional<dawn::native::d3d::InterStageShaderVariablesMask>&
        usedInterstageVariables) {
    Device* device = ToBackend(GetDevice());
    TRACE_EVENT0(device->GetPlatform(), General, "ShaderModuleD3D11::Compile");
    DAWN_ASSERT(!IsError());

    ScopedTintICEHandler scopedICEHandler(device);
    const EntryPointMetadata& entryPoint = GetEntryPoint(programmableStage.entryPoint);

    d3d::D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnkeyedValue(device->GetPlatform());
    req.hlsl.shaderModel = 50;
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.isRobustnessEnabled = device->IsRobustnessEnabled();
    req.hlsl.disableWorkgroupInit = device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);

    if (usedInterstageVariables.has_value()) {
        req.hlsl.interstageLocations = *usedInterstageVariables;
    }

    req.bytecode.hasShaderF16Feature = false;
    req.bytecode.compileFlags = compileFlags;

    // D3D11 only supports FXC.
    req.bytecode.compiler = d3d::Compiler::FXC;
    req.bytecode.d3dCompile = device->GetFunctions()->d3dCompile;
    req.bytecode.compilerVersion = D3D_COMPILER_VERSION;
    DAWN_ASSERT(device->GetDeviceInfo().shaderModel == 50);
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

    tint::BindingRemapperOptions bindingRemapper;

    const BindingInfoArray& moduleBindingInfo = entryPoint.bindings;

    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* groupLayout = ToBackend(layout->GetBindGroupLayout(group));
        const auto& indices = layout->GetBindingIndexInfo()[group];
        const auto& groupBindingInfo = moduleBindingInfo[group];

        for (const auto& [binding, bindingInfo] : groupBindingInfo) {
            BindingIndex bindingIndex = groupLayout->GetBindingIndex(binding);
            tint::BindingPoint srcBindingPoint{static_cast<uint32_t>(group),
                                               static_cast<uint32_t>(binding)};
            tint::BindingPoint dstBindingPoint{0u, indices[bindingIndex]};
            if (srcBindingPoint != dstBindingPoint) {
                bindingRemapper.binding_points.emplace(srcBindingPoint, dstBindingPoint);
            }
        }

        // Tint will add two bindings (plane1, params) for one external texture binding.
        // We need to remap the binding points for the two bindings.
        // we cannot specified the final slot of those two bindings in
        // req.hlsl.externalTextureOptions because the final slots may be conflict with
        // existing other bindings, and then they will be remapped again with bindingRemapper
        // incorrectly. So we have to use intermediate binding slots in
        // req.hlsl.externalTextureOptions, and then map them to the final slots with
        // bindingRemapper.
        for (const auto& [_, expansion] : groupLayout->GetExternalTextureBindingExpansionMap()) {
            uint32_t plane1Slot = indices[groupLayout->GetBindingIndex(expansion.plane1)];
            uint32_t paramsSlot = indices[groupLayout->GetBindingIndex(expansion.params)];
            bindingRemapper.binding_points.emplace(
                tint::BindingPoint{static_cast<uint32_t>(group),
                                   static_cast<uint32_t>(expansion.plane1)},
                tint::BindingPoint{0u, plane1Slot});
            bindingRemapper.binding_points.emplace(
                tint::BindingPoint{static_cast<uint32_t>(group),
                                   static_cast<uint32_t>(expansion.params)},
                tint::BindingPoint{0u, paramsSlot});
        }
    }

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
    if (!programmableStage.metadata->overrides.empty()) {
        substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    }

    req.hlsl.inputProgram = GetTintProgram();
    req.hlsl.entryPointName = programmableStage.entryPoint.c_str();
    req.hlsl.stage = stage;
    // Put the firstIndex into the internally reserved group and binding to avoid conflicting with
    // any existing bindings.
    req.hlsl.firstIndexOffsetRegisterSpace = PipelineLayout::kReservedConstantsBindGroupIndex;
    req.hlsl.firstIndexOffsetShaderRegister = PipelineLayout::kFirstIndexOffsetBindingNumber;
    // Remap to the desired space and binding, [0, kFirstIndexOffsetConstantBufferSlot].
    {
        tint::BindingPoint srcBindingPoint{req.hlsl.firstIndexOffsetRegisterSpace,
                                           req.hlsl.firstIndexOffsetShaderRegister};
        // D3D11 (HLSL SM5.0) doesn't support spaces, so we have to put the firstIndex in the
        // default space(0)
        tint::BindingPoint dstBindingPoint{0u, PipelineLayout::kFirstIndexOffsetConstantBufferSlot};
        bindingRemapper.binding_points.emplace(srcBindingPoint, dstBindingPoint);
    }

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
    MaybeError compileError = [&]() -> MaybeError {
        DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req), d3d::CompiledShader::FromBlob,
                             d3d::CompileShader, "D3D11.CompileShader");
        return {};
    }();

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        d3d::DumpFXCCompiledShader(device, *compiledShader, compileFlags);
    }

    if (compileError.IsError()) {
        return {compileError.AcquireError()};
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    d3d::CompiledShader result = compiledShader.Acquire();
    result.hlslSource = std::string();

    return result;
}

}  // namespace dawn::native::d3d11
