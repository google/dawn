// Copyright 2017 The Dawn & Tint Authors
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

#include "dawn/native/d3d12/ShaderModuleD3D12.h"

#include <string>
#include <unordered_map>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/BitSetIterator.h"
#include "dawn/common/Log.h"
#include "dawn/native/Pipeline.h"
#include "dawn/native/TintUtils.h"
#include "dawn/native/d3d/D3DCompilationRequest.h"
#include "dawn/native/d3d/D3DError.h"
#include "dawn/native/d3d12/BackendD3D12.h"
#include "dawn/native/d3d12/BindGroupLayoutD3D12.h"
#include "dawn/native/d3d12/DeviceD3D12.h"
#include "dawn/native/d3d12/PhysicalDeviceD3D12.h"
#include "dawn/native/d3d12/PipelineLayoutD3D12.h"
#include "dawn/native/d3d12/PlatformFunctionsD3D12.h"
#include "dawn/native/d3d12/UtilsD3D12.h"
#include "dawn/platform/DawnPlatform.h"
#include "dawn/platform/metrics/HistogramMacros.h"
#include "dawn/platform/tracing/TraceEvent.h"

#include "tint/tint.h"

namespace dawn::native::d3d12 {

namespace {

void DumpDXCCompiledShader(Device* device,
                           const dawn::native::d3d::CompiledShader& compiledShader,
                           uint32_t compileFlags) {
    std::ostringstream dumpedMsg;
    // The HLSL may be empty if compilation failed.
    if (!compiledShader.hlslSource.empty()) {
        dumpedMsg << "/* Dumped generated HLSL */" << std::endl
                  << compiledShader.hlslSource << std::endl;
    }

    // The blob may be empty if DXC compilation failed.
    const Blob& shaderBlob = compiledShader.shaderBlob;
    if (!shaderBlob.Empty()) {
        dumpedMsg << "/* DXC compile flags */ " << std::endl
                  << dawn::native::d3d::CompileFlagsToString(compileFlags) << std::endl;
        dumpedMsg << "/* Dumped disassembled DXIL */" << std::endl;
        DxcBuffer dxcBuffer;
        dxcBuffer.Encoding = DXC_CP_UTF8;
        dxcBuffer.Ptr = shaderBlob.Data();
        dxcBuffer.Size = shaderBlob.Size();

        ComPtr<IDxcResult> dxcResult;
        device->GetDxcCompiler()->Disassemble(&dxcBuffer, IID_PPV_ARGS(&dxcResult));

        ComPtr<IDxcBlobEncoding> disassembly;
        if (dxcResult && dxcResult->HasOutput(DXC_OUT_DISASSEMBLY) &&
            SUCCEEDED(
                dxcResult->GetOutput(DXC_OUT_DISASSEMBLY, IID_PPV_ARGS(&disassembly), nullptr))) {
            dumpedMsg << std::string_view(static_cast<const char*>(disassembly->GetBufferPointer()),
                                          disassembly->GetBufferSize());
        } else {
            dumpedMsg << "DXC disassemble failed" << std::endl;
            ComPtr<IDxcBlobEncoding> errors;
            if (dxcResult && dxcResult->HasOutput(DXC_OUT_ERRORS) &&
                SUCCEEDED(dxcResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr))) {
                dumpedMsg << std::string_view(static_cast<const char*>(errors->GetBufferPointer()),
                                              errors->GetBufferSize());
            }
        }
    }

