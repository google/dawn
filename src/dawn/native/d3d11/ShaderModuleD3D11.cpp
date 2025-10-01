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
#include <unordered_map>
#include <utility>

#include "dawn/common/Assert.h"
#include "dawn/common/MatchVariant.h"
#include "dawn/native/ImmediateConstantsLayout.h"
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
    const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
    const std::vector<tint::wgsl::Extension>& internalExtensions) {
    Ref<ShaderModule> shader = AcquireRef(new ShaderModule(device, descriptor, internalExtensions));
    shader->Initialize();
    return shader;
}

ShaderModule::ShaderModule(Device* device,
                           const UnpackedPtr<ShaderModuleDescriptor>& descriptor,
                           std::vector<tint::wgsl::Extension> internalExtensions)
    : ShaderModuleBase(device, descriptor, std::move(internalExtensions)) {}

ResultOrError<d3d::CompiledShader> ShaderModule::Compile(
    const ProgrammableStage& programmableStage,
    SingleShaderStage stage,
    const PipelineLayout* layout,
    uint32_t compileFlags,
    const ImmediateConstantMask& pipelineImmediateMask,
    const std::optional<dawn::native::d3d::InterStageShaderVariablesMask>& usedInterstageVariables,
    const std::optional<tint::hlsl::writer::PixelLocalOptions>& pixelLocalOptions) {
    Device* device = ToBackend(GetDevice());
    TRACE_EVENT0(device->GetPlatform(), General, "ShaderModuleD3D11::Compile");
    DAWN_ASSERT(!IsError());

    d3d::D3DCompilationRequest req = {};
    req.tracePlatform = UnsafeUnserializedValue(device->GetPlatform());
    req.hlsl.shaderModel = 50;
    req.hlsl.disableSymbolRenaming = device->IsToggleEnabled(Toggle::DisableSymbolRenaming);
    req.hlsl.dumpShaders = device->IsToggleEnabled(Toggle::DumpShaders);
    req.hlsl.dumpShadersOnFailure = device->IsToggleEnabled(Toggle::DumpShadersOnFailure);
    req.hlsl.tintOptions.remapped_entry_point_name = device->GetIsolatedEntryPointName();

    req.bytecode.hasShaderF16Feature = false;
    req.bytecode.compileFlags = compileFlags;

    // D3D11 only supports FXC.
    req.bytecode.compiler = d3d::Compiler::FXC;
    req.bytecode.d3dCompile =
        UnsafeUnserializedValue(pD3DCompile{device->GetFunctions()->d3dCompile});
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

    tint::Bindings bindings;

    for (BindGroupIndex group : layout->GetBindGroupLayoutsMask()) {
        const BindGroupLayout* bgl = ToBackend(layout->GetBindGroupLayout(group));
        const auto& indices = layout->GetBindingTableIndexMap()[group];

        for (const auto& [bindingNumber, apiBindingIndex] : bgl->GetBindingMap()) {
            if (!(bgl->GetAPIBindingInfo(apiBindingIndex).visibility & StageBit(stage))) {
                continue;
            }

            tint::BindingPoint srcBindingPoint{
                .group = uint32_t(group),
                .binding = uint32_t(bindingNumber),
            };

            auto ComputeDestinationBindingPoint = [&](BindingIndex bindingIndex) {
                tint::BindingPoint dstBindingPoint{0u, indices[bindingIndex][stage]};
                DAWN_ASSERT(dstBindingPoint.binding != PipelineLayout::kInvalidSlot);
                return dstBindingPoint;
            };

            MatchVariant(
                bgl->GetAPIBindingInfo(apiBindingIndex).bindingLayout,
                [&](const BufferBindingInfo& bindingInfo) {
                    tint::BindingPoint dstBindingPoint =
                        ComputeDestinationBindingPoint(bgl->AsBindingIndex(apiBindingIndex));
                    switch (bindingInfo.type) {
                        case wgpu::BufferBindingType::Uniform:
                            bindings.uniform.emplace(srcBindingPoint, dstBindingPoint);
                            break;
                        case kInternalStorageBufferBinding:
                        case wgpu::BufferBindingType::Storage:
                        case wgpu::BufferBindingType::ReadOnlyStorage:
                        case kInternalReadOnlyStorageBufferBinding:
                            bindings.storage.emplace(srcBindingPoint, dstBindingPoint);
                            break;
                        case wgpu::BufferBindingType::BindingNotUsed:
                        case wgpu::BufferBindingType::Undefined:
                            DAWN_UNREACHABLE();
                            break;
                    }
                },
                [&](const SamplerBindingInfo& bindingInfo) {
                    bindings.sampler.emplace(
                        srcBindingPoint,
                        ComputeDestinationBindingPoint(bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const TextureBindingInfo& bindingInfo) {
                    bindings.texture.emplace(
                        srcBindingPoint,
                        ComputeDestinationBindingPoint(bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const StorageTextureBindingInfo& bindingInfo) {
                    bindings.storage_texture.emplace(
                        srcBindingPoint,
                        ComputeDestinationBindingPoint(bgl->AsBindingIndex(apiBindingIndex)));
                },
                [&](const TexelBufferBindingInfo& bindingInfo) {
                    // TODO(crbug/382544164): Prototype texel buffer feature
                    DAWN_UNREACHABLE();
                },
                [&](const ExternalTextureBindingInfo& bindingInfo) {
                    bindings.external_texture.emplace(
                        srcBindingPoint,
                        tint::ExternalTexture{
                            .metadata = ComputeDestinationBindingPoint(bindingInfo.params),
                            .plane0 = ComputeDestinationBindingPoint(bindingInfo.plane0),
                            .plane1 = ComputeDestinationBindingPoint(bindingInfo.plane1)});
                },

                [](const InputAttachmentBindingInfo&) { DAWN_UNREACHABLE(); },
                [](const StaticSamplerBindingInfo&) { DAWN_UNREACHABLE(); });
        }
    }

    req.hlsl.shaderModuleHash = GetHash();
    req.hlsl.inputProgram = UnsafeUnserializedValue(UseTintProgram());
    req.hlsl.entryPointName = programmableStage.entryPoint.c_str();
    req.hlsl.stage = stage;
    req.hlsl.substituteOverrideConfig = BuildSubstituteOverridesTransformConfig(programmableStage);
    req.hlsl.limits = LimitsForCompilationRequest::Create(device->GetLimits().v1);
    req.hlsl.adapterSupportedLimits = UnsafeUnserializedValue(
        LimitsForCompilationRequest::Create(device->GetAdapter()->GetLimits().v1));
    req.hlsl.maxSubgroupSize = device->GetAdapter()->GetPhysicalDevice()->GetSubgroupMaxSize();

    req.hlsl.tintOptions.disable_robustness = !device->IsRobustnessEnabled();
    req.hlsl.tintOptions.disable_workgroup_init =
        device->IsToggleEnabled(Toggle::DisableWorkgroupInit);
    req.hlsl.tintOptions.bindings = std::move(bindings);
    req.hlsl.tintOptions.scalarize_max_min_clamp =
        device->IsToggleEnabled(Toggle::ScalarizeMaxMinClamp);

    req.hlsl.tintOptions.immediate_binding_point =
        tint::BindingPoint{0, PipelineLayout::kReservedConstantBufferSlot};
    if (stage == SingleShaderStage::Compute) {
        req.hlsl.tintOptions.num_workgroups_start_offset = GetImmediateByteOffsetInPipelineIfAny(
            &ComputeImmediateConstants::numWorkgroups, pipelineImmediateMask);
    } else {
        req.hlsl.tintOptions.first_index_offset = GetImmediateByteOffsetInPipelineIfAny(
            &RenderImmediateConstants::firstVertex, pipelineImmediateMask);
        req.hlsl.tintOptions.first_instance_offset = GetImmediateByteOffsetInPipelineIfAny(
            &RenderImmediateConstants::firstInstance, pipelineImmediateMask);
    }

    if (stage == SingleShaderStage::Vertex) {
        // Now that only vertex shader can have interstage outputs.
        // Pass in the actually used interstage locations for tint to potentially truncate unused
        // outputs.
        if (usedInterstageVariables.has_value()) {
            req.hlsl.tintOptions.interstage_locations = *usedInterstageVariables;
        }
        req.hlsl.tintOptions.truncate_interstage_variables = true;
    } else if (stage == SingleShaderStage::Fragment) {
        if (pixelLocalOptions.has_value()) {
            req.hlsl.tintOptions.pixel_local = *pixelLocalOptions;
        }
    }

    // D3D11 only supports FXC
    req.hlsl.tintOptions.compiler = tint::hlsl::writer::Options::Compiler::kFXC;

    // TODO(dawn:1705): do we need to support it?
    req.hlsl.tintOptions.polyfill_reflect_vec2_f32 = false;

    // D3D11 doesn't support shader model 6+ features
    req.hlsl.tintOptions.polyfill_dot_4x8_packed = true;
    req.hlsl.tintOptions.polyfill_pack_unpack_4x8 = true;

    CacheResult<d3d::CompiledShader> compiledShader;
    DAWN_TRY_LOAD_OR_RUN(compiledShader, device, std::move(req),
                         d3d::CompiledShader::FromValidatedBlob, d3d::CompileShader,
                         "D3D11.CompileShader");

    if (device->IsToggleEnabled(Toggle::DumpShaders)) {
        d3d::DumpFXCCompiledShader(device, *compiledShader, compileFlags);
    }

    device->GetBlobCache()->EnsureStored(compiledShader);

    // Clear the hlslSource. It is only used for logging and should not be used
    // outside of the compilation.
    d3d::CompiledShader result = compiledShader.Acquire();
    result.hlslSource = std::string();

    return result;
}

}  // namespace dawn::native::d3d11