    std::string logMessage = dumpedMsg.str();
    if (!logMessage.empty()) {
        device->EmitLog(WGPULoggingType_Info, logMessage.c_str());
    }
}
}  // namespace

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
    TRACE_EVENT0(device->GetPlatform(), General, "ShaderModuleD3D12::Compile");
    DAWN_ASSERT(!IsError());

    ScopedTintICEHandler scopedICEHandler(device);
    const EntryPointMetadata& entryPoint = GetEntryPoint(programmableStage.entryPoint);

    d3d::D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnkeyedValue(device->GetPlatform());
    req.hlsl.shaderModel = device->GetDeviceInfo().shaderModel;
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.isRobustnessEnabled = device->IsRobustnessEnabled();
    req.hlsl.disableWorkgroupInit = device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);

    if (usedInterstageVariables.has_value()) {
        req.hlsl.interstageLocations = *usedInterstageVariables;
    }

    req.bytecode.hasShaderF16Feature = device->HasFeature(Feature::ShaderF16);
    req.bytecode.compileFlags = compileFlags;

    if (device->IsToggleEnabled(Toggle::UseDXC)) {
        // If UseDXC toggle are not forced to be disable, DXC should have been validated to be
        // available.
        DAWN_ASSERT(ToBackend(device->GetPhysicalDevice())->GetBackend()->IsDXCAvailable());
        // We can get the DXC version information since IsDXCAvailable() is true.
        d3d::DxcVersionInfo dxcVersionInfo =
            ToBackend(device->GetPhysicalDevice())->GetBackend()->GetDxcVersion();

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

    using tint::BindingPoint;

    tint::BindingRemapperOptions bindingRemapper;
    std::unordered_map<BindingPoint, tint::core::Access> accessControls;

    tint::ArrayLengthFromUniformOptions arrayLengthFromUniform;
    arrayLengthFromUniform.ubo_binding = {layout->GetDynamicStorageBufferLengthsRegisterSpace(),
                                          layout->GetDynamicStorageBufferLengthsShaderRegister()};

    const BindingInfoArray& moduleBindingInfo = entryPoint.bindings;
    for (BindGroupIndex group : IterateBitSet(layout->GetBindGroupLayoutsMask())) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
        const auto& moduleGroupBindingInfo = moduleBindingInfo[group];

        // d3d12::BindGroupLayout packs the bindings per HLSL register-space. We modify
        // the Tint AST to make the "bindings" decoration match the offset chosen by
        // d3d12::BindGroupLayout so that Tint produces HLSL with the correct registers
        // assigned to each interface variable.
        for (const auto& [binding, bindingInfo] : moduleGroupBindingInfo) {
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
                accessControls.emplace(srcBindingPoint, tint::core::Access::kReadWrite);
            }

            // On D3D12 backend all storage buffers without Dynamic Buffer Offset will always be
            // bound to root descriptor tables, where D3D12 runtime can guarantee that OOB-read will
            // always return 0 and OOB-write will always take no action, so we don't need to do
            // robustness transform on them. Note that we still need to do robustness transform on
            // uniform buffers because only sized array is allowed in uniform buffers, so FXC will
            // report compilation error when the indexing to the array in a cBuffer is out of bound
            // and can be checked at compilation time. Storage buffers are OK because they are
            // always translated with RWByteAddressBuffers, which has no such sized arrays.
            //
            // For example below WGSL shader will cause compilation error when we skip robustness
            // transform on uniform buffers:
            //
            // struct TestData {
            //     data: array<vec4<u32>, 3>,
            // };
            // @group(0) @binding(0) var<uniform> s: TestData;
            //
            // fn test() -> u32 {
            //     let index = 1000000u;
            //     if (s.data[index][0] != 0u) {    // error X3504: array index out of bounds
            //         return 0x1004u;
            //     }
            //     return 0u;
            // }
            if ((bindingInfo.buffer.type == wgpu::BufferBindingType::Storage ||
                 bindingInfo.buffer.type == wgpu::BufferBindingType::ReadOnlyStorage) &&
                !bgl->GetBindingInfo(bindingIndex).buffer.hasDynamicOffset) {
                req.hlsl.bindingPointsIgnoredInRobustnessTransform.emplace_back(srcBindingPoint);
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

    std::optional<tint::ast::transform::SubstituteOverride::Config> substituteOverrideConfig;
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
    req.hlsl.accessControls = std::move(accessControls);
    req.hlsl.externalTextureOptions = BuildExternalTextureTransformBindings(layout);
    req.hlsl.arrayLengthFromUniform = std::move(arrayLengthFromUniform);
    req.hlsl.substituteOverrideConfig = std::move(substituteOverrideConfig);

    req.hlsl.polyfillReflectVec2F32 = device->IsToggleEnabled(Toggle::D3D12PolyfillReflectVec2F32);

    const CombinedLimits& limits = device->GetLimits();
    req.hlsl.limits = LimitsForCompilationRequest::Create(limits.v1);

    CacheResult<d3d::CompiledShader> compiledShader;
    MaybeError compileError = [&]() -> MaybeError {
        DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req), d3d::CompiledShader::FromBlob,
                             d3d::CompileShader, "D3D12.CompileShader");
        return {};
    }();

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        if (device->IsToggleEnabled(Toggle::UseDXC)) {
            DumpDXCCompiledShader(device, *compiledShader, compileFlags);
        } else {
            d3d::DumpFXCCompiledShader(device, *compiledShader, compileFlags);
        }
    }

    if (compileError.IsError()) {
        return {compileError.AcquireError()};
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    d3d::CompiledShader result = compiledShader.Acquire();
    result.hlslSource = "";
    return result;
}

}  // namespace dawn::native::d3d12
